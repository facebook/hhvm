/* Copyright (c) 2003, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "m_string.h"
#include "my_inttypes.h"
#include "my_xml.h"

#define MY_XML_UNKNOWN 'U'
#define MY_XML_EOF 'E'
#define MY_XML_STRING 'S'
#define MY_XML_IDENT 'I'
#define MY_XML_EQ '='
#define MY_XML_LT '<'
#define MY_XML_GT '>'
#define MY_XML_SLASH '/'
#define MY_XML_COMMENT 'C'
#define MY_XML_TEXT 'T'
#define MY_XML_QUESTION '?'
#define MY_XML_EXCLAM '!'
#define MY_XML_CDATA 'D'

struct MY_XML_ATTR {
  const char *beg;
  const char *end;
};

/*
  XML ctype:
*/
#define MY_XML_ID0 0x01 /* Identifier initial character */
#define MY_XML_ID1 0x02 /* Identifier medial  character */
#define MY_XML_SPC 0x08 /* Spacing character */

/*
 http://www.w3.org/TR/REC-xml/
 [4] NameChar ::= Letter | Digit | '.' | '-' | '_' | ':' |
                  CombiningChar | Extender
 [5] Name ::= (Letter | '_' | ':') (NameChar)*
*/

static char my_xml_ctype[256] = {
    /*00*/ 0, 0, 0, 0, 0, 0, 0, 0,
    0,        8, 8, 0, 0, 8, 0, 0,
    /*10*/ 0, 0, 0, 0, 0, 0, 0, 0,
    0,        0, 0, 0, 0, 0, 0, 0,
    /*20*/ 8, 0, 0, 0, 0, 0, 0, 0,
    0,        0, 0, 0, 0, 2, 2, 0, /*  !"#$%&'()*+,-./ */
    /*30*/ 2, 2, 2, 2, 2, 2, 2, 2,
    2,        2, 3, 0, 0, 0, 0, 0, /* 0123456789:;<=>? */
    /*40*/ 0, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3, /* @ABCDEFGHIJKLMNO */
    /*50*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 0, 0, 0, 0, 3, /* PQRSTUVWXYZ[\]^_ */
    /*60*/ 0, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3, /* `abcdefghijklmno */
    /*70*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 0, 0, 0, 0, 0, /* pqrstuvwxyz{|}~  */
    /*80*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3,
    /*90*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3,
    /*A0*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3,
    /*B0*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3,
    /*C0*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3,
    /*D0*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3,
    /*E0*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3,
    /*F0*/ 3, 3, 3, 3, 3, 3, 3, 3,
    3,        3, 3, 3, 3, 3, 3, 3};

#define my_xml_is_space(c) (my_xml_ctype[(uchar)(c)] & MY_XML_SPC)
#define my_xml_is_id0(c) (my_xml_ctype[(uchar)(c)] & MY_XML_ID0)
#define my_xml_is_id1(c) (my_xml_ctype[(uchar)(c)] & MY_XML_ID1)

static const char *lex2str(int lex) {
  switch (lex) {
    case MY_XML_EOF:
      return "END-OF-INPUT";
    case MY_XML_STRING:
      return "STRING";
    case MY_XML_IDENT:
      return "IDENT";
    case MY_XML_CDATA:
      return "CDATA";
    case MY_XML_EQ:
      return "'='";
    case MY_XML_LT:
      return "'<'";
    case MY_XML_GT:
      return "'>'";
    case MY_XML_SLASH:
      return "'/'";
    case MY_XML_COMMENT:
      return "COMMENT";
    case MY_XML_TEXT:
      return "TEXT";
    case MY_XML_QUESTION:
      return "'?'";
    case MY_XML_EXCLAM:
      return "'!'";
  }
  return "unknown token";
}

static void my_xml_norm_text(MY_XML_ATTR *a) {
  for (; (a->beg < a->end) && my_xml_is_space(a->beg[0]); a->beg++)
    ;
  for (; (a->beg < a->end) && my_xml_is_space(a->end[-1]); a->end--)
    ;
}

static inline bool my_xml_parser_prefix_cmp(MY_XML_PARSER *p, const char *s,
                                            size_t slen) {
  return (p->cur + slen > p->end) || memcmp(p->cur, s, slen);
}

