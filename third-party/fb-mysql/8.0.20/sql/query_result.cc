/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/query_result.h"

#include "my_config.h"

#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <algorithm>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "sql/derror.h"  // ER_THD
#include "sql/item.h"
#include "sql/item_func.h"
#include "sql/mysqld.h"            // key_select_to_file
#include "sql/parse_tree_nodes.h"  // PT_select_var
#include "sql/protocol.h"
#include "sql/sp_rcontext.h"  // sp_rcontext
#include "sql/sql_class.h"    // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_exchange.h"
#include "sql/system_variables.h"
#include "sql_string.h"

using std::min;

bool Query_result_send::send_result_set_metadata(THD *thd, List<Item> &list,
                                                 uint flags) {
  bool res;
  if (!(res = thd->send_result_metadata(&list, flags)))
    is_result_set_started = true;
  return res;
}

void Query_result_send::abort_result_set(THD *thd) {
  DBUG_TRACE;

  if (is_result_set_started && thd->sp_runtime_ctx) {
    /*
      We're executing a stored procedure, have an open result
      set and an SQL exception condition. In this situation we
      must abort the current statement, silence the error and
      start executing the continue/exit handler if one is found.
      Before aborting the statement, let's end the open result set, as
      otherwise the client will hang due to the violation of the
      client/server protocol.
    */
    thd->sp_runtime_ctx->end_partial_result_set = true;
  }
}

/* Send data to client. Returns 0 if ok */

bool Query_result_send::send_data(THD *thd, List<Item> &items) {
  Protocol *protocol = thd->get_protocol();
  DBUG_TRACE;

  protocol->start_row();
  if (thd->send_result_set_row(&items)) {
    protocol->abort_row();
    return true;
  }

  thd->inc_sent_row_count(1);
  return protocol->end_row();
}

bool Query_result_send::send_eof(THD *thd) {
  /*
    Don't send EOF if we're in error condition (which implies we've already
    sent or are sending an error)
  */
  if (thd->is_error()) return true;
  ::my_eof(thd);
  is_result_set_started = false;
  return false;
}

static const String default_line_term("\n", default_charset_info);
static const String default_escaped("\\", default_charset_info);
static const String default_field_term("\t", default_charset_info);
static const String default_xml_row_term("<row>", default_charset_info);
static const String my_empty_string("", default_charset_info);

sql_exchange::sql_exchange(const char *name, bool flag,
                           enum enum_filetype filetype_arg)
    : file_name(name),
      dumpfile(flag),
      compressed_chunk_expr(nullptr),
      load_compressed(false),
      skip_lines(0) {
  field.opt_enclosed = false;
  filetype = filetype_arg;
  field.field_term = &default_field_term;
  field.enclosed = line.line_start = &my_empty_string;
  line.line_term =
      filetype == FILETYPE_CSV ? &default_line_term : &default_xml_row_term;
  field.escaped = &default_escaped;
  cs = nullptr;
}

bool sql_exchange::escaped_given(void) {
  return field.escaped != &default_escaped;
}

/************************************************************************
  Handling writing to file
************************************************************************/

void Query_result_to_file::send_error(THD *, uint errcode, const char *err) {
  my_message(errcode, err, MYF(0));
  if (file > 0) {
    (void)end_io_cache(&cache);
    mysql_file_close(file, MYF(0));
    /* Delete file on error */
    mysql_file_delete(key_select_to_file, path, MYF(0));
    file = -1;
  }
}

bool Query_result_to_file::send_eof(THD *thd) {
  bool error = (end_io_cache(&cache) != 0);
  if (mysql_file_close(file, MYF(MY_WME)) || thd->is_error()) error = true;

  if (!error) {
    ::my_ok(thd, row_count);
  }
  file = -1;
  return error;
}

void Query_result_to_file::close_file_handle() {
  if (file >= 0) {
    (void)end_io_cache(&cache);
    mysql_file_close(file, MYF(0));
    file = -1;
  }
}

void Query_result_to_file::cleanup(THD *) {
  /* In case of error send_eof() may be not called: close the file here. */
  close_file_handle();
  path[0] = '\0';
  row_count = 0;
}

/***************************************************************************
** Export of select to textfile
***************************************************************************/

