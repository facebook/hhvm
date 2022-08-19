#ifndef MY_MD5_SIZE_INCLUDED
#define MY_MD5_SIZE_INCLUDED
/* Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file include/my_md5_size.h

  This is not part of md5.h, so that it can be included using C linkage,
  unlike that file.
*/

/* Hash size in bytes */
#define MD5_HASH_SIZE 16
/* 16 bytes of binary = 32 printable characters */
#define MD5_HASH_TO_STRING_LENGTH 32

#endif  // MY_MD5_SIZE_INCLUDED
