/* Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/*
  This code needs extra visibility in the lexer structures
*/

#include "sql/sql_digest.h"

#include "lex_string.h"
#include "m_ctype.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "sha2.h"                   // SHA256
#include "sql/lexer_yystype.h"      // Lexer_yystype
#include "sql/sql_digest_stream.h"  // sql_digest_state
#include "sql/sql_yacc.h"           // Generated code.
#include "sql_string.h"             // String

#define LEX_TOKEN_WITH_DEFINITION
#include "sql/lex_token.h"

#define SIZE_OF_A_TOKEN 2

ulong max_digest_length = 0;
ulong get_max_digest_length() { return max_digest_length; }

/**
  Read a single token from token array.
*/
inline uint read_token(const sql_digest_storage *digest_storage, uint index,
                       uint *tok) {
  size_t safe_byte_count = digest_storage->m_byte_count;

  if (index + SIZE_OF_A_TOKEN <= safe_byte_count &&
      safe_byte_count <= digest_storage->m_token_array_length) {
    const unsigned char *src = &digest_storage->m_token_array[index];
    *tok = src[0] | (src[1] << 8);
    return index + SIZE_OF_A_TOKEN;
  }

  /* The input byte stream is exhausted. */
  *tok = 0;
  return MAX_DIGEST_STORAGE_SIZE + 1;
}

/**
  Store a single token in token array.
*/
inline void store_token(sql_digest_storage *digest_storage, uint token) {
  /* WRITE: ok to assert, storing a token is race free. */
  DBUG_ASSERT(digest_storage->m_byte_count <=
              digest_storage->m_token_array_length);

  if (digest_storage->m_byte_count + SIZE_OF_A_TOKEN <=
      digest_storage->m_token_array_length) {
    unsigned char *dest =
        &digest_storage->m_token_array[digest_storage->m_byte_count];
    dest[0] = token & 0xff;
    dest[1] = (token >> 8) & 0xff;
    digest_storage->m_byte_count += SIZE_OF_A_TOKEN;
  } else {
    digest_storage->m_full = true;
  }
}

/**
  Read an identifier from token array.
*/
inline uint read_identifier(const sql_digest_storage *digest_storage,
                            uint index, const char **id_string,
                            int *id_length) {
  uint new_index;
  uint safe_byte_count = digest_storage->m_byte_count;

  /* READ: never assert on data, reading can be racy when used concurrently
   * (pfs). */

  /*
    token + length + string are written in an atomic way,
    so we do always expect a length + string here
  */

  uint bytes_needed = SIZE_OF_A_TOKEN;
  /* If we can read token and identifier length */
  if ((safe_byte_count <= digest_storage->m_token_array_length) &&
      (index + bytes_needed) <= safe_byte_count) {
    const unsigned char *src = &digest_storage->m_token_array[index];
    /* Read the length of identifier */
    uint length = src[0] | (src[1] << 8);
    bytes_needed += length;
    /* If we can read entire identifier from token array */
    if ((index + bytes_needed) <= safe_byte_count) {
      *id_string = pointer_cast<const char *>(src) + 2;
      *id_length = length;

      new_index = index + bytes_needed;
      return new_index;
    }
  }

  /* The input byte stream is exhausted. */
  return MAX_DIGEST_STORAGE_SIZE + 1;
}

/**
  Store an identifier in token array.
*/
inline void store_token_identifier(sql_digest_storage *digest_storage,
                                   uint token, size_t id_length,
                                   const char *id_name) {
  /* WRITE: ok to assert, storing a token is race free. */
  DBUG_ASSERT(digest_storage->m_byte_count <=
              digest_storage->m_token_array_length);

  size_t bytes_needed = 2 * SIZE_OF_A_TOKEN + id_length;
  if (digest_storage->m_byte_count + bytes_needed <=
      (unsigned int)digest_storage->m_token_array_length) {
    unsigned char *dest =
        &digest_storage->m_token_array[digest_storage->m_byte_count];
    /* Write the token */
    dest[0] = token & 0xff;
    dest[1] = (token >> 8) & 0xff;
    /* Write the string length */
    dest[2] = id_length & 0xff;
    dest[3] = (id_length >> 8) & 0xff;
    /* Write the string data */
    if (id_length > 0) {
      memcpy((char *)(dest + 4), id_name, id_length);
    }
    digest_storage->m_byte_count += bytes_needed;
  } else {
    digest_storage->m_full = true;
  }
}