static int my_xml_scan(MY_XML_PARSER *p, MY_XML_ATTR *a) {
  int lex;

  for (; (p->cur < p->end) && my_xml_is_space(p->cur[0]); p->cur++)
    ;

  if (p->cur >= p->end) {
    a->beg = p->end;
    a->end = p->end;
    lex = MY_XML_EOF;
    goto ret;
  }

  a->beg = p->cur;
  a->end = p->cur;

  if (!my_xml_parser_prefix_cmp(p, STRING_WITH_LEN("<!--"))) {
    for (; p->cur < p->end; p->cur++) {
      if (!my_xml_parser_prefix_cmp(p, STRING_WITH_LEN("-->"))) {
        p->cur += 3;
        break;
      }
    }
    a->end = p->cur;
    lex = MY_XML_COMMENT;
  } else if (!my_xml_parser_prefix_cmp(p, STRING_WITH_LEN("<![CDATA["))) {
    p->cur += 9;
    for (; p->cur < p->end - 2; p->cur++) {
      if (p->cur[0] == ']' && p->cur[1] == ']' && p->cur[2] == '>') {
        p->cur += 3;
        a->end = p->cur;
        break;
      }
    }
    lex = MY_XML_CDATA;
  } else if (strchr("?=/<>!", p->cur[0])) {
    p->cur++;
    a->end = p->cur;
    lex = a->beg[0];
  } else if ((p->cur[0] == '"') || (p->cur[0] == '\'')) {
    /*
      "string" or 'string' found.
      Scan until the closing quote/doublequote, or until the END-OF-INPUT.
    */
    p->cur++;
    for (; (p->cur < p->end) && (p->cur[0] != a->beg[0]); p->cur++) {
    }
    a->end = p->cur;
    if (p->cur < p->end) /* Closing quote or doublequote has been found */
      p->cur++;
    a->beg++;
    if (!(p->flags & MY_XML_FLAG_SKIP_TEXT_NORMALIZATION)) my_xml_norm_text(a);
    lex = MY_XML_STRING;
  } else if (my_xml_is_id0(p->cur[0])) {
    p->cur++;
    while (p->cur < p->end && my_xml_is_id1(p->cur[0])) p->cur++;
    a->end = p->cur;
    my_xml_norm_text(a);
    lex = MY_XML_IDENT;
  } else
    lex = MY_XML_UNKNOWN;

#if 0
  printf("LEX=%s[%d]\n",lex2str(lex),a->end-a->beg);
#endif

ret:
  return lex;
}

static int my_xml_value(MY_XML_PARSER *st, const char *str, size_t len) {
  return (st->value) ? (st->value)(st, str, len) : MY_XML_OK;
}

/**
  Ensure the attr buffer is wide enough to hold the new value

  Expand and/or allocate dynamic buffer as needed to hold the concatenated
  path and the terminating zero.

  @param st   the parser instance
  @param len  the length of the attribute to be added
  @return state
  @retval 1  failed
  @retval 0  success
*/
static int my_xml_attr_ensure_space(MY_XML_PARSER *st, size_t len) {
  size_t ofs = st->attr.end - st->attr.start;
  len++;  // Add terminating zero.
  if (ofs + len > st->attr.buffer_size) {
    st->attr.buffer_size = (SIZE_T_MAX - len) / 2 > st->attr.buffer_size
                               ? st->attr.buffer_size * 2 + len
                               : SIZE_T_MAX;

    if (!st->attr.buffer) {
      st->attr.buffer = (char *)my_str_malloc(st->attr.buffer_size);
      if (st->attr.buffer)
        memcpy(st->attr.buffer, st->attr.static_buffer,
               ofs + 1 /*term. zero */);
    } else
      st->attr.buffer =
          (char *)my_str_realloc(st->attr.buffer, st->attr.buffer_size);
    st->attr.start = st->attr.buffer;
    st->attr.end = st->attr.start + ofs;

    return st->attr.buffer ? MY_XML_OK : MY_XML_ERROR;
  }
  return MY_XML_OK;
}

/** rewind the attr buffer to initial state */
static void my_xml_attr_rewind(MY_XML_PARSER *p) {
  /* keep the buffer already allocated */
  p->attr.end = p->attr.start;
}

static int my_xml_enter(MY_XML_PARSER *st, const char *str, size_t len) {
  if (my_xml_attr_ensure_space(st, len + 1 /* the separator char */))
    return MY_XML_ERROR;

  if (st->attr.end > st->attr.start) {
    st->attr.end[0] = '/';
    st->attr.end++;
  }
  memcpy(st->attr.end, str, len);
  st->attr.end += len;
  st->attr.end[0] = '\0';
  if (st->flags & MY_XML_FLAG_RELATIVE_NAMES)
    return st->enter ? st->enter(st, str, len) : MY_XML_OK;
  else
    return st->enter
               ? st->enter(st, st->attr.start, st->attr.end - st->attr.start)
               : MY_XML_OK;
}

