/* Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_WRITE_SET_HANDLER_INCLUDED
#define RPL_WRITE_SET_HANDLER_INCLUDED

#include "my_inttypes.h"

extern const char *transaction_write_set_hashing_algorithms[];

class THD;
struct TABLE;

/**
  Function that returns the write set extraction algorithm name.

  @param[in] algorithm  The algorithm value

  @return the algorithm name
*/
const char *get_write_set_algorithm_string(unsigned int algorithm);

/**
  Function to add the hash of the PKE to the transaction context object.

  @param[in] table - TABLE object
  @param[in] thd - THD object pointing to current thread.
  @param[in] record - The record to process (record[0] or record[1]).
*/
void add_pke(TABLE *table, THD *thd, uchar *record);

#endif