void compute_digest_hash(const sql_digest_storage *digest_storage,
                         unsigned char *hash) {
  compute_md5_hash((char *)hash, (const char *)digest_storage->m_token_array,
                   digest_storage->m_byte_count);
}

/*
  Iterate token array and updates digest_text.
*/
void compute_digest_text(const sql_digest_storage *digest_storage,
                         String *digest_text) {
  DBUG_ASSERT(digest_storage != nullptr);
  uint byte_count = digest_storage->m_byte_count;
  String *digest_output = digest_text;
  uint tok = 0;
  uint current_byte = 0;
  lex_token_string *tok_data;

  /*
    When a space needs to be appended, set add_space to true,
    and delay actually adding the space until the next token
    is found.
    This is to prevent printing digest text
    with a trailing space character.
  */
  bool add_space = false;

  /* Reset existing data */
  digest_output->length(0);

  if (byte_count > digest_storage->m_token_array_length) {
    digest_output->append("\0", 1);
    return;
  }

  /* Convert text to utf8 */
  const CHARSET_INFO *from_cs =
      get_charset(digest_storage->m_charset_number, MYF(0));
  const CHARSET_INFO *to_cs = &my_charset_utf8_bin;

  if (from_cs == nullptr) {
    /*
      Can happen, as we do dirty reads on digest_storage,
      which can be written to in another thread.
    */
    digest_output->append("\0", 1);
    return;
  }

  char id_buffer[NAME_LEN + 1] = {'\0'};
  const char *id_string;
  size_t id_length;
  bool convert_text = !my_charset_same(from_cs, to_cs);

  while (current_byte < byte_count) {
    current_byte = read_token(digest_storage, current_byte, &tok);

    if (tok <= 0 || tok >= array_elements(lex_token_array) ||
        current_byte > max_digest_length) {
      return;
    }

    tok_data = &lex_token_array[tok];

    switch (tok) {
      /* All identifiers are printed with their name. */
      case IDENT:
      case IDENT_QUOTED:
      case TOK_IDENT:
      case TOK_IDENT_AT: {
        const char *id_ptr = nullptr;
        int id_len = 0;
        uint err_cs = 0;

        /* Get the next identifier from the storage buffer. */
        current_byte =
            read_identifier(digest_storage, current_byte, &id_ptr, &id_len);
        if (current_byte > max_digest_length) {
          /* Truncation */
          return;
        }

        if (convert_text) {
          /* Verify that the converted text will fit. */
          if (to_cs->mbmaxlen * id_len > NAME_LEN) {
            if (add_space) {
              digest_output->append(" ", 1);
              add_space = false;
            }

            digest_output->append("...", 3);
            break;
          }
          /* Convert identifier string into the storage character set. */
          id_length = my_convert(id_buffer, NAME_LEN, to_cs, id_ptr, id_len,
                                 from_cs, &err_cs);
          id_string = id_buffer;
        } else {
          id_string = id_ptr;
          id_length = id_len;
        }

        if (id_length == 0 || err_cs != 0) {
          break;
        }

        if (add_space) {
          digest_output->append(" ", 1);
          add_space = false;
        }

        /* Copy the converted identifier into the digest string. */
        digest_output->append("`", 1);
        if (id_length > 0) {
          digest_output->append(id_string, id_length);
        }
        if (tok == TOK_IDENT_AT)  // No space before @ in "table@query_block".
        {
          digest_output->append("`", 1);
        } else {
          digest_output->append("`", 1);
          add_space = true;
        }
      } break;

      /* Everything else is printed as is. */
      default:
        if (add_space) {
          digest_output->append(" ", 1);
          add_space = false;
        }

        int tok_length = tok_data->m_token_length;

        digest_output->append(tok_data->m_token_string, tok_length);
        if (tok_data->m_append_space) {
          add_space = true;
        }
        break;
    }
  }
}

static inline uint peek_token(const sql_digest_storage *digest, uint index) {
  uint token;
  DBUG_ASSERT(index + SIZE_OF_A_TOKEN <= digest->m_byte_count);
  DBUG_ASSERT(digest->m_byte_count <= digest->m_token_array_length);

  token =
      ((digest->m_token_array[index + 1]) << 8) | digest->m_token_array[index];
  return token;
}

