/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <sys/types.h>

#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "mysql.h"
#include "mysql_com.h"

/* Get the length of next field. Change parameter to point at fieldstart */
ulong STDCALL net_field_length(uchar **packet) {
  const uchar *pos = *packet;
  if (*pos < 251) {
    (*packet)++;
    return (ulong)*pos;
  }
  if (*pos == 251) {
    (*packet)++;
    return NULL_LENGTH;
  }
  if (*pos == 252) {
    (*packet) += 3;
    return (ulong)uint2korr(pos + 1);
  }
  if (*pos == 253) {
    (*packet) += 4;
    return (ulong)uint3korr(pos + 1);
  }
  (*packet) += 9; /* Must be 254 when here */
  return (ulong)uint4korr(pos + 1);
}

/* The same as above but with max length check */
ulong STDCALL net_field_length_checked(uchar **packet, ulong max_length) {
  ulong len;
  uchar *pos = (uchar *)*packet;

  if (*pos < 251) {
    (*packet)++;
    len = (ulong)*pos;
    return (len > max_length) ? max_length : len;
  }
  if (*pos == 251) {
    (*packet)++;
    return NULL_LENGTH;
  }
  if (*pos == 252) {
    (*packet) += 3;
    len = (ulong)uint2korr(pos + 1);
    return (len > max_length) ? max_length : len;
  }
  if (*pos == 253) {
    (*packet) += 4;
    len = (ulong)uint3korr(pos + 1);
    return (len > max_length) ? max_length : len;
  }
  (*packet) += 9; /* Must be 254 when here */
  len = (ulong)uint4korr(pos + 1);
  return (len > max_length) ? max_length : len;
}

/* The same as above but returns longlong */
uint64_t net_field_length_ll(uchar **packet) {
  const uchar *pos = *packet;
  if (*pos < 251) {
    (*packet)++;
    return (uint64_t)*pos;
  }
  if (*pos == 251) {
    (*packet)++;
    return (uint64_t)NULL_LENGTH;
  }
  if (*pos == 252) {
    (*packet) += 3;
    return (uint64_t)uint2korr(pos + 1);
  }
  if (*pos == 253) {
    (*packet) += 4;
    return (uint64_t)uint3korr(pos + 1);
  }
  (*packet) += 9; /* Must be 254 when here */
  return (uint64_t)uint8korr(pos + 1);
}

/*
  Store an integer with simple packing into a output package

  SYNOPSIS
    net_store_length()
    pkg			Store the packed integer here
    length		integers to store

  NOTES
    This is mostly used to store lengths of strings.
    We have to cast the result of the LL because of a bug in Forte CC
    compiler.

  RETURN
   Position in 'pkg' after the packed length
*/

uchar *net_store_length(uchar *packet, ulonglong length) {
  if (length < (ulonglong)251LL) {
    *packet = (uchar)length;
    return packet + 1;
  }
  /* 251 is reserved for NULL */
  if (length < (ulonglong)65536LL) {
    *packet++ = 252;
    int2store(packet, (uint)length);
    return packet + 2;
  }
  if (length < (ulonglong)16777216LL) {
    *packet++ = 253;
    int3store(packet, (ulong)length);
    return packet + 3;
  }
  *packet++ = 254;
  int8store(packet, length);
  return packet + 8;
}

/**
  The length of space required to store the resulting length-encoded integer
  for the given number. This function can be used at places where one needs to
  dynamically allocate the buffer for a given number to be stored as length-
  encoded integer.

  @param [in] num   the input number

  @return length of buffer needed to store this number [1, 3, 4, 9].
*/

uint net_length_size(ulonglong num) {
  if (num < 251ULL) return 1;
  if (num < 65536LL) return 3;
  if (num < 16777216ULL) return 4;
  return 9;
}

/**
  length of buffer required to represent a length-encoded string
  give the length part of length encoded string. This function can
  be used before calling net_field_lenth/net_field_length_ll.

  @param [in] pos  Length information of length-encoded string

  @return length of buffer needed to store this number [1, 3, 4, 9].
*/

uint net_field_length_size(const uchar *pos) {
  if (*pos <= 251) return 1;
  if (*pos == 252) return 3;
  if (*pos == 253) return 4;
  return 9;
}
