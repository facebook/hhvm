/* Copyright (c) 2000, 2017, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA  */

#ifndef _my_xml_h
#define _my_xml_h

/**
  @file include/my_xml.h
*/

#include <stddef.h>
#include <sys/types.h>

#define MY_XML_OK 0
#define MY_XML_ERROR 1

/*
  A flag whether to use absolute tag names in call-back functions,
  like "a", "a.b" and "a.b.c" (used in character set file parser),
  or relative names like "a", "b" and "c".
*/
#define MY_XML_FLAG_RELATIVE_NAMES 1

/*
  A flag whether to skip normilization of text values before calling
  call-back functions: i.e. skip leading/trailing spaces,
  \r, \n, \t characters.
*/
#define MY_XML_FLAG_SKIP_TEXT_NORMALIZATION 2

enum my_xml_node_type {
  MY_XML_NODE_TAG,  /* can have TAG, ATTR and TEXT children */
  MY_XML_NODE_ATTR, /* can have TEXT children               */
  MY_XML_NODE_TEXT  /* cannot have children                 */
};

struct MY_XML_PARSER {
  int flags;
  enum my_xml_node_type current_node_type;
  char errstr[128];

  struct {
    char static_buffer[128];
    char *buffer;
    size_t buffer_size;
    char *start;
    char *end;
  } attr;

  const char *beg;
  const char *cur;
  const char *end;
  void *user_data;
  int (*enter)(MY_XML_PARSER *st, const char *val, size_t len);
  int (*value)(MY_XML_PARSER *st, const char *val, size_t len);
  int (*leave_xml)(MY_XML_PARSER *st, const char *val, size_t len);
};

void my_xml_parser_create(MY_XML_PARSER *st);
void my_xml_parser_free(MY_XML_PARSER *st);
int my_xml_parse(MY_XML_PARSER *st, const char *str, size_t len);

void my_xml_set_value_handler(MY_XML_PARSER *st,
                              int (*)(MY_XML_PARSER *, const char *,
                                      size_t len));
void my_xml_set_enter_handler(MY_XML_PARSER *st,
                              int (*)(MY_XML_PARSER *, const char *,
                                      size_t len));
void my_xml_set_leave_handler(MY_XML_PARSER *st,
                              int (*)(MY_XML_PARSER *, const char *,
                                      size_t len));
void my_xml_set_user_data(MY_XML_PARSER *st, void *);

size_t my_xml_error_pos(MY_XML_PARSER *st);
uint my_xml_error_lineno(MY_XML_PARSER *st);

const char *my_xml_error_string(MY_XML_PARSER *st);

#endif /* _my_xml_h */
