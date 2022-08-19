/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_aes_impl.h
*/

/** Maximum supported key length */
#define MAX_AES_KEY_LENGTH 256

/* TODO: remove in a future version */
/* Guard against using an old export control restriction #define */
#ifdef AES_USE_KEY_BITS
#error AES_USE_KEY_BITS not supported
#endif

extern uint *my_aes_opmode_key_sizes;

void my_aes_create_key(const unsigned char *key, uint key_length, uint8 *rkey,
                       enum my_aes_opmode opmode);