/**
  Function to read last two tokens from token array. If an identifier
  is found, do not look for token before that.
*/
static inline void peek_last_two_tokens(
    const sql_digest_storage *digest_storage, uint last_id_index, uint *t1,
    uint *t2) {
  uint byte_count = digest_storage->m_byte_count;
  uint peek_index = byte_count;

  if (last_id_index + SIZE_OF_A_TOKEN <= peek_index) {
    /* Take last token. */
    peek_index -= SIZE_OF_A_TOKEN;
    *t1 = peek_token(digest_storage, peek_index);

    if (last_id_index + SIZE_OF_A_TOKEN <= peek_index) {
      /* Take 2nd token from last. */
      peek_index -= SIZE_OF_A_TOKEN;
      *t2 = peek_token(digest_storage, peek_index);
    } else {
      *t2 = TOK_UNUSED;
    }
  } else {
    *t1 = TOK_UNUSED;
    *t2 = TOK_UNUSED;
  }
}

/**
  Function to read last three tokens from token array. If an identifier
  is found, do not look for token before that.
*/
static inline void peek_last_three_tokens(
    const sql_digest_storage *digest_storage, uint last_id_index, uint *t1,
    uint *t2, uint *t3) {
  uint byte_count = digest_storage->m_byte_count;
  uint peek_index = byte_count;

  if (last_id_index + SIZE_OF_A_TOKEN <= peek_index) {
    /* Take last token. */
    peek_index -= SIZE_OF_A_TOKEN;
    *t1 = peek_token(digest_storage, peek_index);

    if (last_id_index + SIZE_OF_A_TOKEN <= peek_index) {
      /* Take 2nd token from last. */
      peek_index -= SIZE_OF_A_TOKEN;
      *t2 = peek_token(digest_storage, peek_index);

      if (last_id_index + SIZE_OF_A_TOKEN <= peek_index) {
        /* Take 3rd token from last. */
        peek_index -= SIZE_OF_A_TOKEN;
        *t3 = peek_token(digest_storage, peek_index);
      } else {
        *t3 = TOK_UNUSED;
      }
    } else {
      *t2 = TOK_UNUSED;
      *t3 = TOK_UNUSED;
    }
  } else {
    *t1 = TOK_UNUSED;
    *t2 = TOK_UNUSED;
    *t3 = TOK_UNUSED;
  }
}

