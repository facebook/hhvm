/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_STRING_SERVICE_H
#define MYSQL_STRING_SERVICE_H

#include <mysql/components/service.h>

#include "my_inttypes.h"

/**
  The string functions as a service to the mysql_server component.
  So, that by default this service is available to all the components
  register to the server.
*/

DEFINE_SERVICE_HANDLE(my_h_string);
DEFINE_SERVICE_HANDLE(my_h_string_iterator);

/**
  Service for String create and destroy.
*/
BEGIN_SERVICE_DEFINITION(mysql_string_factory)
/**
  Creates a new instance of string object

  @param out_string holds pointer to newly created string object.
  @return Status of performed operation
  @retval false success @retval true failure
*/
DECLARE_BOOL_METHOD(create, (my_h_string * out_string));

/**
  Destroys specified string object and data contained by it.

  @param string String object handle to release.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_METHOD(void, destroy, (my_h_string string));
END_SERVICE_DEFINITION(mysql_string_factory)

/**
  Service for String case conversions, to lower case and to upper case.
*/
BEGIN_SERVICE_DEFINITION(mysql_string_case)
/**
  Convert a String pointed by handle to lower case. Conversion depends on the
  client character set info

  @param out_string Holds the converted lower case string object.
  @param in_string Pointer to string object to be converted.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(tolower, (my_h_string * out_string, my_h_string in_string));

/**
  Convert a String pointed by handle to upper case. Conversion depends on the
  client character set info

  @param out_string Holds the converted upper case string object.
  @param in_string Pointer to string object to be converted.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(toupper, (my_h_string * out_string, my_h_string in_string));
END_SERVICE_DEFINITION(mysql_string_case)

/**
  Service for conversions, string to buffer and buffer to string.
*/
BEGIN_SERVICE_DEFINITION(mysql_string_converter)
/**
  alocates a string object and converts the character buffer to string
  of specified charset_name.
  please call destroy() api to free the allocated string after this api.

  @param [out] out_string Pointer to string object handle to set new string
    to.
  @param in_buffer Pointer to the buffer with data to be interpreted as
    string.
  @param length Length of the buffer to copy into string, in bytes, not in
    character count.
  @param charset_name charset that is used for convertion.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(convert_from_buffer,
                    (my_h_string * out_string, const char *in_buffer,
                     uint64 length, const char *charset_name));

/**
  converts the mysql_string to the character set specified by
  charset_name parameter.

  @param in_string Pointer to string object handle to set new string
    to.
  @param [out] out_buffer Pointer to the buffer with data to be interpreted
    as characters.
  @param length Length of the buffer to hold out put in characters.
  @param charset_name Handle to charset that is used for convertion.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(convert_to_buffer,
                    (my_h_string in_string, char *out_buffer, uint64 length,
                     const char *charset_name));
END_SERVICE_DEFINITION(mysql_string_converter)

/**
  Service to get a character in String and number of characters in string
*/
BEGIN_SERVICE_DEFINITION(mysql_string_character_access)
/**
  Gets client character code of character on specified index position in
  string to a specified buffer.

  @param string String object handle to get character from.
  @param index Index, position of character to query.
  @param [out] out_char Pointer to long value to store character to.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(get_char,
                    (my_h_string string, uint index, ulong *out_char));

/**
  Gets length of specified string expressed as number of characters.

  @param string String object handle to get length of.
  @param [out] out_length Pointer to 64bit value to store length of string to.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(get_char_length, (my_h_string string, uint *out_length));
END_SERVICE_DEFINITION(mysql_string_character_access)

/**
  Service to get a byte in String and number of bytes in string
*/
BEGIN_SERVICE_DEFINITION(mysql_string_byte_access)
/**
  Gets byte code of string at specified index position to a
  specified 32-bit buffer.

  @param string String object handle to get character from.
  @param index Index, position of character to query.
  @param [out] out_char Pointer to 32bit value to store byte to.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(get_byte, (my_h_string string, uint index, uint *out_char));

/**
  Gets length of specified string expressed as number of bytes.

  @param string String object handle to get length of.
  @param [out] out_length Pointer to 32bit value to store length of string to.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(get_byte_length, (my_h_string string, uint *out_length));
END_SERVICE_DEFINITION(mysql_string_byte_access)

/**
  Service for listing Strings by iterator.
*/
BEGIN_SERVICE_DEFINITION(mysql_string_iterator)
/**
  Creates an iterator for a specified string to allow iteration through all
    characters in the string.

  @param string String object handle to get iterator to.
  @param [out] out_iterator Pointer to string iterator handle to store result
    object to.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(iterator_create,
                    (my_h_string string, my_h_string_iterator *out_iterator));

/**
  Retrieves character code at current iterator position and advances the
    iterator.

  @param iter String iterator object handle to advance.
  @param [out] out_char Pointer to 64bit value to store character to. May be
    NULL to omit retrieval of character and just advance the iterator.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(iterator_get_next,
                    (my_h_string_iterator iter, int *out_char));

/**
  Releases the string iterator object specified.

  @param iter String iterator object handle te release.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_METHOD(void, iterator_destroy, (my_h_string_iterator iter));
END_SERVICE_DEFINITION(mysql_string_iterator)

/**
  Service for String c_type.
*/
BEGIN_SERVICE_DEFINITION(mysql_string_ctype)
/**
  Checks if character on current position the iterator points to is an upper
    case.

  @param iter String iterator object handle to advance.
  @param [out] out Pointer to bool value to store if character is an upper
    case.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(is_upper, (my_h_string_iterator iter, bool *out));

/**
  Checks if character on current position the iterator points to is a lower
    case.

  @param iter String iterator object handle to advance.
  @param [out] out Pointer to bool value to store if character is a lower
    case.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(is_lower, (my_h_string_iterator iter, bool *out));

/**
  Checks if character on current position the iterator points to is a digit.

  @param iter String iterator object handle to advance.
  @param [out] out Pointer to bool value to store if character is a digit.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(is_digit, (my_h_string_iterator iter, bool *out));
END_SERVICE_DEFINITION(mysql_string_ctype)
#endif /* MYSQL_STRING_SERVICE_H */