// This is a hack to make it compile. File permissions are different on Windows.
#ifdef _WIN32
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IRGRP 00040
#endif

/*
  Create file with IO cache

  SYNOPSIS
    create_file()
    thd			Thread handle
    path		File name
    file_name		File name in input
    compressed		is file compressed
    exchange		Excange class
    cache		IO cache

  RETURN
    >= 0 	File handle
   -1		Error
*/

static File create_file(THD *thd, char *path, const char *file_name,
                        bool compressed, IO_CACHE *cache) {
  File file;
  uint option = MY_UNPACK_FILENAME | MY_RELATIVE_PATH;

  if (!dirname_length(file_name)) {
    strxnmov(path, FN_REFLEN - 1, mysql_real_data_home,
             thd->db().str ? thd->db().str : "", NullS);
    (void)fn_format(path, file_name, path, "", option);
  } else
    (void)fn_format(path, file_name, mysql_real_data_home, "", option);

  if (!is_secure_file_path(path)) {
    /* Write only allowed to dir or subdir specified by secure_file_priv */
    my_error(ER_OPTION_PREVENTS_STATEMENT, MYF(0), "--secure-file-priv", "");
    return -1;
  }

  if (!access(path, F_OK)) {
    my_error(ER_FILE_EXISTS_ERROR, MYF(0), file_name);
    return -1;
  }
  /* Create the file world readable */
  if ((file = mysql_file_create(key_select_to_file, path,
                                S_IRUSR | S_IWUSR | S_IRGRP, O_WRONLY | O_EXCL,
                                MYF(MY_WME))) < 0)
    return file;
#ifdef HAVE_FCHMOD
  (void)fchmod(file, S_IRUSR | S_IWUSR | S_IRGRP);  // Because of umask()
#else
  (void)chmod(path, S_IRUSR | S_IWUSR | S_IRGRP);
#endif
  if (init_io_cache_with_opt_compression(cache, file, 0L, WRITE_CACHE, 0L, true,
                                         MYF(MY_WME), compressed)) {
    mysql_file_close(file, MYF(0));
    /* Delete file on error, it was just created */
    mysql_file_delete(key_select_to_file, path, MYF(0));
    return -1;
  }
  return file;
}

/*
  Create a new compressed file of the form <filename>.<monotonic number>.zst.
  This is used to assign unique names for files created by chunking.
 */
bool Query_result_export::open_new_compressed_file(THD *thd) {
  char new_file_buff[FN_REFLEN];
  int wr = snprintf(new_file_buff, sizeof(new_file_buff), "%s.%lu.zst",
                    exchange->file_name, current_chunk_idx++);

  if (wr >= FN_REFLEN || wr < 0) return true;

  if ((file = create_file(thd, path, new_file_buff, true /* compressed */,
                          &cache)) < 0)
    return true;

  return false;
}

