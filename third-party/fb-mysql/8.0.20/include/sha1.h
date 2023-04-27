#ifndef SHA1_INCLUDED
#define SHA1_INCLUDED

/* Copyright (c) 2002, 2017, Oracle and/or its affiliates. All rights reserved.

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
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/**
  @file include/sha1.h
*/

#include <stddef.h>

#include "my_compiler.h"
#include "my_config.h"
#include "my_inttypes.h"
#include "my_macros.h"

#define SHA1_HASH_SIZE 20 /* Hash size in bytes */

void compute_sha1_hash(uint8 *digest, const char *buf, size_t len)
#if defined(HAVE_VISIBILITY_HIDDEN)
    MY_ATTRIBUTE((visibility("hidden")))
#endif
        ;
void compute_sha1_hash_multi(uint8 *digest, const char *buf1, int len1,
                             const char *buf2, int len2)
#if defined(HAVE_VISIBILITY_HIDDEN)
    MY_ATTRIBUTE((visibility("hidden")))
#endif
        ;

#endif /* SHA1_INCLUDED */
