/* Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.

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

/*  File   : strmake.c
    Author : Michael Widenius
    Updated: 20 Jul 1984
    Defines: strmake()

    strmake(dst,src,length) moves length characters, or until end, of src to
    dst and appends a closing NUL to dst.
    Note that if strlen(src) >= length then dst[length] will be set to \0
    strmake() returns pointer to closing null
*/

#include <stddef.h>
#include "m_string.h"  // IWYU pragma: keep

char *strmake(char *dst, const char *src, size_t length) {
#ifdef EXTRA_DEBUG
  /*
    'length' is the maximum length of the string; the buffer needs
    to be one character larger to accomodate the terminating '\0'.
    This is easy to get wrong, so we make sure we write to the
    entire length of the buffer to identify incorrect buffer-sizes.
    We only initialise the "unused" part of the buffer here, a) for
    efficiency, and b) because dst==src is allowed, so initialising
    the entire buffer would overwrite the source-string. Also, we
    write a character rather than '\0' as this makes spotting these
    problems in the results easier.
  */
  uint n = 0;
  while (n < length && src[n++])
    ;
  memset(dst + n, (int)'Z', length - n + 1);
#endif

  while (length--)
    if (!(*dst++ = *src++)) return dst - 1;
  *dst = 0;
  return dst;
}
