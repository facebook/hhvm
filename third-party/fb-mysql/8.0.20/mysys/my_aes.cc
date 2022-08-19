/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_aes.cc
*/

#include <sys/types.h>

#include "m_string.h"
#include "my_aes.h"
#include "my_inttypes.h"
#include "mysys/my_aes_impl.h"

/**
  Transforms an arbitrary long key into a fixed length AES key

  AES keys are of fixed length. This routine takes an arbitrary long key
  iterates over it in AES key length increment and XORs the bytes with the
  AES key buffer being prepared.
  The bytes from the last incomplete iteration are XORed to the start
  of the key until their depletion.
  Needed since crypto function routines expect a fixed length key.

  @param [in] key               Key to use for real key creation
  @param [in] key_length        Length of the key
  @param [out] rkey             Real key (used by OpenSSL)
  @param [out] opmode           encryption mode
*/

void my_aes_create_key(const unsigned char *key, uint key_length, uint8 *rkey,
                       enum my_aes_opmode opmode) {
  const uint key_size = my_aes_opmode_key_sizes[opmode] / 8;
  uint8 *rkey_end;                         /* Real key boundary */
  uint8 *ptr;                              /* Start of the real key*/
  const uint8 *sptr;                       /* Start of the working key */
  const uint8 *key_end = key + key_length; /* Working key boundary*/

  rkey_end = rkey + key_size;

  memset(rkey, 0, key_size); /* Set initial key  */

  for (ptr = rkey, sptr = key; sptr < key_end; ptr++, sptr++) {
    if (ptr == rkey_end) /*  Just loop over tmp_key until we used all key */
      ptr = rkey;
    *ptr ^= *sptr;
  }
}
