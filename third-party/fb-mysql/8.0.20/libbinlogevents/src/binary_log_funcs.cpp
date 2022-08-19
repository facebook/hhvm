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

#include "libbinlogevents/export/binary_log_funcs.h"

#include "byteorder.h"
#include "field_types.h"  // enum_field_types
#include "my_time.h"      // DATETIME_MAX_DECIMALS
#include "wrapper_functions.h"

#include <stdint.h>
#include <string.h>
#include <climits>
#include <cstring>

/**
   Max value for an unsigned integer of 'bits' bits.

   The somewhat contorted expression is to avoid overflow.
 */
static unsigned int uint_max(int bits) {
  BAPI_ASSERT(static_cast<unsigned int>(bits) <=
              sizeof(unsigned int) * CHAR_BIT);
  return (((1U << (bits - 1)) - 1) << 1) | 1;
}

/**
  Calculate binary size of packed numeric time representation.

  @param   dec   Precision.
  The same formula is used to find the binary size of the packed numeric time
  in libbinlogevents/src/value.cpp calc_field_size().
  If any modification is done here the same needs to be done in the
  aforementioned method in libbinlogevents also.
*/
unsigned int my_time_binary_length(unsigned int dec) {
  BAPI_ASSERT(dec <= DATETIME_MAX_DECIMALS);
  return 3 + (dec + 1) / 2;
}

/**
  Calculate binary size of packed datetime representation.
  @param dec  Precision.

  The same formula is used to find the binary size of the packed numeric time
  in libbinlogevents/src/value.cpp calc_field_size().
  If any modification is done here the same needs to be done in the
  aforementioned method in libbinlogevents also.
*/
unsigned int my_datetime_binary_length(unsigned int dec) {
  BAPI_ASSERT(dec <= DATETIME_MAX_DECIMALS);
  return 5 + (dec + 1) / 2;
}

/**
  Calculate on-disk size of a timestamp value.

  @param  dec  Precision.

  The same formula is used to find the binary size of the packed numeric time
  in libbinlogevents/src/value.cpp calc_field_size().
  If any modification is done here the same needs to be done in the
  aforementioned method in libbinlogevents also.
*/
unsigned int my_timestamp_binary_length(unsigned int dec) {
  BAPI_ASSERT(dec <= DATETIME_MAX_DECIMALS);
  return 4 + (dec + 1) / 2;
}

/**
   Compute the maximum display length of a field.

   @param sql_type Type of the field
   @param metadata The metadata from the master for the field.
   @return Maximum length of the field in bytes.
 */
unsigned int max_display_length_for_field(enum_field_types sql_type,
                                          unsigned int metadata) {
  BAPI_ASSERT(metadata >> 16 == 0);

  switch (sql_type) {
    case MYSQL_TYPE_NEWDECIMAL:
      return metadata >> 8;

    case MYSQL_TYPE_FLOAT:
      return 12;

    case MYSQL_TYPE_DOUBLE:
      return 22;

    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_ENUM:
      return metadata & 0x00ff;

    case MYSQL_TYPE_STRING: {
      unsigned char type = metadata >> 8;
      if (type == MYSQL_TYPE_SET || type == MYSQL_TYPE_ENUM)
        return metadata & 0xff;
      else
        return (((metadata >> 4) & 0x300) ^ 0x300) + (metadata & 0x00ff);
    }

    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_TINY:
      return 4;

    case MYSQL_TYPE_SHORT:
      return 6;

    case MYSQL_TYPE_INT24:
      return 9;

    case MYSQL_TYPE_LONG:
      return 11;

    case MYSQL_TYPE_LONGLONG:
      return 20;

    case MYSQL_TYPE_NULL:
      return 0;

    case MYSQL_TYPE_NEWDATE:
      return 3;

    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_TIME2:
      return 3;

    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_TIMESTAMP2:
      return 4;

    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_DATETIME2:
      return 8;

    case MYSQL_TYPE_BIT:
      /*
        Decode the size of the bit field from the master.
      */
      BAPI_ASSERT((metadata & 0xff) <= 7);
      return 8 * (metadata >> 8U) + (metadata & 0x00ff);

    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
      return metadata;

      /*
        The actual length for these types does not really matter since
        they are used to calc_pack_length, which ignores the given
        length for these types.

        Since we want this to be accurate for other uses, we return the
        maximum size in bytes of these BLOBs.
      */

    case MYSQL_TYPE_TINY_BLOB:
      return uint_max(1 * 8);

    case MYSQL_TYPE_MEDIUM_BLOB:
      return uint_max(3 * 8);

    case MYSQL_TYPE_BLOB:
      /*
        For the blob type, Field::real_type() lies and say that all
        blobs are of type MYSQL_TYPE_BLOB. In that case, we have to look
        at the length instead to decide what the max display size is.
       */
      return uint_max(metadata * 8);

    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_JSON:
      return uint_max(4 * 8);

    default:
      return UINT_MAX;
  }
}

