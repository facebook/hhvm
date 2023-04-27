/*
   Copyright (c) 2015, 2019, Oracle and/or its affiliates. All Rights Reserved.
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

#ifndef XA_AUX_H
#define XA_AUX_H
#include "m_string.h"  // _dig_vec_lower

/**
  Function serializes XID which is characterized by by four last arguments
  of the function.
  Serialized XID is presented in valid hex format and is returned to
  the caller in a buffer pointed by the first argument.
  The buffer size provived by the caller must be not less than
  8 + 2 * XIDDATASIZE +  4 * sizeof(XID::formatID) + 1, see
  XID::serialize_xid() that is a caller and plugin.h for XID declaration.

  @param buf  pointer to a buffer allocated for storing serialized data
  @param fmt  formatID value
  @param gln  gtrid_length value
  @param bln  bqual_length value
  @param dat  data value

  @return  the value of the buffer pointer
*/

inline char *serialize_xid(char *buf, long fmt, long gln, long bln,
                           const char *dat) {
  int i;
  char *c = buf;
  /*
    Build a string like following pattern:
      X'hex11hex12...hex1m',X'hex21hex22...hex2n',11
    and store it into buf.
    Here hex1i and hex2k are hexadecimals representing XID's internal
    raw bytes (1 <= i <= m, 1 <= k <= n), and `m' and `n' even numbers
    half of which corresponding to the lengths of XID's components.
  */
  *c++ = 'X';
  *c++ = '\'';
  for (i = 0; i < gln; i++) {
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) >> 4];
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) & 0x0f];
  }
  *c++ = '\'';

  *c++ = ',';
  *c++ = 'X';
  *c++ = '\'';
  for (; i < gln + bln; i++) {
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) >> 4];
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) & 0x0f];
  }
  *c++ = '\'';
  sprintf(c, ",%lu", fmt);

  return buf;
}

#endif /* XA_AUX_H */
