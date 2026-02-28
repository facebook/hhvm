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

/**
  @file mysys/typelib.cc
  Functions to handle typelib
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "typelib.h"

#define is_field_separator(X) ((X) == ',' || (X) == '=')

int find_type_or_exit(const char *x, TYPELIB *typelib, const char *option) {
  int res;
  const char **ptr;

  if ((res = find_type(x, typelib, FIND_TYPE_BASIC)) <= 0) {
    ptr = typelib->type_names;
    if (!*x)
      fprintf(stderr, "No option given to %s\n", option);
    else
      fprintf(stderr, "Unknown option to %s: %s\n", option, x);
    fprintf(stderr, "Alternatives are: '%s'", *ptr);
    while (*++ptr) fprintf(stderr, ",'%s'", *ptr);
    fprintf(stderr, "\n");
    exit(1);
  }
  return res;
}

/**
  Search after a string in a list of strings. Endspace in x is not compared.

  @param x              String to find
  @param typelib        TYPELIB (struct of pointer to values + count)
  @param flags          flags to tune behaviour: a combination of
                        FIND_TYPE_NO_PREFIX
                        FIND_TYPE_ALLOW_NUMBER
                        FIND_TYPE_COMMA_TERM.
                        FIND_TYPE_NO_OVERWRITE can be passed but is
                        superfluous (is always implicitely on).

  @retval
    -1  Too many matching values
  @retval
    0   No matching value
  @retval
    >0  Offset+1 in typelib for matched string
*/

int find_type(const char *x, const TYPELIB *typelib, uint flags) {
  int find, pos;
  int findpos = 0; /* guarded by find */
  const char *i;
  const char *j;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("x: '%s'  lib: %p", x, typelib));

  DBUG_ASSERT(!(flags & ~(FIND_TYPE_NO_PREFIX | FIND_TYPE_ALLOW_NUMBER |
                          FIND_TYPE_NO_OVERWRITE | FIND_TYPE_COMMA_TERM)));
  if (!typelib->count) {
    DBUG_PRINT("exit", ("no count"));
    return 0;
  }
  find = 0;
  for (pos = 0; (j = typelib->type_names[pos]); pos++) {
    for (i = x;
         *i && (!(flags & FIND_TYPE_COMMA_TERM) || !is_field_separator(*i)) &&
         my_toupper(&my_charset_latin1, *i) ==
             my_toupper(&my_charset_latin1, *j);
         i++, j++)
      ;
    if (!*j) {
      while (*i == ' ') i++; /* skip_end_space */
      if (!*i || ((flags & FIND_TYPE_COMMA_TERM) && is_field_separator(*i)))
        return pos + 1;
    }
    if ((!*i && (!(flags & FIND_TYPE_COMMA_TERM) || !is_field_separator(*i))) &&
        (!*j || !(flags & FIND_TYPE_NO_PREFIX))) {
      find++;
      findpos = pos;
    }
  }
  if (find == 0 && (flags & FIND_TYPE_ALLOW_NUMBER) && x[0] == '#' &&
      strend(x)[-1] == '#' && (findpos = atoi(x + 1) - 1) >= 0 &&
      (uint)findpos < typelib->count)
    find = 1;
  else if (find == 0 || !x[0]) {
    DBUG_PRINT("exit", ("Couldn't find type"));
    return 0;
  } else if (find != 1 || (flags & FIND_TYPE_NO_PREFIX)) {
    DBUG_PRINT("exit", ("Too many possybilities"));
    return -1;
  }
  return findpos + 1;
} /* find_type */

/**
  Get type

  @note
  first type is 0
*/

const char *get_type(TYPELIB *typelib, uint nr) {
  if (nr < (uint)typelib->count && typelib->type_names)
    return (typelib->type_names[nr]);
  return "?";
}

/**
  Create an integer value to represent the supplied comma-seperated
  string where each string in the TYPELIB denotes a bit position.

  @param x      string to decompose
  @param lib    TYPELIB (struct of pointer to values + count)
  @param err    index (not char position) of string element which was not
                found or 0 if there was no error

  @retval
    a integer representation of the supplied string
*/

uint64_t find_typeset(const char *x, TYPELIB *lib, int *err) {
  uint64_t result;
  int find;
  const char *i;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("x: '%s'  lib: %p", x, lib));

  if (!lib->count) {
    DBUG_PRINT("exit", ("no count"));
    return 0;
  }
  result = 0;
  *err = 0;
  while (*x) {
    (*err)++;
    i = x;
    while (*x && !is_field_separator(*x)) x++;
    if (x[0] && x[1]) /* skip separator if found */
      x++;
    if ((find = find_type(i, lib, FIND_TYPE_COMMA_TERM) - 1) < 0) return 0;
    result |= (1ULL << find);
  }
  *err = 0;
  return result;
} /* find_set */

/**
  Create a copy of a specified TYPELIB structure.

  @param root   pointer to a MEM_ROOT object for allocations
  @param from   pointer to a source TYPELIB structure

  @retval
    pointer to the new TYPELIB structure on successful copy
  @retval
    NULL otherwise
*/