sql_digest_state *digest_add_token(sql_digest_state *state, uint token,
                                   Lexer_yystype *yylval) {
  sql_digest_storage *digest_storage = nullptr;

  digest_storage = &state->m_digest_storage;

  /*
    Stop collecting further tokens if digest storage is full or
    if END token is received.
  */
  if (digest_storage->m_full || token == END_OF_INPUT) {
    return nullptr;
  }

  /*
    Take last_token 2 tokens collected till now. These tokens will be used
    in reduce for normalisation. Make sure not to consider ID tokens in reduce.
  */
  uint last_token;
  uint last_token2;

  switch (token) {
    case NUM:
    case LONG_NUM:
    case ULONGLONG_NUM:
    case DECIMAL_NUM:
    case FLOAT_NUM:
    case BIN_NUM:
    case HEX_NUM: {
      bool found_unary;
      do {
        found_unary = false;
        peek_last_two_tokens(digest_storage, state->m_last_id_index,
                             &last_token, &last_token2);

        if ((last_token == '-') || (last_token == '+')) {
          /*
            We need to differentiate:
            - a <unary minus> operator
            - a <unary plus> operator
            from
            - a <binary minus> operator
            - a <binary plus> operator
            to only reduce "a = -1" to "a = ?", and not change "b - 1" to "b ?"

            Binary operators are found inside an expression,
            while unary operators are found at the beginning of an expression,
            or after operators.

            To achieve this, every token that is followed by an <expr>
            expression in the SQL grammar is flagged. See sql/sql_yacc.yy See
            sql/gen_lex_token.cc

            For example,
            "(-1)" is parsed as "(", "-", NUM, ")", and
            lex_token_array["("].m_start_expr is true, so reduction of the "-"
            NUM is done, the result is "(?)".
            "(a-1)" is parsed as "(", ID, "-", NUM, ")", and
            lex_token_array[ID].m_start_expr is false, so the operator is
            binary, no reduction is done, and the result is "(a-?)".
          */
          if (lex_token_array[last_token2].m_start_expr) {
            /*
              REDUCE:
              TOK_GENERIC_VALUE := (UNARY_PLUS | UNARY_MINUS) (NUM | LOG_NUM |
              ... | FLOAT_NUM)

              REDUCE:
              TOK_GENERIC_VALUE := (UNARY_PLUS | UNARY_MINUS) TOK_GENERIC_VALUE
            */
            token = TOK_GENERIC_VALUE;
            digest_storage->m_byte_count -= SIZE_OF_A_TOKEN;
            found_unary = true;
          }
        }
      } while (found_unary);
    }
    /* fall through, for case NULL_SYM below */
    case LEX_HOSTNAME:
    case TEXT_STRING:
    case NCHAR_STRING:
    case PARAM_MARKER: {
      /*
        REDUCE:
        TOK_GENERIC_VALUE := BIN_NUM | DECIMAL_NUM | ... | ULONGLONG_NUM
      */
      token = TOK_GENERIC_VALUE;

      peek_last_two_tokens(digest_storage, state->m_last_id_index, &last_token,
                           &last_token2);

      if ((last_token2 == TOK_GENERIC_VALUE ||
           last_token2 == TOK_GENERIC_VALUE_LIST) &&
          (last_token == ',')) {
        /*
          REDUCE:
          TOK_GENERIC_VALUE_LIST :=
            TOK_GENERIC_VALUE ',' TOK_GENERIC_VALUE

          REDUCE:
          TOK_GENERIC_VALUE_LIST :=
            TOK_GENERIC_VALUE_LIST ',' TOK_GENERIC_VALUE
        */
        digest_storage->m_byte_count -= 2 * SIZE_OF_A_TOKEN;
        token = TOK_GENERIC_VALUE_LIST;
      }
      /*
        Add this token or the resulting reduce to digest storage.
      */
      store_token(digest_storage, token);
      break;
    }
    case ')': {
      peek_last_two_tokens(digest_storage, state->m_last_id_index, &last_token,
                           &last_token2);

      if (last_token == TOK_GENERIC_VALUE && last_token2 == '(') {
        /*
          REDUCE:
          TOK_ROW_SINGLE_VALUE :=
            '(' TOK_GENERIC_VALUE ')'
        */
        digest_storage->m_byte_count -= 2 * SIZE_OF_A_TOKEN;
        token = TOK_ROW_SINGLE_VALUE;

        /* Read last two tokens again */
        peek_last_two_tokens(digest_storage, state->m_last_id_index,
                             &last_token, &last_token2);

        if ((last_token2 == TOK_ROW_SINGLE_VALUE ||
             last_token2 == TOK_ROW_SINGLE_VALUE_LIST) &&
            (last_token == ',')) {
          /*
            REDUCE:
            TOK_ROW_SINGLE_VALUE_LIST :=
              TOK_ROW_SINGLE_VALUE ',' TOK_ROW_SINGLE_VALUE

            REDUCE:
            TOK_ROW_SINGLE_VALUE_LIST :=
              TOK_ROW_SINGLE_VALUE_LIST ',' TOK_ROW_SINGLE_VALUE
          */
          digest_storage->m_byte_count -= 2 * SIZE_OF_A_TOKEN;
          token = TOK_ROW_SINGLE_VALUE_LIST;
        } else if (last_token == IN_SYM) {
          /*
            REDUCE:
            TOK_IN_GENERIC_VALUE_EXPRESSION :=
              IN_SYM TOK_ROW_SINGLE_VALUE
          */
          digest_storage->m_byte_count -= SIZE_OF_A_TOKEN;
          token = TOK_IN_GENERIC_VALUE_EXPRESSION;
        }
      } else if (last_token == TOK_GENERIC_VALUE_LIST && last_token2 == '(') {
        /*
          REDUCE:
          TOK_ROW_MULTIPLE_VALUE :=
            '(' TOK_GENERIC_VALUE_LIST ')'
        */
        digest_storage->m_byte_count -= 2 * SIZE_OF_A_TOKEN;
        token = TOK_ROW_MULTIPLE_VALUE;

        /* Read last two tokens again */
        peek_last_two_tokens(digest_storage, state->m_last_id_index,
                             &last_token, &last_token2);

        if ((last_token2 == TOK_ROW_MULTIPLE_VALUE ||
             last_token2 == TOK_ROW_MULTIPLE_VALUE_LIST) &&
            (last_token == ',')) {
          /*
            REDUCE:
            TOK_ROW_MULTIPLE_VALUE_LIST :=
              TOK_ROW_MULTIPLE_VALUE ',' TOK_ROW_MULTIPLE_VALUE

            REDUCE:
            TOK_ROW_MULTIPLE_VALUE_LIST :=
              TOK_ROW_MULTIPLE_VALUE_LIST ',' TOK_ROW_MULTIPLE_VALUE
          */
          digest_storage->m_byte_count -= 2 * SIZE_OF_A_TOKEN;
          token = TOK_ROW_MULTIPLE_VALUE_LIST;
        } else if (last_token == IN_SYM) {
          /*
          REDUCE:
          TOK_IN_GENERIC_VALUE_EXPRESSION :=
          IN_SYM TOK_ROW_MULTIPLE_VALUE
          */
          digest_storage->m_byte_count -= SIZE_OF_A_TOKEN;
          token = TOK_IN_GENERIC_VALUE_EXPRESSION;
        }
      }
      /*
        Add this token or the resulting reduce to digest storage.
      */
      store_token(digest_storage, token);
      break;
    }
    case IDENT:
    case IDENT_QUOTED:
    case TOK_IDENT_AT: {
      Lexer_yystype *lex_token = yylval;
      char *yytext = lex_token->lex_str.str;
      size_t yylen = lex_token->lex_str.length;

      /*
        REDUCE:
          TOK_IDENT := IDENT | IDENT_QUOTED
        The parser gives IDENT or IDENT_TOKEN for the same text,
        depending on the character set used.
        We unify both to always print the same digest text,
        and always have the same digest hash.
      */
      if (token != TOK_IDENT_AT) {
        token = TOK_IDENT;
      }
      /* Add this token and identifier string to digest storage. */
      store_token_identifier(digest_storage, token, yylen, yytext);

      /* Update the index of last identifier found. */
      state->m_last_id_index = digest_storage->m_byte_count;
      break;
    }
    case 0: {
      if (digest_storage->m_byte_count < SIZE_OF_A_TOKEN) {
        break;
      }
      unsigned int temp_tok;
      read_token(digest_storage, digest_storage->m_byte_count - SIZE_OF_A_TOKEN,
                 &temp_tok);
      if (temp_tok == ';') {
        digest_storage->m_byte_count -= SIZE_OF_A_TOKEN;
      }
      break;
    }
    default: {
      /* Add this token to digest storage. */
      store_token(digest_storage, token);
      break;
    }
  }

  return state;
}