bool Query_result_export::prepare(THD *thd, List<Item> &list,
                                  SELECT_LEX_UNIT *u) {
  bool blob_flag = false;
  bool string_results = false, non_string_results = false;
  uncompressed_chunk_size_limit = 0;
  Item *chunk_expr = exchange->compressed_chunk_expr;
  if (chunk_expr) {
    longlong chunk_size_mb;
    if (chunk_expr->type() != Item::INT_ITEM ||
        (chunk_size_mb = chunk_expr->val_int()) < 0 ||
        chunk_size_mb > max_chunk_limit_mb) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), "COMPRESSED");
      return true;
    }
    uncompressed_chunk_size_limit = chunk_size_mb * 1024 * 1024;
  }

  unit = u;
  if (strlen(exchange->file_name) + NAME_LEN >= FN_REFLEN)
    strmake(path, exchange->file_name, FN_REFLEN - 1);

  write_cs = exchange->cs ? exchange->cs : &my_charset_bin;

  /* Check if there is any blobs in data */
  {
    List_iterator_fast<Item> li(list);
    Item *item;
    while ((item = li++)) {
      if (item->max_length >= MAX_BLOB_WIDTH) {
        blob_flag = true;
        break;
      }
      if (item->result_type() == STRING_RESULT)
        string_results = true;
      else
        non_string_results = true;
    }
  }
  if (exchange->field.escaped->numchars() > 1 ||
      exchange->field.enclosed->numchars() > 1) {
    my_error(ER_WRONG_FIELD_TERMINATORS, MYF(0));
    return true;
  }
  if (exchange->field.escaped->length() > 1 ||
      exchange->field.enclosed->length() > 1 ||
      !my_isascii(exchange->field.escaped->ptr()[0]) ||
      !my_isascii(exchange->field.enclosed->ptr()[0]) ||
      !exchange->field.field_term->is_ascii() ||
      !exchange->line.line_term->is_ascii() ||
      !exchange->line.line_start->is_ascii()) {
    /*
      Current LOAD DATA INFILE recognizes field/line separators "as is" without
      converting from client charset to data file charset. So, it is supposed,
      that input file of LOAD DATA INFILE consists of data in one charset and
      separators in other charset. For the compatibility with that [buggy]
      behaviour SELECT INTO OUTFILE implementation has been saved "as is" too,
      but the new warning message has been added:

        Non-ASCII separator arguments are not fully supported
    */
    push_warning(thd, Sql_condition::SL_WARNING,
                 WARN_NON_ASCII_SEPARATOR_NOT_IMPLEMENTED,
                 ER_THD(thd, WARN_NON_ASCII_SEPARATOR_NOT_IMPLEMENTED));
  }
  field_term_length = exchange->field.field_term->length();
  field_term_char = field_term_length
                        ? (int)(uchar)(*exchange->field.field_term)[0]
                        : INT_MAX;
  if (!exchange->line.line_term->length())
    exchange->line.line_term =
        exchange->field.field_term;  // Use this if it exists
  field_sep_char = (exchange->field.enclosed->length()
                        ? (int)(uchar)(*exchange->field.enclosed)[0]
                        : field_term_char);
  if (exchange->field.escaped->length() &&
      (exchange->escaped_given() ||
       !(thd->variables.sql_mode & MODE_NO_BACKSLASH_ESCAPES)))
    escape_char = (int)(uchar)(*exchange->field.escaped)[0];
  else
    escape_char = -1;
  is_ambiguous_field_sep = (strchr(ESCAPE_CHARS, field_sep_char) != nullptr);
  is_unsafe_field_sep = (strchr(NUMERIC_CHARS, field_sep_char) != nullptr);
  line_sep_char = (exchange->line.line_term->length()
                       ? (int)(uchar)(*exchange->line.line_term)[0]
                       : INT_MAX);
  if (!field_term_length) exchange->field.opt_enclosed = false;
  if (!exchange->field.enclosed->length())
    exchange->field.opt_enclosed = true;  // A little quicker loop
  fixed_row_size =
      (!field_term_length && !exchange->field.enclosed->length() && !blob_flag);
  if ((is_ambiguous_field_sep && exchange->field.enclosed->is_empty() &&
       (string_results || is_unsafe_field_sep)) ||
      (exchange->field.opt_enclosed && non_string_results &&
       field_term_length && strchr(NUMERIC_CHARS, field_term_char))) {
    push_warning(thd, Sql_condition::SL_WARNING, ER_AMBIGUOUS_FIELD_TERM,
                 ER_THD(thd, ER_AMBIGUOUS_FIELD_TERM));
    is_ambiguous_field_term = true;
  } else
    is_ambiguous_field_term = false;

  return false;
}

bool Query_result_export::start_execution(THD *thd) {
  if (exchange->compressed_chunk_expr) {
    /*
      If compression was specified, create a new file with
      name <filename>.0.zst.
    */
    if (open_new_compressed_file(thd)) return true;
  } else {
    /*
      If compression was not specified, just create a single
      file and dump data in it. There will be no chunking.
    */
    if ((file = create_file(thd, path, exchange->file_name,
                            false /* compressed */, &cache)) < 0)
      return true;
  }

  return false;
}

int Query_result_export::write_io_cache(const uchar *buf, size_t length) {
  int write_err = my_b_write(&cache, buf, length);
  uncompressed_chunk_size_current += (write_err == 0 ? length : 0);
  return write_err;
}

