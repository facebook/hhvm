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

#include <string.h>
#include <sys/types.h>

#include <mysql/components/minimal_chassis.h>
#include "m_ctype.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/components/service_implementation.h"
#include "mysql/components/services/mysql_string.h"
#include "mysql/psi/psi_memory.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_string_service_imp.h"
#include "sql_string.h"

PSI_memory_key key_memory_string_service_iterator;

/**
  The string functions as a service to the mysql_server component.
  So, that by default this service is available to all the components
  register to the server.
*/

struct my_h_string_imp {};

struct my_h_string_iterator_imp {};

/**
  Creates a new instance of string object

  @param out_string holds pointer to newly created string object.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::create, (my_h_string * out_string)) {
  try {
    String *res = new String[1];
    *out_string = (my_h_string)res;
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Convert a String pointed by handle to lower case. Conversion depends on the
  client character set info

  @param out_string Holds the converted lower case string object.
  @param in_string Pointer to string object to be converted.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::tolower,
                   (my_h_string * out_string, my_h_string in_string)) {
  try {
    String *str = reinterpret_cast<String *>(in_string);
    if (str == nullptr) return true;
    String *res = reinterpret_cast<String *>(*out_string);
    const CHARSET_INFO *cs = str->charset();
    if (cs->casedn_multiply == 1) {
      res->copy(*str);
      my_casedn_str(cs, res->c_ptr_quick());
    } else {
      size_t len = str->length() * cs->casedn_multiply;
      res->set_charset(cs);
      res->alloc(len);
      len = cs->cset->casedn(cs, str->ptr(), str->length(), res->ptr(), len);
      res->length(len);
    }
    *out_string = (my_h_string)res;
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Convert a String pointed by handle to upper case. Conversion depends on the
  client character set info

  @param out_string Holds the converted lower case string object.
  @param in_string Pointer to string object to be converted.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::toupper,
                   (my_h_string * out_string, my_h_string in_string)) {
  try {
    String *str = reinterpret_cast<String *>(in_string);
    if (str == nullptr) return true;
    String *res = reinterpret_cast<String *>(*out_string);
    const CHARSET_INFO *cs = str->charset();
    if (cs->caseup_multiply == 1) {
      res->copy(*str);
      my_caseup_str(cs, res->c_ptr_quick());
    } else {
      size_t len = str->length() * cs->caseup_multiply;
      res->set_charset(cs);
      res->alloc(len);
      len = cs->cset->caseup(cs, str->ptr(), str->length(), res->ptr(), len);
      res->length(len);
    }
    *out_string = (my_h_string)res;
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Allocates a string object and converts the character buffer to string
  and just sets the specified charset_name in the string object. It does
  not performs the conversion of buffer into the specified character set.
  Caller must free the allocated string by calling destroy() api.

  @param [out] out_string Pointer to string object handle to set new string
    to.
  @param in_buffer Pointer to the buffer with data to be interpreted as
    string.
  @param length Length of the buffer to copy in bytes, not in character count.
  @param charset_name Handle to charset that is used for convertion.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::convert_from_buffer,
                   (my_h_string * out_string, const char *in_buffer,
                    uint64 length, const char *charset_name)) {
  try {
    if (in_buffer == nullptr || length == 0 || length > strlen(in_buffer))
      return true;

    String *res = new String[1];
    CHARSET_INFO *cs =
        get_charset_by_csname(charset_name, MY_CS_PRIMARY, MYF(0));

    if (!cs || res->copy(in_buffer, length, cs)) return true;
    *out_string = (my_h_string)res;
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  converts the mysql_string to the character buffer specified by
  charset_name parameter.

  @param [out] out_buffer Pointer to char buffer used to hold the converted
    string.
  @param in_string pointer to string handle which will be converted to char
    data.
  @param length Length of the buffer to copy in bytes, not in character count.
  @param charset_name Handle to charset that is used for convertion.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::convert_to_buffer,
                   (my_h_string in_string, char *out_buffer, uint64 length,
                    const char *charset_name)) {
  try {
    String *str = reinterpret_cast<String *>(in_string);
    uint error;
    CHARSET_INFO *cs =
        get_charset_by_csname(charset_name, MY_CS_PRIMARY, MYF(0));
    if (str == nullptr || cs == nullptr || length == 0) return true;
    size_t len = my_convert(out_buffer, length - 1, cs, str->ptr(),
                            str->length(), str->charset(), &error);
    out_buffer[len] = '\0';

    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Destroys specified string object and data contained by it.

  @param string String object handle to release.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_METHOD(void, mysql_string_imp::destroy, (my_h_string string)) {
  try {
    String *str = reinterpret_cast<String *>(string);
    if (str == nullptr) return;
    str->mem_free();
    delete[] str;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
}

/**
  Gets character code of character on specified index position in
  string to a specified buffer.

  @param string String object handle to get character from.
  @param index Index, position of character to query.
  @param [out] out_char Pointer to long value to store character to.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::get_char,
                   (my_h_string string, uint index, ulong *out_char)) {
  try {
    String *str = reinterpret_cast<String *>(string);
    if (str == nullptr || index >= str->length()) return true;
    my_charset_conv_mb_wc mb_wc = (str->charset())->cset->mb_wc;
    int ret = str->charpos(index);
    if (ret < 0) return true;
    const char *ptr = (str->ptr() + ret);
    if ((*mb_wc)(str->charset(), out_char, pointer_cast<const uchar *>(ptr),
                 (const uchar *)(str->ptr() + str->length())) <= 0)
      return true;

    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Gets length of specified string expressed as number of characters.

  @param string String object handle to get length of.
  @param [out] out_length Pointer to 32bit value to store length of string to.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::get_char_length,
                   (my_h_string string, uint *out_length)) {
  try {
    String *str = reinterpret_cast<String *>(string);
    if (str == nullptr) return true;
    *out_length = str->numchars();
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Gets byte code of string on specified index position in
  string to a specified 32-bit buffer.

  @param string String object handle to get character from.
  @param index Index, position of character to query.
  @param [out] out_char Pointer to 32bit value to store byte to.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::get_byte,
                   (my_h_string string, uint index, uint *out_char)) {
  try {
    String *str = reinterpret_cast<String *>(string);
    if (str == nullptr || index >= str->length()) return true;

    const char *ptr = str->ptr();
    if (ptr == nullptr) return true;
    *out_char = ptr[index];
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Gets length of specified string expressed as number of bytes.

  @param string String object handle to get length of.
  @param [out] out_length Pointer to 32bit value to store length of string to.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::get_byte_length,
                   (my_h_string string, uint *out_length)) {
  try {
    String *str = reinterpret_cast<String *>(string);
    if (str == nullptr) return true;
    *out_length = str->length();
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

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
DEFINE_BOOL_METHOD(mysql_string_imp::iterator_create,
                   (my_h_string string, my_h_string_iterator *out_iterator)) {
  try {
    String *str = reinterpret_cast<String *>(string);
    if (str == nullptr) return true;
    st_string_iterator *iterator = (st_string_iterator *)my_malloc(
        key_memory_string_service_iterator, sizeof(st_string_iterator), MYF(0));
    iterator->iterator_str = str;
    iterator->iterator_ptr = str->ptr();
    iterator->ctype = 0;
    *out_iterator = (my_h_string_iterator)iterator;

    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Retrieves character code at current iterator position and advances the
    iterator.

  @param iter String iterator object handle to advance.
  @param [out] out_char Pointer to 32bit value to store character to. May be
    NULL to omit retrieval of character and just advance the iterator.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::iterator_get_next,
                   (my_h_string_iterator iter, int *out_char)) {
  try {
    int char_len, tmp_len;
    st_string_iterator *iterator = (st_string_iterator *)iter;
    if (iterator == nullptr) return true;
    const String *str = iterator->iterator_str;
    const CHARSET_INFO *cs = str->charset();
    const char *end = str->ptr() + str->length();
    *out_char = 0;
    if (iterator->iterator_ptr >= end) return true;
    char_len = (cs->cset->ctype(
        cs, out_char, pointer_cast<const uchar *>(iterator->iterator_ptr),
        pointer_cast<const uchar *>(end)));
    iterator->ctype = *out_char;
    tmp_len = (char_len > 0 ? char_len : (char_len < 0 ? -char_len : 1));
    if (iterator->iterator_ptr + tmp_len > end)
      return true;
    else
      iterator->iterator_ptr += tmp_len;
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Releases the string iterator object specified.

  @param iter String iterator object handle te release.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_METHOD(void, mysql_string_imp::iterator_destroy,
              (my_h_string_iterator iter)) {
  try {
    if (iter == nullptr) return;
    my_free((st_string_iterator *)iter);
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
}

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
DEFINE_BOOL_METHOD(mysql_string_imp::is_upper,
                   (my_h_string_iterator iter, bool *out)) {
  try {
    st_string_iterator *iterator = (st_string_iterator *)iter;
    if (iterator == nullptr) return true;
    *out = (iterator->ctype & _MY_U);
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

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
DEFINE_BOOL_METHOD(mysql_string_imp::is_lower,
                   (my_h_string_iterator iter, bool *out)) {
  try {
    st_string_iterator *iterator = (st_string_iterator *)iter;
    if (iterator == nullptr) return true;
    *out = (iterator->ctype & _MY_L);
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Checks if character on current position the iterator points to is a digit.

  @param iter String iterator object handle to advance.
  @param [out] out Pointer to bool value to store if character is a digit.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_string_imp::is_digit,
                   (my_h_string_iterator iter, bool *out)) {
  try {
    st_string_iterator *iterator = (st_string_iterator *)iter;
    if (iterator == nullptr) return true;
    *out = (iterator->ctype & _MY_NMR);
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}
