/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

/*
 * Functions exported from this package.
 */

#ifndef BINARY_LOG_FUNCS_INCLUDED
#define BINARY_LOG_FUNCS_INCLUDED

// We use cstdint if this is 2011 standard (or later)
#if __cplusplus > 201100L
#include <cstdint>
enum enum_field_types : int;
#else
#include <stdint.h>
#include "field_types.h"  // enum_field_types
#endif

#ifdef __cplusplus
extern "C" {
#endif

unsigned int my_time_binary_length(unsigned int dec);
unsigned int my_datetime_binary_length(unsigned int dec);
unsigned int my_timestamp_binary_length(unsigned int dec);

/**
  This helper function calculates the size in bytes of a particular field in a
  row type event as defined by the field_ptr and metadata_ptr arguments.

  @param col Field type code
  @param master_data The field data
  @param metadata The field metadata

  @note We need the actual field data because the string field size is not
  part of the meta data. :(

  @return The size in bytes of a particular field
*/
uint32_t calc_field_size(unsigned char col, const unsigned char *master_data,
                         unsigned int metadata);

/**
   Compute the maximum display length of a field.

   @param sql_type Type of the field
   @param metadata The metadata from the master for the field.
   @return Maximum length of the field in bytes.
 */
unsigned int max_display_length_for_field(enum_field_types sql_type,
                                          unsigned int metadata);

/**
   Returns the size of array to hold a binary representation of a decimal
   @param  precision  number of significant digits in a particular radix R
                      where R is either 2 or 10.
   @param  scale      to what position to round.
   @return  size in bytes
*/
int decimal_binary_size(int precision, int scale);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif /* BINARY_LOG_FUNCS_INCLUDED */