#define NEED_ESCAPING(x)                              \
  ((int)(uchar)(x) == escape_char ||                  \
   (enclosed ? (int)(uchar)(x) == field_sep_char      \
             : (int)(uchar)(x) == field_term_char) || \
   (int)(uchar)(x) == line_sep_char || !(x))

bool Query_result_export::send_data(THD *thd, List<Item> &items) {
  DBUG_TRACE;
  char buff[MAX_FIELD_WIDTH], null_buff[2], space[MAX_FIELD_WIDTH];
  char cvt_buff[MAX_FIELD_WIDTH];
  String cvt_str(cvt_buff, sizeof(cvt_buff), write_cs);
  bool space_inited = false;
  String tmp(buff, sizeof(buff), &my_charset_bin), *res;
  tmp.length(0);

  row_count++;
  Item *item;
  size_t used_length = 0;
  uint items_left = items.elements;
  List_iterator_fast<Item> li(items);

  /*
    Check if we have crossed the chunk size limit. Open a new file
    if that is the case.
    In case of special value of '0' to COMPRESSED, we do not implement
    chunking.
   */
  if (uncompressed_chunk_size_limit > 0 &&
      uncompressed_chunk_size_current >= uncompressed_chunk_size_limit) {
    /* close file handle of previous file */
    close_file_handle();
    uncompressed_chunk_size_current = 0;
    if (open_new_compressed_file(thd)) goto err;
  }

  if (write_io_cache(
          pointer_cast<const uchar *>(exchange->line.line_start->ptr()),
          exchange->line.line_start->length()))
    goto err;
  while ((item = li++)) {
    Item_result result_type = item->result_type();
    bool enclosed =
        (exchange->field.enclosed->length() &&
         (!exchange->field.opt_enclosed || result_type == STRING_RESULT));
    res = item->val_str(&tmp);
    if (res && !my_charset_same(write_cs, res->charset()) &&
        !my_charset_same(write_cs, &my_charset_bin)) {
      const char *well_formed_error_pos;
      const char *cannot_convert_error_pos;
      const char *from_end_pos;
      const char *error_pos;
      size_t bytes;
      uint64 estimated_bytes =
          ((uint64)res->length() / res->charset()->mbminlen + 1) *
              write_cs->mbmaxlen +
          1;
      estimated_bytes = std::min(estimated_bytes, uint64(UINT_MAX32));
      if (cvt_str.mem_realloc((uint32)estimated_bytes)) {
        my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), (uint32)estimated_bytes);
        goto err;
      }

      bytes = well_formed_copy_nchars(
          write_cs, cvt_str.ptr(), cvt_str.alloced_length(), res->charset(),
          res->ptr(), res->length(),
          UINT_MAX32,  // copy all input chars,
                       // i.e. ignore nchars parameter
          &well_formed_error_pos, &cannot_convert_error_pos, &from_end_pos);
      error_pos = well_formed_error_pos ? well_formed_error_pos
                                        : cannot_convert_error_pos;
      if (error_pos) {
        char printable_buff[32];
        convert_to_printable(printable_buff, sizeof(printable_buff), error_pos,
                             res->ptr() + res->length() - error_pos,
                             res->charset(), 6);
        push_warning_printf(thd, Sql_condition::SL_WARNING,
                            ER_TRUNCATED_WRONG_VALUE_FOR_FIELD,
                            ER_THD(thd, ER_TRUNCATED_WRONG_VALUE_FOR_FIELD),
                            "string", printable_buff, item->item_name.ptr(),
                            static_cast<long>(row_count));
      } else if (from_end_pos < res->ptr() + res->length()) {
        /*
          result is longer than UINT_MAX32 and doesn't fit into String
        */
        push_warning_printf(thd, Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED,
                            ER_THD(thd, WARN_DATA_TRUNCATED), item->full_name(),
                            static_cast<long>(row_count));
      }
      cvt_str.length(bytes);
      res = &cvt_str;
    }
    if (res && enclosed) {
      if (my_b_write(
              &cache,
              pointer_cast<const uchar *>(exchange->field.enclosed->ptr()),
              exchange->field.enclosed->length()))
        goto err;
    }
    if (!res) {  // NULL
      if (!fixed_row_size) {
        if (escape_char != -1)  // Use \N syntax
        {
          null_buff[0] = escape_char;
          null_buff[1] = 'N';
          if (write_io_cache((uchar *)null_buff, 2)) goto err;
        } else if (write_io_cache(pointer_cast<const uchar *>("NULL"), 4))
          goto err;
      } else {
        used_length = 0;  // Fill with space
      }
    } else {
      if (fixed_row_size)
        used_length = min<size_t>(res->length(), item->max_length);
      else
        used_length = res->length();
      if ((result_type == STRING_RESULT || is_unsafe_field_sep) &&
          escape_char != -1) {
        const char *pos, *start, *end;
        bool escape_4_bytes = false;
        int in_escapable_4_bytes = 0;
        const CHARSET_INFO *res_charset = res->charset();
        const CHARSET_INFO *character_set_client =
            thd->variables.character_set_client;
        bool check_following_byte =
            (res_charset == &my_charset_bin) &&
            character_set_client->escape_with_backslash_is_dangerous;
        /*
          The judgement of mbmaxlenlen == 2 is for gb18030 only.
          Since there are several charsets with mbmaxlen == 4,
          so we have to use mbmaxlenlen == 2 here, which is only true
          for gb18030 currently.
        */
        DBUG_ASSERT(character_set_client->mbmaxlen == 2 ||
                    my_mbmaxlenlen(character_set_client) == 2 ||
                    !character_set_client->escape_with_backslash_is_dangerous);
        for (start = pos = res->ptr(), end = pos + used_length; pos != end;
             pos++) {
          bool need_escape = false;
          if (use_mb(res_charset)) {
            int l;
            if ((l = my_ismbchar(res_charset, pos, end))) {
              pos += l - 1;
              continue;
            }
          }

          /*
            Special case when dumping BINARY/VARBINARY/BLOB values
            for the clients with character sets big5, cp932, gbk, sjis
            and gb18030, which can have the escape character
            (0x5C "\" by default) as the second byte of a multi-byte sequence.

            The escape character had better be single-byte character,
            non-ASCII characters are not prohibited, but not fully supported.

            If
            - pos[0] is a valid multi-byte head (e.g 0xEE) and
            - pos[1] is 0x00, which will be escaped as "\0",

            then we'll get "0xEE + 0x5C + 0x30" in the output file.

            If this file is later loaded using this sequence of commands:

            mysql> create table t1 (a varchar(128)) character set big5;
            mysql> LOAD DATA INFILE 'dump.txt' INTO TABLE t1;

            then 0x5C will be misinterpreted as the second byte
            of a multi-byte character "0xEE + 0x5C", instead of
            escape character for 0x00.

            To avoid this confusion, we'll escape the multi-byte
            head character too, so the sequence "0xEE + 0x00" will be
            dumped as "0x5C + 0xEE + 0x5C + 0x30".

            Note, in the condition below we only check if
            mbcharlen is equal to 2, because there are no
            character sets with mbmaxlen longer than 2
            and with escape_with_backslash_is_dangerous set.
            DBUG_ASSERT before the loop makes that sure.

            But gb18030 is an exception. First of all, 2-byte codes
            would be affected by the issue above without doubt.
            Then, 4-byte gb18030 codes would be affected as well.

            Supposing the input is GB+81358130, and the
            field_term_char is set to '5', escape char is 0x5C by default.
            When we come to the first byte 0x81, if we don't escape it but
            escape the second byte 0x35 as it's the field_term_char,
            we would get 0x81 0x5C 0x35 0x81 0x30 for the gb18030 character.
            That would be the same issue as mentioned above.

            Also, if we just escape the leading 2 bytes, we would get
            0x5C 0x81 0x5C 0x35 0x81 0x30 in this case.
            The reader of this sequence would assume that 0x81 0x30
            is the starting of a new gb18030 character, which would
            result in further confusion.

            Once we find any byte of the 4-byte gb18030 character should
            be escaped, we have to escape all the 4 bytes.
            So for GB+81358130, we will get:
            0x5C 0x81 0x5C 0x35 0x5C 0x81 0x30

            The byte 0x30 shouldn't be escaped(no matter it's the second
            or fourth byte in the sequence), since '\0' would be treated
            as 0x00, which is not what we expect. And 0x30 would be treated as
            an ASCII char when we read it, which is correct.
          */

          DBUG_ASSERT(in_escapable_4_bytes >= 0);
          if (in_escapable_4_bytes > 0) {
            DBUG_ASSERT(check_following_byte);
            /* We should escape or not escape all the 4 bytes. */
            need_escape = escape_4_bytes;
          } else if (NEED_ESCAPING(*pos)) {
            need_escape = true;
            if (my_mbmaxlenlen(character_set_client) == 2 &&
                my_mbcharlen_ptr(character_set_client, pos, end) == 4) {
              in_escapable_4_bytes = 4;
              escape_4_bytes = true;
            }
          } else if (check_following_byte) {
            int len = my_mbcharlen_ptr(character_set_client, pos, end);
            if (len == 2 && pos + 1 < end && NEED_ESCAPING(pos[1]))
              need_escape = true;
            else if (len == 4 && my_mbmaxlenlen(character_set_client) == 2 &&
                     pos + 3 < end) {
              in_escapable_4_bytes = 4;
              escape_4_bytes = (NEED_ESCAPING(pos[1]) ||
                                NEED_ESCAPING(pos[2]) || NEED_ESCAPING(pos[3]));
              need_escape = escape_4_bytes;
            }
          }
          /* Mark how many coming bytes should be escaped, only for gb18030 */
          if (in_escapable_4_bytes > 0) {
            in_escapable_4_bytes--;
            /*
             Note that '0' (0x30) in the middle of a 4-byte sequence
             can't be escaped. Please read more details from above comments.
             2-byte codes won't be affected by this issue.
            */
            if (pos[0] == 0x30) need_escape = false;
          }

          if (need_escape &&
              /*
               Don't escape field_term_char by doubling - doubling is only
               valid for ENCLOSED BY characters:
              */
              (enclosed || !is_ambiguous_field_term ||
               (int)(uchar)*pos != field_term_char)) {
            char tmp_buff[2];
            tmp_buff[0] =
                ((int)(uchar)*pos == field_sep_char && is_ambiguous_field_sep)
                    ? field_sep_char
                    : escape_char;
            tmp_buff[1] = *pos ? *pos : '0';
            if (write_io_cache(pointer_cast<const uchar *>(start),
                               (uint)(pos - start)) ||
                write_io_cache((uchar *)tmp_buff, 2))
              goto err;
            start = pos + 1;
          }
        }

        /* Assert that no escape mode is active here */
        DBUG_ASSERT(in_escapable_4_bytes == 0);

        if (write_io_cache(pointer_cast<const uchar *>(start),
                           (uint)(pos - start)))
          goto err;
      } else if (write_io_cache((uchar *)res->ptr(), used_length))
        goto err;
    }
    if (fixed_row_size) {  // Fill with space
      if (item->max_length > used_length) {
        /* QQ:  Fix by adding a my_b_fill() function */
        if (!space_inited) {
          space_inited = true;
          memset(space, ' ', sizeof(space));
        }
        size_t length = item->max_length - used_length;
        for (; length > sizeof(space); length -= sizeof(space)) {
          if (write_io_cache((uchar *)space, sizeof(space))) goto err;
        }
        if (write_io_cache((uchar *)space, length)) goto err;
      }
    }
    if (res && enclosed) {
      if (my_b_write(
              &cache,
              pointer_cast<const uchar *>(exchange->field.enclosed->ptr()),
              exchange->field.enclosed->length()))
        goto err;
    }
    if (--items_left) {
      if (my_b_write(
              &cache,
              pointer_cast<const uchar *>(exchange->field.field_term->ptr()),
              field_term_length))
        goto err;
    }
  }
  if (write_io_cache(
          pointer_cast<const uchar *>(exchange->line.line_term->ptr()),
          exchange->line.line_term->length()))
    goto err;

  /* fsync the file after every select_into_file_fsync_size bytes
       optionally sleep */
  if (thd->variables.select_into_file_fsync_size != 0) {
    my_off_t cur_fsize = my_b_tell(&cache);
    if (cur_fsize - last_fsync_off >=
        thd->variables.select_into_file_fsync_size) {
      if (flush_io_cache(&cache) || mysql_file_sync(cache.file, MYF(MY_WME)))
        goto err;
#ifndef DBUG_OFF
      n_fsyncs++;
#endif
      last_fsync_off = cur_fsize;
      if (thd->variables.select_into_file_fsync_timeout)
        my_sleep(thd->variables.select_into_file_fsync_timeout * 1000);
    }
  }

  return false;
