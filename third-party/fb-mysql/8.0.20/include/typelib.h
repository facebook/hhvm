/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _typelib_h
#define _typelib_h

/**
  @file include/typelib.h
*/

#include "my_inttypes.h"

struct MEM_ROOT;

struct TYPELIB {             /* Different types saved here */
  size_t count{0};           /* How many types */
  const char *name{nullptr}; /* Name of typelib */
  const char **type_names{nullptr};
  unsigned int *type_lengths{nullptr};
};

extern uint64_t find_typeset(const char *x, TYPELIB *typelib,
                             int *error_position);
extern int find_type_or_exit(const char *x, TYPELIB *typelib,
                             const char *option);
#define FIND_TYPE_BASIC 0
/** makes @c find_type() require the whole name, no prefix */
#define FIND_TYPE_NO_PREFIX (1 << 0)
/** always implicitely on, so unused, but old code may pass it */
#define FIND_TYPE_NO_OVERWRITE (1 << 1)
/** makes @c find_type() accept a number */
#define FIND_TYPE_ALLOW_NUMBER (1 << 2)
/** makes @c find_type() treat ',' as terminator */
#define FIND_TYPE_COMMA_TERM (1 << 3)

extern int find_type(const char *x, const TYPELIB *typelib, unsigned int flags);
extern const char *get_type(TYPELIB *typelib, unsigned int nr);
extern TYPELIB *copy_typelib(MEM_ROOT *root, TYPELIB *from);

extern TYPELIB sql_protocol_typelib;

uint64_t find_set_from_flags(const TYPELIB *lib, size_t default_name,
                             uint64_t cur_set, uint64_t default_set,
                             const char *str, unsigned int length,
                             const char **err_pos, unsigned int *err_len,
                             bool dedup = false);

#endif /* _typelib_h */