static void mstr(char *s, const char *src, size_t l1, size_t l2) {
  l1 = l1 < l2 ? l1 : l2;
  memcpy(s, src, l1);
  s[l1] = '\0';
}

static int my_xml_leave(MY_XML_PARSER *p, const char *str, size_t slen) {
  char *e;
  size_t glen;
  char s[32];
  char g[32];
  int rc;

  /* Find previous '/' or beginning */
  for (e = p->attr.end; (e > p->attr.start) && (e[0] != '/'); e--)
    ;
  glen = (size_t)((e[0] == '/') ? (p->attr.end - e - 1) : p->attr.end - e);

  if (str && (slen != glen)) {
    mstr(s, str, sizeof(s) - 1, slen);
    if (glen) {
      mstr(g, e + 1, sizeof(g) - 1, glen),
          sprintf(p->errstr, "'</%s>' unexpected ('</%s>' wanted)", s, g);
    } else
      sprintf(p->errstr, "'</%s>' unexpected (END-OF-INPUT wanted)", s);
    return MY_XML_ERROR;
  }

  if (p->flags & MY_XML_FLAG_RELATIVE_NAMES)
    rc = p->leave_xml ? p->leave_xml(p, str, slen) : MY_XML_OK;
  else
    rc = (p->leave_xml
              ? p->leave_xml(p, p->attr.start, p->attr.end - p->attr.start)
              : MY_XML_OK);

  *e = '\0';
  p->attr.end = e;

  return rc;
}

int my_xml_parse(MY_XML_PARSER *p, const char *str, size_t len) {
  my_xml_attr_rewind(p);

  p->beg = str;
  p->cur = str;
  p->end = str + len;

  while (p->cur < p->end) {
    MY_XML_ATTR a;
    if (p->cur[0] == '<') {
      int lex;
      int question = 0;
      int exclam = 0;

      lex = my_xml_scan(p, &a);

      if (MY_XML_COMMENT == lex) continue;

      if (lex == MY_XML_CDATA) {
        a.beg += 9;
        a.end -= 3;
        my_xml_value(p, a.beg, (size_t)(a.end - a.beg));
        continue;
      }

      lex = my_xml_scan(p, &a);

      if (MY_XML_SLASH == lex) {
        if (MY_XML_IDENT != (lex = my_xml_scan(p, &a))) {
          sprintf(p->errstr, "%s unexpected (ident wanted)", lex2str(lex));
          return MY_XML_ERROR;
        }
        if (MY_XML_OK != my_xml_leave(p, a.beg, (size_t)(a.end - a.beg)))
          return MY_XML_ERROR;
        lex = my_xml_scan(p, &a);
        goto gt;
      }

      if (MY_XML_EXCLAM == lex) {
        lex = my_xml_scan(p, &a);
        exclam = 1;
      } else if (MY_XML_QUESTION == lex) {
        lex = my_xml_scan(p, &a);
        question = 1;
      }

      if (MY_XML_IDENT == lex) {
        p->current_node_type = MY_XML_NODE_TAG;
        if (MY_XML_OK != my_xml_enter(p, a.beg, (size_t)(a.end - a.beg)))
          return MY_XML_ERROR;
      } else {
        sprintf(p->errstr, "%s unexpected (ident or '/' wanted)", lex2str(lex));
        return MY_XML_ERROR;
      }

      while ((MY_XML_IDENT == (lex = my_xml_scan(p, &a))) ||
             ((MY_XML_STRING == lex && exclam))) {
        MY_XML_ATTR b;
        if (MY_XML_EQ == (lex = my_xml_scan(p, &b))) {
          lex = my_xml_scan(p, &b);
          if ((lex == MY_XML_IDENT) || (lex == MY_XML_STRING)) {
            p->current_node_type = MY_XML_NODE_ATTR;
            if ((MY_XML_OK !=
                 my_xml_enter(p, a.beg, (size_t)(a.end - a.beg))) ||
                (MY_XML_OK !=
                 my_xml_value(p, b.beg, (size_t)(b.end - b.beg))) ||
                (MY_XML_OK != my_xml_leave(p, a.beg, (size_t)(a.end - a.beg))))
              return MY_XML_ERROR;
          } else {
            sprintf(p->errstr, "%s unexpected (ident or string wanted)",
                    lex2str(lex));
            return MY_XML_ERROR;
          }
        } else if (MY_XML_IDENT == lex) {
          p->current_node_type = MY_XML_NODE_ATTR;
          if ((MY_XML_OK != my_xml_enter(p, a.beg, (size_t)(a.end - a.beg))) ||
              (MY_XML_OK != my_xml_leave(p, a.beg, (size_t)(a.end - a.beg))))
            return MY_XML_ERROR;
        } else if ((MY_XML_STRING == lex) && exclam) {
          /*
            We are in <!DOCTYPE>, e.g.
            <!DOCTYPE name SYSTEM "SystemLiteral">
            <!DOCTYPE name PUBLIC "PublidLiteral" "SystemLiteral">
            Just skip "SystemLiteral" and "PublicidLiteral"
          */
        } else
          break;
      }

      if (lex == MY_XML_SLASH) {
        if (MY_XML_OK != my_xml_leave(p, nullptr, 0)) return MY_XML_ERROR;
        lex = my_xml_scan(p, &a);
      }

    gt:
      if (question) {
        if (lex != MY_XML_QUESTION) {
          sprintf(p->errstr, "%s unexpected ('?' wanted)", lex2str(lex));
          return MY_XML_ERROR;
        }
        if (MY_XML_OK != my_xml_leave(p, nullptr, 0)) return MY_XML_ERROR;
        lex = my_xml_scan(p, &a);
      }

      if (exclam) {
        if (MY_XML_OK != my_xml_leave(p, nullptr, 0)) return MY_XML_ERROR;
      }

      if (lex != MY_XML_GT) {
        sprintf(p->errstr, "%s unexpected ('>' wanted)", lex2str(lex));
        return MY_XML_ERROR;
      }
    } else {
      a.beg = p->cur;
      for (; (p->cur < p->end) && (p->cur[0] != '<'); p->cur++)
        ;
      a.end = p->cur;

      if (!(p->flags & MY_XML_FLAG_SKIP_TEXT_NORMALIZATION))
        my_xml_norm_text(&a);
      if (a.beg != a.end) {
        my_xml_value(p, a.beg, (size_t)(a.end - a.beg));
      }
    }
  }

  if (p->attr.start[0]) {
    sprintf(p->errstr, "unexpected END-OF-INPUT");
    return MY_XML_ERROR;
  }
  return MY_XML_OK;
}