err:
  return true;
}

void Query_result_export::cleanup(THD *thd) {
  DBUG_EXECUTE_IF("print_select_file_fsync_stats", {
    // NO_LINT_DEBUG
    fprintf(stderr, "[select_to_file][fsync_count] %u\n", n_fsyncs);
  });

  thd->set_sent_row_count(row_count);
  Query_result_to_file::cleanup(thd);
}

/***************************************************************************
** Dump of query to a binary file
***************************************************************************/

bool Query_result_dump::prepare(THD *, List<Item> &, SELECT_LEX_UNIT *u) {
  unit = u;
  return false;
}

bool Query_result_dump::start_execution(THD *thd) {
  if ((file = create_file(thd, path, exchange->file_name,
                          false /* compressed */, &cache)) < 0)
    return true;
  return false;
}

bool Query_result_dump::send_data(THD *, List<Item> &items) {
  List_iterator_fast<Item> li(items);
  char buff[MAX_FIELD_WIDTH];
  String tmp(buff, sizeof(buff), &my_charset_bin), *res;
  tmp.length(0);
  Item *item;
  DBUG_TRACE;

  if (row_count++ > 1) {
    my_error(ER_TOO_MANY_ROWS, MYF(0));
    goto err;
  }
  while ((item = li++)) {
    res = item->val_str(&tmp);
    if (!res)  // If NULL
    {
      if (my_b_write(&cache, pointer_cast<const uchar *>(""), 1)) goto err;
    } else if (my_b_write(&cache, (uchar *)res->ptr(), res->length())) {
      char errbuf[MYSYS_STRERROR_SIZE];
      my_error(ER_ERROR_ON_WRITE, MYF(0), path, my_errno(),
               my_strerror(errbuf, sizeof(errbuf), my_errno()));
      goto err;
    }
  }
  return false;
err:
  return true;
}

