/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SDI_UTILS_INCLUDED
#define SDI_UTILS_INCLUDED

#include <stddef.h>

#include "my_inttypes.h"

/**
  This function retrieves serialized meta data from the data dictionary
  for the given schema and table. The serialized meta data may be used
  for various purposes, such as writing it to a file, shipping it to
  other processes, etc.

  @param[in]  schema_name      Name of the schema
  @param[in]  table_name       Name of the table
  @param[out] meta_data        The serialized meta data of the table
  @param[out] meta_data_length The size of the meta data blob

  @retval false Success
  @retval true  Error, e.g. the table was not found in the data dictionary.
                In this case, the values of 'meta_data_length' and
                'meta_data' are not defined.
*/

bool create_serialized_meta_data(const char *schema_name,
                                 const char *table_name, uchar **meta_data,
                                 size_t *meta_data_length);

/**
  This function takes the submitted serialized meta data, de-serializes
  and analyzes it, and merges it into the data dictionary.

  @param[in] meta_data        The serialized meta data
  @param[in] meta_data_length The size of the meta data blob
  @param[in] readonly         Disallow conflict resolution affecting the
                              imported meta data

  @retval false Success
  @retval true  Error, e.g. a conflict was detected while 'readonly' is
                set to 'true'.
*/

bool import_serialized_meta_data(const uchar *meta_data,
                                 size_t meta_data_length, bool readonly);

/**
  This function takes the two submitted serialized meta data blobs,
  and compares them.

  @param[in] a_meta_data        First serialized meta data blob
  @param[in] a_meta_data_length Size of the first meta data blob
  @param[in] b_meta_data        Second serialized meta data blob
  @param[in] b_meta_data_length Size of the second meta data blob

  @retval false The two blobs are identical
  @retval true  The two blobs are different
*/

bool different_serialized_meta_data(const uchar *a_meta_data,
                                    size_t a_meta_data_length,
                                    const uchar *b_meta_data,
                                    size_t b_meta_data_length);

#endif /* SDI_UTILS_INCLUDED */