void my_xml_parser_create(MY_XML_PARSER *p) {
  memset(p, 0, sizeof(p[0]));
  /*
    Use static buffer while it's sufficient.
  */
  p->attr.start = p->attr.end = p->attr.static_buffer;
  p->attr.buffer_size = sizeof(p->attr.static_buffer);
}

void my_xml_parser_free(MY_XML_PARSER *p) {
  if (p->attr.buffer) {
    my_str_free(p->attr.buffer);
    p->attr.buffer = nullptr;
  }
}

void my_xml_set_value_handler(MY_XML_PARSER *p,
                              int (*action)(MY_XML_PARSER *p, const char *s,
                                            size_t l)) {
  p->value = action;
}

void my_xml_set_enter_handler(MY_XML_PARSER *p,
                              int (*action)(MY_XML_PARSER *p, const char *s,
                                            size_t l)) {
  p->enter = action;
}

void my_xml_set_leave_handler(MY_XML_PARSER *p,
                              int (*action)(MY_XML_PARSER *p, const char *s,
                                            size_t l)) {
  p->leave_xml = action;
}

void my_xml_set_user_data(MY_XML_PARSER *p, void *user_data) {
  p->user_data = user_data;
}

const char *my_xml_error_string(MY_XML_PARSER *p) { return p->errstr; }

size_t my_xml_error_pos(MY_XML_PARSER *p) {
  const char *beg = p->beg;
  const char *s;
  for (s = p->cur - 1; s > p->beg - 1; s--) {
    if (s[0] == '\n') {
      beg = s;
      break;
    }
  }
  return (size_t)(p->cur - beg);
}

uint my_xml_error_lineno(MY_XML_PARSER *p) {
  uint res = 0;
  const char *s;
  for (s = p->beg; s < p->cur; s++) {
    if (s[0] == '\n') res++;
  }
  return res;
}