/***************************************************************************
  Dump of select to variables
***************************************************************************/

bool Query_dumpvar::prepare(THD *, List<Item> &list, SELECT_LEX_UNIT *u) {
  unit = u;

  if (var_list.elements != list.elements) {
    my_error(ER_WRONG_NUMBER_OF_COLUMNS_IN_SELECT, MYF(0));
    return true;
  }

  return false;
}

bool Query_dumpvar::check_simple_select() const {
  my_error(ER_SP_BAD_CURSOR_SELECT, MYF(0));
  return true;
}

bool Query_dumpvar::send_data(THD *thd, List<Item> &items) {
  List_iterator_fast<PT_select_var> var_li(var_list);
  List_iterator<Item> it(items);
  Item *item;
  PT_select_var *mv;
  DBUG_TRACE;

  if (row_count++) {
    my_error(ER_TOO_MANY_ROWS, MYF(0));
    return true;
  }
  while ((mv = var_li++) && (item = it++)) {
    if (mv->is_local()) {
      if (thd->sp_runtime_ctx->set_variable(thd, mv->get_offset(), &item))
        return true;
    } else {
      /*
        Create Item_func_set_user_vars with delayed non-constness. We
        do this so that Item_get_user_var::const_item() will return
        the same result during
        Item_func_set_user_var::save_item_result() as they did during
        optimization and execution.
       */
      Item_func_set_user_var *suv =
          new Item_func_set_user_var(mv->name, item, true);
      if (suv->fix_fields(thd, nullptr)) return true;
      suv->save_item_result(item);
      if (suv->update()) return true;
    }
  }
  return thd->is_error();
}

bool Query_dumpvar::send_eof(THD *thd) {
  if (!row_count)
    push_warning(thd, Sql_condition::SL_WARNING, ER_SP_FETCH_NO_DATA,
                 ER_THD(thd, ER_SP_FETCH_NO_DATA));
  /*
    Don't send EOF if we're in error condition (which implies we've already
    sent or are sending an error)
  */
  if (thd->is_error()) return true;

  ::my_ok(thd, row_count);
  return false;
}