int decimal_binary_size(int precision, int scale) {
  static const int dig2bytes[10] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 4};
  int intg = precision - scale, intg0 = intg / 9, frac0 = scale / 9,
      intg0x = intg - intg0 * 9, frac0x = scale - frac0 * 9;

  BAPI_ASSERT(scale >= 0 && precision > 0 && scale <= precision);
  return intg0 * sizeof(uint32_t) + dig2bytes[intg0x] +
         frac0 * sizeof(uint32_t) + dig2bytes[frac0x];
}

uint32_t calc_field_size(unsigned char col, const unsigned char *master_data,
                         unsigned int metadata) {
  uint32_t length = 0;

  switch ((col)) {
    case MYSQL_TYPE_NEWDECIMAL:
      length = decimal_binary_size(metadata >> 8, metadata & 0xff);
      break;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      length = metadata;
      break;
    /*
      The cases for SET and ENUM are include for completeness, however
      both are mapped to type MYSQL_TYPE_STRING and their real types
      are encoded in the field metadata.
    */
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_STRING: {
      unsigned char type = metadata >> 8U;
      if ((type == MYSQL_TYPE_SET) || (type == MYSQL_TYPE_ENUM))
        length = metadata & 0x00ff;
      else {
        /*
          We are reading the actual size from the master_data record
          because this field has the actual lengh stored in the first
          one or two bytes.
        */
        length = max_display_length_for_field(MYSQL_TYPE_STRING, metadata) > 255
                     ? 2
                     : 1;

        if (length == 1)
          length += *master_data;
        else {
          uint32_t temp = 0;
          memcpy(&temp, master_data, 2);
          length = length + le32toh(temp);
        }
      }
      break;
    }
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_TINY:
      length = 1;
      break;
    case MYSQL_TYPE_SHORT:
      length = 2;
      break;
    case MYSQL_TYPE_INT24:
      length = 3;
      break;
    case MYSQL_TYPE_LONG:
      length = 4;
      break;
    case MYSQL_TYPE_LONGLONG:
      length = 8;
      break;
    case MYSQL_TYPE_NULL:
      length = 0;
      break;
    case MYSQL_TYPE_NEWDATE:
      length = 3;
      break;
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
      length = 3;
      break;
    case MYSQL_TYPE_TIME2:
      /*
        The original methods in the server to calculate the binary size of the
        packed numeric time representation is defined in my_time.c, the
        signature being  unsigned int my_time_binary_length(uint)

        The length below needs to be updated if the above method is updated in
        the server
      */
      length = my_time_binary_length(metadata);
      break;
    case MYSQL_TYPE_TIMESTAMP:
      length = 4;
      break;
    case MYSQL_TYPE_TIMESTAMP2:
      /*
        The original methods in the server to calculate the binary size of the
        packed numeric time representation is defined in time.c, the signature
        being  unsigned int my_timestamp_binary_length(uint)

        The length below needs to be updated if the above method is updated in
        the server
      */
      length = my_timestamp_binary_length(metadata);
      break;
    case MYSQL_TYPE_DATETIME:
      length = 8;
      break;
    case MYSQL_TYPE_DATETIME2:
      /*
        The original methods in the server to calculate the binary size of the
        packed numeric time representation is defined in time.c, the signature
        being  unsigned int my_datetime_binary_length(uint)

        The length below needs to be updated if the above method is updated in
        the server
      */
      length = my_datetime_binary_length(metadata);
      break;
    case MYSQL_TYPE_BIT: {
      /*
        Decode the size of the bit field from the master.
          from_len is the length in bytes from the master
          from_bit_len is the number of extra bits stored in the master record
        If from_bit_len is not 0, add 1 to the length to account for accurate
        number of bytes needed.
      */
      unsigned int from_len = (metadata >> 8U) & 0x00ff;
      unsigned int from_bit_len = metadata & 0x00ff;
      BAPI_ASSERT(from_bit_len <= 7);
      length = from_len + ((from_bit_len > 0) ? 1 : 0);
      break;
    }
    case MYSQL_TYPE_VARCHAR: {
      length = metadata > 255 ? 2 : 1;
      if (length == 1)
        length += (uint32_t)*master_data;
      else {
        uint32_t temp = 0;
        memcpy(&temp, master_data, 2);
        length = length + le32toh(temp);
      }
      break;
    }
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_JSON: {
      /*
        Compute the length of the data. We cannot use get_length() here
        since it is dependent on the specific table (and also checks the
        packlength using the internal 'table' pointer) and replication
        is using a fixed format for storing data in the binlog.
      */
      switch (metadata) {
        case 1:
          length = *master_data;
          break;
        case 2:
          memcpy(&length, master_data, 2);
          length = le32toh(length);
          break;
        case 3:
          memcpy(&length, master_data, 3);
          length = le32toh(length);
          break;
        case 4:
          memcpy(&length, master_data, 4);
          length = le32toh(length);
          break;
        default:
          BAPI_ASSERT(0);  // Should not come here
          break;
      }

      length += metadata;
      break;
    }
    default:
      length = UINT_MAX;
  }
  return length;
}