sql_digest_state *digest_reduce_token(sql_digest_state *state, uint token_left,
                                      uint token_right) {
  sql_digest_storage *digest_storage = nullptr;

  digest_storage = &state->m_digest_storage;

  /*
    Stop collecting further tokens if digest storage is full.
  */
  if (digest_storage->m_full) {
    return nullptr;
  }

  uint last_token;
  uint last_token2;
  uint last_token3;
  uint token_to_push = TOK_UNUSED;

  peek_last_two_tokens(digest_storage, state->m_last_id_index, &last_token,
                       &last_token2);

  /*
    There is only one caller of digest_reduce_token(),
    see sql/sql_yacc.yy, rule literal := NULL_SYM.
    REDUCE:
      token_left := token_right
    Used for:
      TOK_GENERIC_VALUE := NULL_SYM
  */

  if (last_token == token_right) {
    /*
      Current stream is like:
        TOKEN_X TOKEN_RIGHT .
      REDUCE to
        TOKEN_X TOKEN_LEFT .
    */
    digest_storage->m_byte_count -= SIZE_OF_A_TOKEN;
    store_token(digest_storage, token_left);
  } else {
    /*
      Current stream is like:
        TOKEN_X TOKEN_RIGHT TOKEN_Y .
      Pop TOKEN_Y
        TOKEN_X TOKEN_RIGHT . TOKEN_Y
      REDUCE to
        TOKEN_X TOKEN_LEFT . TOKEN_Y
    */
    DBUG_ASSERT(last_token2 == token_right);
    digest_storage->m_byte_count -= 2 * SIZE_OF_A_TOKEN;
    store_token(digest_storage, token_left);
    token_to_push = last_token;
  }

  peek_last_three_tokens(digest_storage, state->m_last_id_index, &last_token,
                         &last_token2, &last_token3);

  if ((last_token3 == TOK_GENERIC_VALUE ||
       last_token3 == TOK_GENERIC_VALUE_LIST) &&
      (last_token2 == ',') && (last_token == TOK_GENERIC_VALUE)) {
    /*
      REDUCE:
      TOK_GENERIC_VALUE_LIST :=
        TOK_GENERIC_VALUE ',' TOK_GENERIC_VALUE

      REDUCE:
      TOK_GENERIC_VALUE_LIST :=
        TOK_GENERIC_VALUE_LIST ',' TOK_GENERIC_VALUE
    */
    digest_storage->m_byte_count -= 3 * SIZE_OF_A_TOKEN;
    store_token(digest_storage, TOK_GENERIC_VALUE_LIST);
  }

  if (token_to_push != TOK_UNUSED) {
    /*
      Push TOKEN_Y
    */
    store_token(digest_storage, token_to_push);
  }

  return state;
}
