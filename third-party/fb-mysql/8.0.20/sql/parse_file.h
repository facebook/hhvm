/* Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _PARSE_FILE_H_
#define _PARSE_FILE_H_

#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_inttypes.h"

struct MEM_ROOT;

#define PARSE_FILE_TIMESTAMPLENGTH 19

enum file_opt_type {
  FILE_OPTIONS_STRING,    /**< String (LEX_STRING) */
  FILE_OPTIONS_ESTRING,   /**< Escaped string (LEX_STRING) */
  FILE_OPTIONS_ULONGLONG, /**< ulonglong parameter (ulonglong) */
  FILE_OPTIONS_TIMESTAMP, /**< timestamp (LEX_STRING have to be
                             allocated with length 20 (19+1) */
  FILE_OPTIONS_STRLIST,   /**< list of escaped strings
                             (List<LEX_STRING>) */
  FILE_OPTIONS_ULLLIST    /**< list of ulonglong values
                             (List<ulonglong>) */
};

struct File_option {
  LEX_CSTRING name;   /**< Name of the option */
  size_t offset;      /**< offset to base address of value */
  file_opt_type type; /**< Option type */
};

/**
  This hook used to catch no longer supported keys and process them for
  backward compatibility.
*/

class Unknown_key_hook {
 public:
  virtual ~Unknown_key_hook() = default;
  virtual bool process_unknown_string(const char *&unknown_key, uchar *base,
                                      MEM_ROOT *mem_root, const char *end) = 0;
};

/**
  Dummy hook for parsers which do not need hook for unknown keys.
*/
class File_parser_dummy_hook : public Unknown_key_hook {
 public:
  bool process_unknown_string(const char *&unknown_key, uchar *, MEM_ROOT *,
                              const char *) override;
};

extern File_parser_dummy_hook file_parser_dummy_hook;

bool get_file_options_ulllist(const char *&ptr, const char *end,
                              const char *line, uchar *base,
                              File_option *parameter, MEM_ROOT *mem_root);

class File_parser;

File_parser *sql_parse_prepare(const LEX_STRING *file_name, MEM_ROOT *mem_root,
                               bool bad_format_errors);

class File_parser {
  const char *start, *end;
  LEX_CSTRING file_type;
  bool content_ok;

 public:
  File_parser() : start(nullptr), end(nullptr), content_ok(false) {
    file_type.str = nullptr;
    file_type.length = 0;
  }

  bool ok() { return content_ok; }
  const LEX_CSTRING &type() const { return file_type; }
  bool parse(uchar *base, MEM_ROOT *mem_root, struct File_option *parameters,
             uint required, Unknown_key_hook *hook) const;

  friend File_parser *sql_parse_prepare(const LEX_STRING *file_name,
                                        MEM_ROOT *mem_root,
                                        bool bad_format_errors);
};
#endif /* _PARSE_FILE_H_ */