TYPELIB *copy_typelib(MEM_ROOT *root, TYPELIB *from) {
  TYPELIB *to;
  uint i;

  if (!from) return nullptr;

  if (!(to = (TYPELIB *)root->Alloc(sizeof(TYPELIB)))) return nullptr;

  if (!(to->type_names = (const char **)root->Alloc(
            (sizeof(char *) + sizeof(int)) * (from->count + 1))))
    return nullptr;
  to->type_lengths = (unsigned int *)(to->type_names + from->count + 1);
  to->count = from->count;
  if (from->name) {
    if (!(to->name = strdup_root(root, from->name))) return nullptr;
  } else
    to->name = nullptr;

  for (i = 0; i < from->count; i++) {
    if (!(to->type_names[i] =
              strmake_root(root, from->type_names[i], from->type_lengths[i])))
      return nullptr;
    to->type_lengths[i] = from->type_lengths[i];
  }
  to->type_names[to->count] = nullptr;
  to->type_lengths[to->count] = 0;

  return to;
}

static const char *on_off_default_names[] = {"off", "on", "default", nullptr};
static TYPELIB on_off_default_typelib = {
    array_elements(on_off_default_names) - 1, "", on_off_default_names,
    nullptr};

/**
  Parse a TYPELIB name from the buffer

  @param lib          Set of names to scan for.
  @param strpos INOUT Start of the buffer (updated to point to the next
                      character after the name)
  @param end          End of the buffer

  @note
  The buffer is assumed to contain one of the names specified in the TYPELIB,
  followed by comma, '=', or end of the buffer.

  @retval
    0   No matching name
  @retval
    >0  Offset+1 in typelib for matched name
*/

static uint parse_name(const TYPELIB *lib, const char **strpos,
                       const char *end) {
  const char *pos = *strpos;
  uint find = find_type(pos, lib, FIND_TYPE_COMMA_TERM);
  for (; pos != end && *pos != '=' && *pos != ','; pos++)
    ;
  *strpos = pos;
  return find;
}

/**
  Parse and apply a set of flag assingments

  @param lib               Flag names
  @param default_name      Number of "default" in the typelib
  @param cur_set           Current set of flags (start from this state)
  @param default_set       Default set of flags (use this for assign-default
                           keyword and flag=default assignments)
  @param str               String to be parsed
  @param length            Length of the string
  @param err_pos      OUT  If error, set to point to start of wrong set string
                           NULL on success
  @param err_len      OUT  If error, set to the length of wrong set string
  @param dedup             Should duplicate entries be considered as valid and
                           deduplicated? Default is false.

  @details
  Parse a set of flag assignments, that is, parse a string in form:

    param_name1=value1,param_name2=value2,...

  where the names are specified in the TYPELIB, and each value can be
  either 'on','off', or 'default'. Setting the same name twice is not
  allowed (unless dedup argument is set to true, see note below).

  Besides param=val assignments, we support the "default" keyword (keyword
  default_name in the typelib). It can be used one time, if specified it
  causes us to build the new set over the default_set rather than cur_set
  value.

  @note
  it's not charset aware

  if the argument 'dedup' is set to true then duplicate entries can be specified
  for the same parameter. The duplicate entries will be de-duplicated and the
  latest entry will be given the precedence.

  @retval
    Parsed set value if (*errpos == NULL), otherwise undefined
*/

uint64_t find_set_from_flags(const TYPELIB *lib, size_t default_name,
                             uint64_t cur_set, uint64_t default_set,
                             const char *str, uint length, const char **err_pos,
                             uint *err_len, bool dedup) {
  const char *end = str + length;
  uint64_t flags_to_set = 0, flags_to_clear = 0, res;
  bool set_defaults = false;

  *err_pos = nullptr; /* No error yet */
  if (str != end) {
    const char *start = str;
    for (;;) {
      const char *pos = start;
      uint flag_no, value;

      if (!(flag_no = parse_name(lib, &pos, end))) goto err;

      if (flag_no == default_name) {
        /* Using 'default' twice isn't allowed. */
        if (set_defaults) goto err;
        set_defaults = true;
      } else {
        uint64_t bit = (1ULL << (flag_no - 1));

        /* if duplicate entries need to be allowed then clear the values
         * set for the previous entries before setting the latest entry. This
         * will lead to the latest entry taking precedence over the previous
         * entries.
         */
        if (dedup) {
          if (flags_to_set & bit) flags_to_set &= ~bit;
          if (flags_to_clear & bit) flags_to_clear &= ~bit;
        }

        /* parse the '=on|off|default' */
        if ((flags_to_clear | flags_to_set) & bit || pos >= end ||
            *pos++ != '=' ||
            !(value = parse_name(&on_off_default_typelib, &pos, end)))
          goto err;

        if (value == 1) /* this is '=off' */
          flags_to_clear |= bit;
        else if (value == 2) /* this is '=on' */
          flags_to_set |= bit;
        else /* this is '=default'  */
        {
          if (default_set & bit)
            flags_to_set |= bit;
          else
            flags_to_clear |= bit;
        }
      }
      if (pos >= end) break;

      if (*pos++ != ',') goto err;

      start = pos;
      continue;
    err:
      *err_pos = start;
      *err_len = (uint)(end - start);
      break;
    }
  }
  res = set_defaults ? default_set : cur_set;
  res |= flags_to_set;
  res &= ~flags_to_clear;
  return res;
}
