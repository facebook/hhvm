/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/json_binary.h"

#include <string.h>
#include <algorithm>  // std::min
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "m_ctype.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysqld_error.h"
#ifdef MYSQL_SERVER
#include "sql/check_stack.h"
#endif
#include "sql/field.h"     // Field_json
#include "sql/json_dom.h"  // Json_dom
#include "sql/json_syntax_check.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/system_variables.h"
#include "sql/table.h"  // TABLE::add_binary_diff()
#include "sql_string.h"
#include "template_utils.h"  // down_cast

namespace {

constexpr char JSONB_TYPE_SMALL_OBJECT = 0x0;
constexpr char JSONB_TYPE_LARGE_OBJECT = 0x1;
constexpr char JSONB_TYPE_SMALL_ARRAY = 0x2;
constexpr char JSONB_TYPE_LARGE_ARRAY = 0x3;
constexpr char JSONB_TYPE_LITERAL = 0x4;
constexpr char JSONB_TYPE_INT16 = 0x5;
constexpr char JSONB_TYPE_UINT16 = 0x6;
constexpr char JSONB_TYPE_INT32 = 0x7;
constexpr char JSONB_TYPE_UINT32 = 0x8;
constexpr char JSONB_TYPE_INT64 = 0x9;
constexpr char JSONB_TYPE_UINT64 = 0xA;
constexpr char JSONB_TYPE_DOUBLE = 0xB;
constexpr char JSONB_TYPE_STRING = 0xC;
constexpr char JSONB_TYPE_OPAQUE = 0xF;

constexpr char JSONB_NULL_LITERAL = 0x0;
constexpr char JSONB_TRUE_LITERAL = 0x1;
constexpr char JSONB_FALSE_LITERAL = 0x2;

/*
  The size of offset or size fields in the small and the large storage
  format for JSON objects and JSON arrays.
*/
constexpr uint8 SMALL_OFFSET_SIZE = 2;
constexpr uint8 LARGE_OFFSET_SIZE = 4;

/*
  The size of key entries for objects when using the small storage
  format or the large storage format. In the small format it is 4
  bytes (2 bytes for key length and 2 bytes for key offset). In the
  large format it is 6 (2 bytes for length, 4 bytes for offset).
*/
constexpr uint8 KEY_ENTRY_SIZE_SMALL = 2 + SMALL_OFFSET_SIZE;
constexpr uint8 KEY_ENTRY_SIZE_LARGE = 2 + LARGE_OFFSET_SIZE;

/*
  The size of value entries for objects or arrays. When using the
  small storage format, the entry size is 3 (1 byte for type, 2 bytes
  for offset). When using the large storage format, it is 5 (1 byte
  for type, 4 bytes for offset).
*/
constexpr uint8 VALUE_ENTRY_SIZE_SMALL = 1 + SMALL_OFFSET_SIZE;
constexpr uint8 VALUE_ENTRY_SIZE_LARGE = 1 + LARGE_OFFSET_SIZE;

}  // namespace

namespace json_binary {

/// Status codes for JSON serialization.
enum enum_serialization_result {
  /**
    Success. The JSON value was successfully serialized.
  */
  OK,
  /**
    The JSON value was too big to be serialized. If this status code
    is returned, and the small storage format is in use, the caller
    should retry the serialization with the large storage format. If
    this status code is returned, and the large format is in use,
    my_error() will already have been called.
  */
  VALUE_TOO_BIG,
  /**
    Some other error occurred. my_error() will have been called with
    more specific information about the failure.
  */
  FAILURE
};

#ifdef MYSQL_SERVER
static enum_serialization_result serialize_json_value(
    const THD *thd, const Json_dom *dom, size_t type_pos, String *dest,
    size_t depth, bool small_parent);
static void write_offset_or_size(char *dest, size_t offset_or_size, bool large);
#endif  // ifdef MYSQL_SERVER
static uint8 offset_size(bool large);

#ifdef MYSQL_SERVER
bool serialize(const THD *thd, const Json_dom *dom, String *dest) {
  // Reset the destination buffer.
  dest->length(0);
  dest->set_charset(&my_charset_bin);

  // Reserve space (one byte) for the type identifier.
  if (dest->append('\0')) return true; /* purecov: inspected */
  return serialize_json_value(thd, dom, 0, dest, 0, false) != OK;
}

/**
  Reserve space for the given amount of extra bytes at the end of a
  String buffer. If the String needs to allocate more memory, it will
  grow by at least 50%, to avoid frequent reallocations.
*/
static bool reserve(String *buffer, size_t bytes_needed) {
  return buffer->reserve(bytes_needed, buffer->length() / 2);
}

/** Encode a 16-bit int at the end of the destination string. */
static bool append_int16(String *dest, int16 value) {
  if (reserve(dest, sizeof(value))) return true; /* purecov: inspected */
  int2store(dest->ptr() + dest->length(), value);
  dest->length(dest->length() + sizeof(value));
  return false;
}

/** Encode a 32-bit int at the end of the destination string. */
static bool append_int32(String *dest, int32 value) {
  if (reserve(dest, sizeof(value))) return true; /* purecov: inspected */
  int4store(dest->ptr() + dest->length(), value);
  dest->length(dest->length() + sizeof(value));
  return false;
}

/** Encode a 64-bit int at the end of the destination string. */
static bool append_int64(String *dest, int64 value) {
  if (reserve(dest, sizeof(value))) return true; /* purecov: inspected */
  int8store(dest->ptr() + dest->length(), value);
  dest->length(dest->length() + sizeof(value));
  return false;
}

/**
  Append an offset or a size to a String.

  @param dest  the destination String
  @param offset_or_size  the offset or size to append
  @param large  if true, use the large storage format (4 bytes);
                otherwise, use the small storage format (2 bytes)
  @return false if successfully appended, true otherwise
*/
static bool append_offset_or_size(String *dest, size_t offset_or_size,
                                  bool large) {
  if (large)
    return append_int32(dest, static_cast<int32>(offset_or_size));
  else
    return append_int16(dest, static_cast<int16>(offset_or_size));
}

/**
  Insert an offset or a size at the specified position in a String. It
  is assumed that the String has already allocated enough space to
  hold the value.

  @param dest  the destination String
  @param pos   the position in the String
  @param offset_or_size  the offset or size to append
  @param large  if true, use the large storage format (4 bytes);
                otherwise, use the small storage format (2 bytes)
*/
static void insert_offset_or_size(String *dest, size_t pos,
                                  size_t offset_or_size, bool large) {
  DBUG_ASSERT(pos + offset_size(large) <= dest->alloced_length());
  write_offset_or_size(dest->ptr() + pos, offset_or_size, large);
}

/**
  Write an offset or a size to a char array. The char array is assumed to be
  large enough to hold an offset or size value.

  @param dest            the array to write to
  @param offset_or_size  the offset or size to write
  @param large           if true, use the large storage format
*/
static void write_offset_or_size(char *dest, size_t offset_or_size,
                                 bool large) {
  if (large)
    int4store(dest, static_cast<uint32>(offset_or_size));
  else
    int2store(dest, static_cast<uint16>(offset_or_size));
}

/**
  Check if the size of a document exceeds the maximum JSON binary size
  (4 GB, aka UINT_MAX32). Raise an error if it is too big.

  @param size  the size of the document
  @return true if the document is too big, false otherwise
*/
static bool check_document_size(size_t size) {
  if (size > UINT_MAX32) {
    /* purecov: begin inspected */
    my_error(ER_JSON_VALUE_TOO_BIG, MYF(0));
    return true;
    /* purecov: end */
  }
  return false;
}

/**
  Append a length to a String. The number of bytes used to store the length
  uses a variable number of bytes depending on how large the length is. If the
  highest bit in a byte is 1, then the length is continued on the next byte.
  The least significant bits are stored in the first byte.

  @param  dest   the destination String
  @param  length the length to write
  @return false on success, true on error
*/
static bool append_variable_length(String *dest, size_t length) {
  do {
    // Filter out the seven least significant bits of length.
    uchar ch = (length & 0x7F);

    /*
      Right-shift length to drop the seven least significant bits. If there
      is more data in length, set the high bit of the byte we're writing
      to the String.
    */
    length >>= 7;
    if (length != 0) ch |= 0x80;

    if (dest->append(ch)) return true; /* purecov: inspected */
  } while (length != 0);

  if (check_document_size(dest->length() + length))
    return true; /* purecov: inspected */

  // Successfully appended the length.
  return false;
}
#endif  // ifdef MYSQL_SERVER

/**
  Read a variable length written by append_variable_length().

  @param[in] data  the buffer to read from
  @param[in] data_length  the maximum number of bytes to read from data
  @param[out] length  the length that was read
  @param[out] num  the number of bytes needed to represent the length
  @return  false on success, true if the variable length field is ill-formed
*/
static bool read_variable_length(const char *data, size_t data_length,
                                 uint32 *length, uint8 *num) {
  /*
    It takes five bytes to represent UINT_MAX32, which is the largest
    supported length, so don't look any further.
  */
  const size_t max_bytes = std::min(data_length, static_cast<size_t>(5));

  size_t len = 0;
  for (size_t i = 0; i < max_bytes; i++) {
    // Get the next 7 bits of the length.
    len |= (data[i] & 0x7f) << (7 * i);
    if ((data[i] & 0x80) == 0) {
      // The length shouldn't exceed 32 bits.
      if (len > UINT_MAX32) return true; /* purecov: inspected */

      // This was the last byte. Return successfully.
      *num = static_cast<uint8>(i + 1);
      *length = static_cast<uint32>(len);
      return false;
    }
  }

  // No more available bytes. Return true to signal error.
  return true; /* purecov: inspected */
}

/**
  Check if the specified offset or size is too big to store in the
  binary JSON format.

  If the small storage format is used, the caller is expected to retry
  serialization in the large storage format, so no error is generated
  if the offset or size is too big. If the large storage format is
  used, an error will be generated if the offset or size is too big.

  @param offset_or_size  the offset or size to check
  @param large    if true, we are using the large storage format
    for JSON arrays and objects, which allows offsets and sizes that
    fit in a uint32; otherwise, we are using the small storage format,
    which allow offsets and sizes that fit in a uint16.
  @return true if offset_or_size is too big for the format, false
    otherwise
*/
#ifdef MYSQL_SERVER
static bool is_too_big_for_json(size_t offset_or_size, bool large) {
  if (offset_or_size > UINT_MAX16) {
    if (!large) return true;
    return check_document_size(offset_or_size);
  }

  return false;
}

/**
  Append all the key entries of a JSON object to a destination string.
  The key entries are just a series of offset/length pairs that point
  to where the actual key names are stored.

  @param[in]  object  the JSON object
  @param[out] dest    the destination string
  @param[in]  offset  the offset of the first key
  @param[in]  large   if true, the large storage format will be used
  @return serialization status
*/
static enum_serialization_result append_key_entries(const Json_object *object,
                                                    String *dest, size_t offset,
                                                    bool large) {
#ifndef DBUG_OFF
  const std::string *prev_key = nullptr;
#endif

  /*
    Legacy json object is created by 5.6 json functions and can have
    duplicate keys. Do not serialize the documents created by legacy
    functions.
   */
  if (object->is_legacy_object()) {
    my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
    return FAILURE;
  }

  // Add the key entries.
  for (Json_object::const_iterator it = object->begin(); it != object->end();
       ++it) {
    const std::string *key = &it->first;
    size_t len = key->length();

#ifndef DBUG_OFF
    // Check that the DOM returns the keys in the correct order.
    if (prev_key) {
      DBUG_ASSERT(prev_key->length() <= len);
      if (len == prev_key->length())
        DBUG_ASSERT(memcmp(prev_key->data(), key->data(), len) < 0);
    }
    prev_key = key;
#endif

    // We only have two bytes for the key size. Check if the key is too big.
    if (len > UINT_MAX16) {
      my_error(ER_JSON_KEY_TOO_BIG, MYF(0));
      return FAILURE;
    }

    if (is_too_big_for_json(offset, large))
      return VALUE_TOO_BIG; /* purecov: inspected */

    if (append_offset_or_size(dest, offset, large) ||
        append_int16(dest, static_cast<int16>(len)))
      return FAILURE; /* purecov: inspected */
    offset += len;
  }

  return OK;
}
#endif  // ifdef MYSQL_SERVER

/**
  Will a value of the specified type be inlined?
  @param type  the type to check
  @param large true if the large storage format is used
  @return true if the value will be inlined
*/
static bool inlined_type(uint8 type, bool large) {
  switch (type) {
    case JSONB_TYPE_LITERAL:
    case JSONB_TYPE_INT16:
    case JSONB_TYPE_UINT16:
      return true;
    case JSONB_TYPE_INT32:
    case JSONB_TYPE_UINT32:
      return large;
    default:
      return false;
  }
}

/**
  Get the size of an offset value.
  @param large true if the large storage format is used
  @return the size of an offset
*/
static uint8 offset_size(bool large) {
  return large ? LARGE_OFFSET_SIZE : SMALL_OFFSET_SIZE;
}

/**
  Get the size of a key entry.
  @param large true if the large storage format is used
  @return the size of a key entry
*/
static uint8 key_entry_size(bool large) {
  return large ? KEY_ENTRY_SIZE_LARGE : KEY_ENTRY_SIZE_SMALL;
}

/**
  Get the size of a value entry.
  @param large true if the large storage format is used
  @return the size of a value entry
*/
static uint8 value_entry_size(bool large) {
  return large ? VALUE_ENTRY_SIZE_LARGE : VALUE_ENTRY_SIZE_SMALL;
}

/**
  Attempt to inline a value in its value entry at the beginning of an
  object or an array. This function assumes that the destination
  string has already allocated enough space to hold the inlined value.

  @param[in] value the JSON value
  @param[out] dest the destination string
  @param[in] pos   the offset where the value should be inlined
  @param[in] large true if the large storage format is used
  @return true if the value was inlined, false if it was not
*/
#ifdef MYSQL_SERVER
static bool attempt_inline_value(const Json_dom *value, String *dest,
                                 size_t pos, bool large) {
  int32 inlined_val;
  char inlined_type;
  switch (value->json_type()) {
    case enum_json_type::J_NULL:
      inlined_val = JSONB_NULL_LITERAL;
      inlined_type = JSONB_TYPE_LITERAL;
      break;
    case enum_json_type::J_BOOLEAN:
      inlined_val = down_cast<const Json_boolean *>(value)->value()
                        ? JSONB_TRUE_LITERAL
                        : JSONB_FALSE_LITERAL;
      inlined_type = JSONB_TYPE_LITERAL;
      break;
    case enum_json_type::J_INT: {
      const Json_int *i = down_cast<const Json_int *>(value);
      if (!i->is_16bit() && !(large && i->is_32bit()))
        return false;  // cannot inline this value
      inlined_val = static_cast<int32>(i->value());
      inlined_type = i->is_16bit() ? JSONB_TYPE_INT16 : JSONB_TYPE_INT32;
      break;
    }
    case enum_json_type::J_UINT: {
      const Json_uint *i = down_cast<const Json_uint *>(value);
      if (!i->is_16bit() && !(large && i->is_32bit()))
        return false;  // cannot inline this value
      inlined_val = static_cast<int32>(i->value());
      inlined_type = i->is_16bit() ? JSONB_TYPE_UINT16 : JSONB_TYPE_UINT32;
      break;
    }
    default:
      return false;  // cannot inline value of this type
  }

  (*dest)[pos] = inlined_type;
  insert_offset_or_size(dest, pos + 1, inlined_val, large);
  return true;
}

/**
  Serialize a JSON array at the end of the destination string.

  @param thd    THD handle
  @param array  the JSON array to serialize
  @param dest   the destination string
  @param large  if true, the large storage format will be used
  @param depth  the current nesting level
  @return serialization status
*/
static enum_serialization_result serialize_json_array(const THD *thd,
                                                      const Json_array *array,
                                                      String *dest, bool large,
                                                      size_t depth) {
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr))
    return FAILURE; /* purecov: inspected */

  const size_t start_pos = dest->length();
  const size_t size = array->size();

  if (check_json_depth(++depth)) {
    return FAILURE;
  }

  if (is_too_big_for_json(size, large)) return VALUE_TOO_BIG;

  // First write the number of elements in the array.
  if (append_offset_or_size(dest, size, large))
    return FAILURE; /* purecov: inspected */

  // Reserve space for the size of the array in bytes. To be filled in later.
  const size_t size_pos = dest->length();
  if (append_offset_or_size(dest, 0, large))
    return FAILURE; /* purecov: inspected */

  size_t entry_pos = dest->length();

  // Reserve space for the value entries at the beginning of the array.
  const auto entry_size = value_entry_size(large);
  if (dest->fill(dest->length() + size * entry_size, 0))
    return FAILURE; /* purecov: inspected */

  for (const auto &child : *array) {
    const Json_dom *elt = child.get();
    if (!attempt_inline_value(elt, dest, entry_pos, large)) {
      size_t offset = dest->length() - start_pos;
      if (is_too_big_for_json(offset, large)) return VALUE_TOO_BIG;
      insert_offset_or_size(dest, entry_pos + 1, offset, large);
      auto res = serialize_json_value(thd, elt, entry_pos, dest, depth, !large);
      if (res != OK) return res;
    }
    entry_pos += entry_size;
  }

  // Finally, write the size of the object in bytes.
  size_t bytes = dest->length() - start_pos;
  if (is_too_big_for_json(bytes, large))
    return VALUE_TOO_BIG; /* purecov: inspected */
  insert_offset_or_size(dest, size_pos, bytes, large);

  return OK;
}

/**
  Serialize a JSON object at the end of the destination string.

  @param thd    THD handle
  @param object the JSON object to serialize
  @param dest   the destination string
  @param large  if true, the large storage format will be used
  @param depth  the current nesting level
  @return serialization status
*/
static enum_serialization_result serialize_json_object(
    const THD *thd, const Json_object *object, String *dest, bool large,
    size_t depth) {
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr))
    return FAILURE; /* purecov: inspected */

  const size_t start_pos = dest->length();
  const size_t size = object->cardinality();

  if (check_json_depth(++depth)) {
    return FAILURE;
  }

  if (is_too_big_for_json(size, large))
    return VALUE_TOO_BIG; /* purecov: inspected */

  // First write the number of members in the object.
  if (append_offset_or_size(dest, size, large))
    return FAILURE; /* purecov: inspected */

  // Reserve space for the size of the object in bytes. To be filled in later.
  const size_t size_pos = dest->length();
  if (append_offset_or_size(dest, 0, large))
    return FAILURE; /* purecov: inspected */

  const auto key_entry_size = json_binary::key_entry_size(large);
  const auto value_entry_size = json_binary::value_entry_size(large);

  /*
    Calculate the offset of the first key relative to the start of the
    object. The first key comes right after the value entries.
  */
  const size_t first_key_offset =
      dest->length() + size * (key_entry_size + value_entry_size) - start_pos;

  // Append all the key entries.
  enum_serialization_result res =
      append_key_entries(object, dest, first_key_offset, large);
  if (res != OK) return res;

  const size_t start_of_value_entries = dest->length();

  // Reserve space for the value entries. Will be filled in later.
  dest->fill(dest->length() + size * value_entry_size, 0);

  // Add the actual keys.
  for (const auto &member : *object) {
    if (dest->append(member.first.c_str(), member.first.length()))
      return FAILURE; /* purecov: inspected */
  }

  // Add the values, and update the value entries accordingly.
  size_t entry_pos = start_of_value_entries;
  for (const auto &member : *object) {
    const Json_dom *child = member.second.get();
    if (!attempt_inline_value(child, dest, entry_pos, large)) {
      size_t offset = dest->length() - start_pos;
      if (is_too_big_for_json(offset, large)) return VALUE_TOO_BIG;
      insert_offset_or_size(dest, entry_pos + 1, offset, large);
      res = serialize_json_value(thd, child, entry_pos, dest, depth, !large);
      if (res != OK) return res;
    }
    entry_pos += value_entry_size;
  }

  // Finally, write the size of the object in bytes.
  size_t bytes = dest->length() - start_pos;
  if (is_too_big_for_json(bytes, large)) return VALUE_TOO_BIG;
  insert_offset_or_size(dest, size_pos, bytes, large);

  return OK;
}

/**
  Serialize a JSON opaque value at the end of the destination string.
  @param[in]  opaque    the JSON opaque value
  @param[in]  type_pos  where to write the type specifier
  @param[out] dest      the destination string
  @return serialization status
*/
static enum_serialization_result serialize_opaque(const Json_opaque *opaque,
                                                  size_t type_pos,
                                                  String *dest) {
  DBUG_ASSERT(type_pos < dest->length());
  if (dest->append(static_cast<char>(opaque->type())) ||
      append_variable_length(dest, opaque->size()) ||
      dest->append(opaque->value(), opaque->size()))
    return FAILURE; /* purecov: inspected */
  (*dest)[type_pos] = JSONB_TYPE_OPAQUE;
  return OK;
}

/**
  Serialize a DECIMAL value at the end of the destination string.
  @param[in]  jd        the DECIMAL value
  @param[in]  type_pos  where to write the type specifier
  @param[out] dest      the destination string
  @return serialization status
*/
static enum_serialization_result serialize_decimal(const Json_decimal *jd,
                                                   size_t type_pos,
                                                   String *dest) {
  // Store DECIMALs as opaque values.
  const int bin_size = jd->binary_size();
  char buf[Json_decimal::MAX_BINARY_SIZE];
  if (jd->get_binary(buf)) return FAILURE; /* purecov: inspected */
  Json_opaque o(MYSQL_TYPE_NEWDECIMAL, buf, bin_size);
  return serialize_opaque(&o, type_pos, dest);
}

/**
  Serialize a DATETIME value at the end of the destination string.
  @param[in]  jdt       the DATETIME value
  @param[in]  type_pos  where to write the type specifier
  @param[out] dest      the destination string
  @return serialization status
*/
static enum_serialization_result serialize_datetime(const Json_datetime *jdt,
                                                    size_t type_pos,
                                                    String *dest) {
  // Store datetime as opaque values.
  char buf[Json_datetime::PACKED_SIZE];
  jdt->to_packed(buf);
  Json_opaque o(jdt->field_type(), buf, sizeof(buf));
  return serialize_opaque(&o, type_pos, dest);
}

/**
  Serialize a JSON value at the end of the destination string.

  Also go back and update the type specifier for the value to specify
  the correct type. For top-level documents, the type specifier is
  located in the byte right in front of the value. For documents that
  are nested within other documents, the type specifier is located in
  the value entry portion at the beginning of the parent document.

  @param thd       THD handle
  @param dom       the JSON value to serialize
  @param type_pos  the position of the type specifier to update
  @param dest      the destination string
  @param depth     the current nesting level
  @param small_parent
                   tells if @a dom is contained in an array or object
                   which is stored in the small storage format
  @return          serialization status
*/
static enum_serialization_result serialize_json_value(
    const THD *thd, const Json_dom *dom, size_t type_pos, String *dest,
    size_t depth, bool small_parent) {
  const size_t start_pos = dest->length();
  DBUG_ASSERT(type_pos < start_pos);

  enum_serialization_result result;

  switch (dom->json_type()) {
    case enum_json_type::J_ARRAY: {
      const Json_array *array = down_cast<const Json_array *>(dom);
      (*dest)[type_pos] = JSONB_TYPE_SMALL_ARRAY;
      result = serialize_json_array(thd, array, dest, false, depth);
      /*
        If the array was too large to fit in the small storage format,
        reset the destination buffer and retry with the large storage
        format.

        Possible future optimization: Analyze size up front and pick the
        correct format on the first attempt, so that we don't have to
        redo parts of the serialization.
      */
      if (result == VALUE_TOO_BIG) {
        // If the parent uses the small storage format, it needs to grow too.
        if (small_parent) return VALUE_TOO_BIG;
        dest->length(start_pos);
        (*dest)[type_pos] = JSONB_TYPE_LARGE_ARRAY;
        result = serialize_json_array(thd, array, dest, true, depth);
      }
      break;
    }
    case enum_json_type::J_OBJECT: {
      const Json_object *object = down_cast<const Json_object *>(dom);
      (*dest)[type_pos] = JSONB_TYPE_SMALL_OBJECT;
      result = serialize_json_object(thd, object, dest, false, depth);
      /*
        If the object was too large to fit in the small storage format,
        reset the destination buffer and retry with the large storage
        format.

        Possible future optimization: Analyze size up front and pick the
        correct format on the first attempt, so that we don't have to
        redo parts of the serialization.
      */
      if (result == VALUE_TOO_BIG) {
        // If the parent uses the small storage format, it needs to grow too.
        if (small_parent) return VALUE_TOO_BIG;
        dest->length(start_pos);
        (*dest)[type_pos] = JSONB_TYPE_LARGE_OBJECT;
        result = serialize_json_object(thd, object, dest, true, depth);
      }
      break;
    }
    case enum_json_type::J_STRING: {
      const Json_string *jstr = down_cast<const Json_string *>(dom);
      size_t size = jstr->size();
      if (append_variable_length(dest, size) ||
          dest->append(jstr->value().c_str(), size))
        return FAILURE; /* purecov: inspected */
      (*dest)[type_pos] = JSONB_TYPE_STRING;
      result = OK;
      break;
    }
    case enum_json_type::J_INT: {
      const Json_int *i = down_cast<const Json_int *>(dom);
      longlong val = i->value();
      if (i->is_16bit()) {
        if (append_int16(dest, static_cast<int16>(val)))
          return FAILURE; /* purecov: inspected */
        (*dest)[type_pos] = JSONB_TYPE_INT16;
      } else if (i->is_32bit()) {
        if (append_int32(dest, static_cast<int32>(val)))
          return FAILURE; /* purecov: inspected */
        (*dest)[type_pos] = JSONB_TYPE_INT32;
      } else {
        if (append_int64(dest, val)) return FAILURE; /* purecov: inspected */
        (*dest)[type_pos] = JSONB_TYPE_INT64;
      }
      result = OK;
      break;
    }
    case enum_json_type::J_UINT: {
      const Json_uint *i = down_cast<const Json_uint *>(dom);
      ulonglong val = i->value();
      if (i->is_16bit()) {
        if (append_int16(dest, static_cast<int16>(val)))
          return FAILURE; /* purecov: inspected */
        (*dest)[type_pos] = JSONB_TYPE_UINT16;
      } else if (i->is_32bit()) {
        if (append_int32(dest, static_cast<int32>(val)))
          return FAILURE; /* purecov: inspected */
        (*dest)[type_pos] = JSONB_TYPE_UINT32;
      } else {
        if (append_int64(dest, val)) return FAILURE; /* purecov: inspected */
        (*dest)[type_pos] = JSONB_TYPE_UINT64;
      }
      result = OK;
      break;
    }
    case enum_json_type::J_DOUBLE: {
      // Store the double in a platform-independent eight-byte format.
      const Json_double *d = down_cast<const Json_double *>(dom);
      if (reserve(dest, 8)) return FAILURE; /* purecov: inspected */
      float8store(dest->ptr() + dest->length(), d->value());
      dest->length(dest->length() + 8);
      (*dest)[type_pos] = JSONB_TYPE_DOUBLE;
      result = OK;
      break;
    }
    case enum_json_type::J_NULL:
      if (dest->append(JSONB_NULL_LITERAL))
        return FAILURE; /* purecov: inspected */
      (*dest)[type_pos] = JSONB_TYPE_LITERAL;
      result = OK;
      break;
    case enum_json_type::J_BOOLEAN: {
      char c = (down_cast<const Json_boolean *>(dom)->value())
                   ? JSONB_TRUE_LITERAL
                   : JSONB_FALSE_LITERAL;
      if (dest->append(c)) return FAILURE; /* purecov: inspected */
      (*dest)[type_pos] = JSONB_TYPE_LITERAL;
      result = OK;
      break;
    }
    case enum_json_type::J_OPAQUE:
      result =
          serialize_opaque(down_cast<const Json_opaque *>(dom), type_pos, dest);
      break;
    case enum_json_type::J_DECIMAL:
      result = serialize_decimal(down_cast<const Json_decimal *>(dom), type_pos,
                                 dest);
      break;
    case enum_json_type::J_DATETIME:
    case enum_json_type::J_DATE:
    case enum_json_type::J_TIME:
    case enum_json_type::J_TIMESTAMP:
      result = serialize_datetime(down_cast<const Json_datetime *>(dom),
                                  type_pos, dest);
      break;
    default:
      /* purecov: begin deadcode */
      DBUG_ASSERT(false);
      my_error(ER_INTERNAL_ERROR, MYF(0), "JSON serialization failed");
      return FAILURE;
      /* purecov: end */
  }

  if (result == OK && dest->length() > thd->variables.max_allowed_packet) {
    my_error(ER_WARN_ALLOWED_PACKET_OVERFLOWED, MYF(0),
             "json_binary::serialize", thd->variables.max_allowed_packet);
    return FAILURE;
  }

  return result;
}
#endif  // ifdef MYSQL_SERVER

bool Value::is_valid() const {
  switch (m_type) {
    case ERROR:
      return false;
    case ARRAY:
      // Check that all the array elements are valid.
      for (size_t i = 0; i < element_count(); i++)
        if (!element(i).is_valid()) return false; /* purecov: inspected */
      return true;
    case OBJECT: {
      /*
        Check that all keys and values are valid, and that the keys come
        in the correct order.
      */
      const char *prev_key = nullptr;
      size_t prev_key_len = 0;
      for (size_t i = 0; i < element_count(); i++) {
        Value k = key(i);
        if (!k.is_valid() || !element(i).is_valid())
          return false; /* purecov: inspected */
        const char *curr_key = k.get_data();
        size_t curr_key_len = k.get_data_length();
        if (i > 0) {
          if (prev_key_len > curr_key_len)
            return false; /* purecov: inspected */
          if (prev_key_len == curr_key_len &&
              (memcmp(prev_key, curr_key, curr_key_len) >= 0))
            return false; /* purecov: inspected */
        }
        prev_key = curr_key;
        prev_key_len = curr_key_len;
      }
      return true;
    }
    default:
      // This is a valid scalar value.
      return true;
  }
}

/**
  Create a Value object that represents an error condition.
*/
static Value err() { return Value(Value::ERROR); }

/**
  Parse a JSON scalar value.

  @param type   the binary type of the scalar
  @param data   pointer to the start of the binary representation of the scalar
  @param len    the maximum number of bytes to read from data
  @return  an object that represents the scalar value
*/
static Value parse_scalar(uint8 type, const char *data, size_t len) {
  switch (type) {
    case JSONB_TYPE_LITERAL:
      if (len < 1) return err(); /* purecov: inspected */
      switch (static_cast<uint8>(*data)) {
        case JSONB_NULL_LITERAL:
          return Value(Value::LITERAL_NULL);
        case JSONB_TRUE_LITERAL:
          return Value(Value::LITERAL_TRUE);
        case JSONB_FALSE_LITERAL:
          return Value(Value::LITERAL_FALSE);
        default:
          return err(); /* purecov: inspected */
      }
    case JSONB_TYPE_INT16:
      if (len < 2) return err(); /* purecov: inspected */
      return Value(Value::INT, sint2korr(data));
    case JSONB_TYPE_INT32:
      if (len < 4) return err(); /* purecov: inspected */
      return Value(Value::INT, sint4korr(data));
    case JSONB_TYPE_INT64:
      if (len < 8) return err(); /* purecov: inspected */
      return Value(Value::INT, sint8korr(data));
    case JSONB_TYPE_UINT16:
      if (len < 2) return err(); /* purecov: inspected */
      return Value(Value::UINT, uint2korr(data));
    case JSONB_TYPE_UINT32:
      if (len < 4) return err(); /* purecov: inspected */
      return Value(Value::UINT, uint4korr(data));
    case JSONB_TYPE_UINT64:
      if (len < 8) return err(); /* purecov: inspected */
      return Value(Value::UINT, uint8korr(data));
    case JSONB_TYPE_DOUBLE: {
      if (len < 8) return err(); /* purecov: inspected */
      return Value(float8get(data));
    }
    case JSONB_TYPE_STRING: {
      uint32 str_len;
      uint8 n;
      if (read_variable_length(data, len, &str_len, &n))
        return err();                      /* purecov: inspected */
      if (len < n + str_len) return err(); /* purecov: inspected */
      return Value(data + n, str_len);
    }
    case JSONB_TYPE_OPAQUE: {
      /*
        There should always be at least one byte, which tells the field
        type of the opaque value.
      */
      if (len < 1) return err(); /* purecov: inspected */

      // The type is encoded as a uint8 that maps to an enum_field_types.
      uint8 type_byte = static_cast<uint8>(*data);
      enum_field_types field_type = static_cast<enum_field_types>(type_byte);

      // Then there's the length of the value.
      uint32 val_len;
      uint8 n;
      if (read_variable_length(data + 1, len - 1, &val_len, &n))
        return err();                          /* purecov: inspected */
      if (len < 1 + n + val_len) return err(); /* purecov: inspected */
      return Value(field_type, data + 1 + n, val_len);
    }
    default:
      // Not a valid scalar type.
      return err();
  }
}

/**
  Read an offset or size field from a buffer. The offset could be either
  a two byte unsigned integer or a four byte unsigned integer.

  @param data  the buffer to read from
  @param large tells if the large or small storage format is used; true
               means read four bytes, false means read two bytes
*/
static uint32 read_offset_or_size(const char *data, bool large) {
  return large ? uint4korr(data) : uint2korr(data);
}

/**
  Parse a JSON array or object.

  @param t      type (either ARRAY or OBJECT)
  @param data   pointer to the start of the array or object
  @param len    the maximum number of bytes to read from data
  @param large  if true, the array or object is stored using the large
                storage format; otherwise, it is stored using the small
                storage format
  @return  an object that allows access to the array or object
*/
static Value parse_array_or_object(Value::enum_type t, const char *data,
                                   size_t len, bool large) {
  DBUG_ASSERT(t == Value::ARRAY || t == Value::OBJECT);

  /*
    Make sure the document is long enough to contain the two length fields
    (both number of elements or members, and number of bytes).
  */
  const auto offset_size = json_binary::offset_size(large);
  if (len < 2 * offset_size) return err();
  const uint32 element_count = read_offset_or_size(data, large);
  const uint32 bytes = read_offset_or_size(data + offset_size, large);

  // The value can't have more bytes than what's available in the data buffer.
  if (bytes > len) return err();

  /*
    Calculate the size of the header. It consists of:
    - two length fields
    - if it is a JSON object, key entries with pointers to where the keys
      are stored
    - value entries with pointers to where the actual values are stored
  */
  size_t header_size = 2 * offset_size;
  if (t == Value::OBJECT) header_size += element_count * key_entry_size(large);
  header_size += element_count * value_entry_size(large);

  // The header should not be larger than the full size of the value.
  if (header_size > bytes) return err(); /* purecov: inspected */

  return Value(t, data, bytes, element_count, large);
}

/**
  Parse a JSON value within a larger JSON document.

  @param type   the binary type of the value to parse
  @param data   pointer to the start of the binary representation of the value
  @param len    the maximum number of bytes to read from data
  @return  an object that allows access to the value
*/
static Value parse_value(uint8 type, const char *data, size_t len) {
  switch (type) {
    case JSONB_TYPE_SMALL_OBJECT:
      return parse_array_or_object(Value::OBJECT, data, len, false);
    case JSONB_TYPE_LARGE_OBJECT:
      return parse_array_or_object(Value::OBJECT, data, len, true);
    case JSONB_TYPE_SMALL_ARRAY:
      return parse_array_or_object(Value::ARRAY, data, len, false);
    case JSONB_TYPE_LARGE_ARRAY:
      return parse_array_or_object(Value::ARRAY, data, len, true);
    default:
      return parse_scalar(type, data, len);
  }
}

Value parse_binary(const char *data, size_t len) {
  DBUG_TRACE;
  /*
    Each document should start with a one-byte type specifier, so an
    empty document is invalid according to the format specification.
    Empty documents may appear due to inserts using the IGNORE keyword
    or with non-strict SQL mode, which will insert an empty string if
    the value NULL is inserted into a NOT NULL column. We choose to
    interpret empty values as the JSON null literal.
  */
  if (len == 0) return Value(Value::LITERAL_NULL);

  Value ret = parse_value(data[0], data + 1, len - 1);
  return ret;
}

/**
  Get the element at the specified position of a JSON array or a JSON
  object. When called on a JSON object, it returns the value
  associated with the key returned by key(pos).

  @param pos  the index of the element
  @return a value representing the specified element, or a value where
  type() returns ERROR if pos does not point to an element
*/
Value Value::element(size_t pos) const {
  DBUG_ASSERT(m_type == ARRAY || m_type == OBJECT);

  if (pos >= m_element_count) return err();

  const auto entry_size = value_entry_size(m_large);
  const auto entry_offset = value_entry_offset(pos);

  uint8 type = m_data[entry_offset];

  /*
    Check if this is an inlined scalar value. If so, return it.
    The scalar will be inlined just after the byte that identifies the
    type, so it's found on entry_offset + 1.
  */
  if (inlined_type(type, m_large))
    return parse_scalar(type, m_data + entry_offset + 1, entry_size - 1);

  /*
    Otherwise, it's a non-inlined value, and the offset to where the value
    is stored, can be found right after the type byte in the entry.
  */
  uint32 value_offset = read_offset_or_size(m_data + entry_offset + 1, m_large);

  if (m_length < value_offset || value_offset < entry_offset + entry_size)
    return err(); /* purecov: inspected */

  return parse_value(type, m_data + value_offset, m_length - value_offset);
}

/**
  Get the key of the member stored at the specified position in a JSON
  object.

  @param pos  the index of the member
  @return the key of the specified member, or a value where type()
  returns ERROR if pos does not point to a member
*/
Value Value::key(size_t pos) const {
  DBUG_ASSERT(m_type == OBJECT);

  if (pos >= m_element_count) return err();

  const auto offset_size = json_binary::offset_size(m_large);
  const auto key_entry_size = json_binary::key_entry_size(m_large);
  const auto value_entry_size = json_binary::value_entry_size(m_large);

  // The key entries are located after two length fields of size offset_size.
  const size_t entry_offset = key_entry_offset(pos);

  // The offset of the key is the first part of the key entry.
  const uint32 key_offset = read_offset_or_size(m_data + entry_offset, m_large);

  // The length of the key is the second part of the entry, always two bytes.
  const uint16 key_length = uint2korr(m_data + entry_offset + offset_size);

  /*
    The key must start somewhere after the last value entry, and it must
    end before the end of the m_data buffer.
  */
  if ((key_offset < entry_offset + (m_element_count - pos) * key_entry_size +
                        m_element_count * value_entry_size) ||
      (m_length < key_offset + key_length))
    return err(); /* purecov: inspected */

  return Value(m_data + key_offset, key_length);
}

/**
  Get the value associated with the specified key in a JSON object.

  @param[in] key  the key to look up
  @param[in] length  the length of the key
  @return the value associated with the key, if there is one. otherwise,
  returns ERROR
*/
Value Value::lookup(const char *key, size_t length) const {
  size_t index = lookup_index(key, length);
  if (index == element_count()) return err();
  return element(index);
}

/**
  Get the index of the element with the specified key in a JSON object.

  @param[in] key  the key to look up
  @param[in] length  the length of the key
  @return the index if the key is found, or `element_count()` if the
  key is not found
*/
size_t Value::lookup_index(const char *key, size_t length) const {
  DBUG_ASSERT(m_type == OBJECT);

  const auto offset_size = json_binary::offset_size(m_large);
  const auto entry_size = key_entry_size(m_large);

  const size_t first_entry_offset = key_entry_offset(0);

  size_t lo = 0U;               // lower bound for binary search (inclusive)
  size_t hi = m_element_count;  // upper bound for binary search (exclusive)

  while (lo < hi) {
    // Find the entry in the middle of the search interval.
    size_t idx = (lo + hi) / 2;
    size_t entry_offset = first_entry_offset + idx * entry_size;

    // Keys are ordered on length, so check length first.
    size_t key_len = uint2korr(m_data + entry_offset + offset_size);
    if (length > key_len) {
      lo = idx + 1;
    } else if (length < key_len) {
      hi = idx;
    } else {
      // The keys had the same length, so compare their contents.
      size_t key_offset = read_offset_or_size(m_data + entry_offset, m_large);

      int cmp = memcmp(key, m_data + key_offset, key_len);
      if (cmp > 0)
        lo = idx + 1;
      else if (cmp < 0)
        hi = idx;
      else
        return idx;
    }
  }

  return m_element_count;  // not found
}

/**
  Is this binary value pointing to data that is contained in the specified
  string.

  @param str     a string with binary data
  @retval true   if the string contains data pointed to from this object
  @retval false  otherwise
*/
bool Value::is_backed_by(const String *str) const {
  /*
    The m_data member is only valid for objects, arrays, strings and opaque
    values. Other types have copied the necessary data into the Value object
    and do not depend on data in any String object.
  */
  switch (m_type) {
    case OBJECT:
    case ARRAY:
    case STRING:
    case OPAQUE:
      return m_data >= str->ptr() && m_data < str->ptr() + str->length();
    default:
      return false;
  }
}

/**
  Copy the binary representation of this value into a buffer,
  replacing the contents of the receiving buffer.

  @param thd  THD handle
  @param buf  the receiving buffer
  @return false on success, true otherwise
*/
#ifdef MYSQL_SERVER
bool Value::raw_binary(const THD *thd, String *buf) const {
  // It's not safe to overwrite ourselves.
  DBUG_ASSERT(!is_backed_by(buf));

  // Reset the buffer.
  buf->length(0);
  buf->set_charset(&my_charset_bin);

  switch (m_type) {
    case OBJECT:
    case ARRAY: {
      char tp = m_large ? (m_type == OBJECT ? JSONB_TYPE_LARGE_OBJECT
                                            : JSONB_TYPE_LARGE_ARRAY)
                        : (m_type == OBJECT ? JSONB_TYPE_SMALL_OBJECT
                                            : JSONB_TYPE_SMALL_ARRAY);
      return buf->append(tp) || buf->append(m_data, m_length);
    }
    case STRING:
      return buf->append(JSONB_TYPE_STRING) ||
             append_variable_length(buf, m_length) ||
             buf->append(m_data, m_length);
    case INT: {
      Json_int i(get_int64());
      return serialize(thd, &i, buf) != OK;
    }
    case UINT: {
      Json_uint i(get_uint64());
      return serialize(thd, &i, buf) != OK;
    }
    case DOUBLE: {
      Json_double d(get_double());
      return serialize(thd, &d, buf) != OK;
    }
    case LITERAL_NULL: {
      Json_null n;
      return serialize(thd, &n, buf) != OK;
    }
    case LITERAL_TRUE:
    case LITERAL_FALSE: {
      Json_boolean b(m_type == LITERAL_TRUE);
      return serialize(thd, &b, buf) != OK;
    }
    case OPAQUE:
      return buf->append(JSONB_TYPE_OPAQUE) || buf->append(field_type()) ||
             append_variable_length(buf, m_length) ||
             buf->append(m_data, m_length);
    case ERROR:
      break; /* purecov: inspected */
  }

  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return true;
  /* purecov: end */
}
#endif  // ifdef MYSQL_SERVER

/**
  Find the start offset and the end offset of the specified element.
  @param[in]  pos     which element to check
  @param[out] start   the start offset of the value
  @param[out] end     the end offset of the value (exclusive)
  @param[out] inlined set to true if the specified element is inlined
  @return true if the offsets cannot be determined, false if successful
*/
bool Value::element_offsets(size_t pos, size_t *start, size_t *end,
                            bool *inlined) const {
  DBUG_ASSERT(m_type == ARRAY || m_type == OBJECT);
  DBUG_ASSERT(pos < m_element_count);

  const char *entry = m_data + value_entry_offset(pos);
  if (entry + value_entry_size(m_large) > m_data + m_length)
    return true; /* purecov: inspected */

  if (inlined_type(*entry, m_large)) {
    *start = 0;
    *end = 0;
    *inlined = true;
    return false;
  }

  const size_t val_pos = read_offset_or_size(entry + 1, m_large);
  if (val_pos >= m_length) return true;

  size_t val_end = 0;
  switch (entry[0]) {
    case JSONB_TYPE_INT32:
    case JSONB_TYPE_UINT32:
      val_end = val_pos + 4;
      break;
    case JSONB_TYPE_INT64:
    case JSONB_TYPE_UINT64:
    case JSONB_TYPE_DOUBLE:
      val_end = val_pos + 8;
      break;
    case JSONB_TYPE_STRING:
    case JSONB_TYPE_OPAQUE:
    case JSONB_TYPE_SMALL_OBJECT:
    case JSONB_TYPE_LARGE_OBJECT:
    case JSONB_TYPE_SMALL_ARRAY:
    case JSONB_TYPE_LARGE_ARRAY: {
      Value v = element(pos);
      if (v.type() == ERROR) return true;
      val_end = (v.m_data - this->m_data) + v.m_length;
    } break;
    default:
      return true;
  }

  *start = val_pos;
  *end = val_end;
  *inlined = false;
  return false;
}

/**
  Find the lowest possible offset where a value can be located inside this
  array or object.

  @param[out] offset   the lowest offset where a value can be located
  @return false on success, true on error
*/
bool Value::first_value_offset(size_t *offset) const {
  DBUG_ASSERT(m_type == ARRAY || m_type == OBJECT);

  /*
    Find the lowest offset where a value could be stored. Arrays can
    store them right after the last value entry. Objects can store
    them right after the last key.
  */
  if (m_type == ARRAY || m_element_count == 0) {
    *offset = value_entry_offset(m_element_count);
    return false;
  }

  Value key = this->key(m_element_count - 1);
  if (key.type() == ERROR) return true;

  *offset = key.get_data() + key.get_data_length() - m_data;
  return false;
}

/**
  Does this array or object have enough space to replace the value at
  the given position with another value of a given size?

  @param[in]  pos     the position in the array or object
  @param[in]  needed  the number of bytes needed for the new value
  @param[out] offset  if true is returned, this value is set to an
                      offset relative to the start of the array or
                      object, which tells where the replacement value
                      should be stored
  @return true if there is enough space, false otherwise
*/
bool Value::has_space(size_t pos, size_t needed, size_t *offset) const {
  DBUG_ASSERT(m_type == ARRAY || m_type == OBJECT);
  DBUG_ASSERT(pos < m_element_count);

  /*
    Find the lowest offset where a value could be stored. Arrays can
    store them right after the last value entry. Objects can store
    them right after the last key.
  */
  size_t first_value_offset;
  if (this->first_value_offset(&first_value_offset)) return false;

  /*
    No need to check further if we need more space than the total
    space available in the array or object.
  */
  if (needed > m_length - first_value_offset) return false;

  size_t val_start;
  size_t val_end;
  bool inlined;
  if (element_offsets(pos, &val_start, &val_end, &inlined)) return false;

  if (!inlined && val_end - val_start >= needed) {
    // Found enough space at the position where the original value was located.
    *offset = val_start;
    return true;
  }

  /*
    Need more space. Look for free space after the original value.
    There's potential free space after the end of the original value
    and up to the start of the next non-inlined value.
  */
  const auto entry_size = value_entry_size(m_large);
  size_t i = pos + 1;
  for (auto entry = m_data + value_entry_offset(pos); i < m_element_count;
       ++i) {
    entry += entry_size;
    // TODO Give up after N iterations?
    if (inlined_type(*entry, m_large)) continue;
    val_end = read_offset_or_size(entry + 1, m_large);
    if (val_end > m_length) return false;
    break;
  }

  if (i == m_element_count) {
    /*
      There are no non-inlined values behind the one we are updating,
      so we can use the rest of the space allocated for the array or
      object.
    */
    val_end = m_length;
  }

  if (!inlined && val_end - val_start >= needed) {
    *offset = val_start;
    return true;
  }

  /*
    Still not enough space. See if there's free space we can use in
    front of the original value. We can use space after the end of the
    first non-inlined value we find.
  */
  if (needed > val_end - first_value_offset) return false;
  for (i = pos; i > 0; --i) {
    size_t elt_start;
    size_t elt_end;
    bool elt_inlined;
    if (element_offsets(i - 1, &elt_start, &elt_end, &elt_inlined))
      return false;
    if (elt_inlined) continue;
    val_start = elt_end;
    break;
  }

  if (i == 0) {
    /*
      There are no non-inlined values ahead of the value we are
      updating, so we can start right after the value entries.
    */
    val_start = first_value_offset;
  }

  if (val_start >= first_value_offset && val_end <= m_length &&
      val_start <= val_end && val_end - val_start >= needed) {
    *offset = val_start;
    return true;
  }

  return false;
}

/**
  Get the offset of the key entry that describes the key of the member at a
  given position in this object.

  @param pos   the position of the member
  @return the offset of the key entry, relative to the start of the object
*/
inline size_t Value::key_entry_offset(size_t pos) const {
  DBUG_ASSERT(m_type == OBJECT);
  // The first key entry is located right after the two length fields.
  return 2 * offset_size(m_large) + key_entry_size(m_large) * pos;
}

/**
  Get the offset of the value entry that describes the element at a
  given position in this array or object.

  @param pos  the position of the element
  @return the offset of the entry, relative to the start of the array or object
*/
inline size_t Value::value_entry_offset(size_t pos) const {
  DBUG_ASSERT(m_type == ARRAY || m_type == OBJECT);
  /*
    Value entries come after the two length fields if it's an array, or
    after the two length fields and all the key entries if it's an object.
  */
  size_t first_entry_offset = 2 * offset_size(m_large);
  if (m_type == OBJECT)
    first_entry_offset += m_element_count * key_entry_size(m_large);

  return first_entry_offset + value_entry_size(m_large) * pos;
}

#ifdef MYSQL_SERVER
bool space_needed(const THD *thd, const Json_wrapper *value, bool large,
                  size_t *needed) {
  if (value->type() == enum_json_type::J_ERROR) {
    my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
    return true;
  }

  // Serialize the value to a temporary buffer to find out how big it is.
  StringBuffer<STRING_BUFFER_USUAL_SIZE> buf;
  if (value->to_binary(thd, &buf)) return true; /* purecov: inspected */

  DBUG_ASSERT(buf.length() > 1);

  // If the value can be inlined in the value entry, it doesn't need any space.
  if (inlined_type(buf[0], large)) {
    *needed = 0;
    return false;
  }

  /*
    The first byte in the buffer is the type identifier. We're only
    interested in the size of the data portion, so exclude the type byte
    from the returned size.
  */
  *needed = buf.length() - 1;
  return false;
}

/**
  Update a value in an array or object. The updated value is written to a
  shadow copy. The original array or object is left unchanged, unless the
  shadow copy is actually a pointer to the array backing this Value object. It
  is assumed that the shadow copy is at least as big as the original document,
  and that there is enough space at the given position to hold the new value.

  Typically, if a document is modified multiple times in a single update
  statement, the first invocation of update_in_shadow() will have a Value
  object that points into the binary data in the Field, and write to a separate
  destination buffer. Subsequent updates of the document will have a Value
  object that points to the partially updated value in the destination buffer,
  and write the new modifications to the same buffer.

  All changes made to the binary value are recorded as binary diffs using
  TABLE::add_binary_diff().

  @param field         the column that is updated
  @param pos           the element to update
  @param new_value     the new value of the element
  @param data_offset   where to write the value (offset relative to the
                       beginning of the array or object, obtained with
                       #has_space) or zero if the value can be inlined
  @param data_length   the length of the new value in bytes or zero if
                       the value can be inlined
  @param original      pointer to the start of the JSON document
  @param destination   pointer to the shadow copy of the JSON document
                       (it could be the same as @a original, in which case the
                       original document will be modified)
  @param[out] changed  gets set to true if a change was made to the document,
                       or to false if this operation was a no-op
  @return false on success, true if an error occurred

  @par Example of partial update

  Given the JSON document [ "abc", "def" ], which is serialized like this in a
  JSON column:

      0x02 - type: small JSON array
      0x02 - number of elements (low byte)
      0x00 - number of elements (high byte)
      0x12 - number of bytes (low byte)
      0x00 - number of bytes (high byte)
      0x0C - type of element 0 (string)
      0x0A - offset of element 0 (low byte)
      0x00 - offset of element 0 (high byte)
      0x0C - type of element 1 (string)
      0x0E - offset of element 1 (low byte)
      0x00 - offset of element 1 (high byte)
      0x03 - length of element 0
      'a'
      'b'  - content of element 0
      'c'
      0x03 - length of element 1
      'd'
      'e'  - content of element 1
      'f'

  Let's change element 0 from "abc" to "XY" using the following statement:

      UPDATE t SET j = JSON_SET(j, '$[0]', 'XY')

  Since we're replacing one string with a shorter one, we can just overwrite
  the length byte with the new length, and the beginning of the original string
  data. Since the original string "abc" is longer than the new string "XY",
  we'll have a free byte at the end of the string. This byte is left as is
  ('c'). The resulting binary representation looks like this:

              0x02 - type: small JSON array
              0x02 - number of elements (low byte)
              0x00 - number of elements (high byte)
              0x12 - number of bytes (low byte)
              0x00 - number of bytes (high byte)
              0x0C - type of element 0 (string)
              0x0A - offset of element 0 (low byte)
              0x00 - offset of element 0 (high byte)
              0x0C - type of element 1 (string)
              0x0E - offset of element 1 (low byte)
              0x00 - offset of element 1 (high byte)
      CHANGED 0x02 - length of element 0
      CHANGED 'X'
      CHANGED 'Y'  - content of element 0
      (free)  'c'
              0x03 - length of element 1
              'd'
              'e'  - content of element 1
              'f'

  This change will be represented as one binary diff that covers the three
  changed bytes.

  Let's now change element 1 from "def" to "XYZW":

      UPDATE t SET j = JSON_SET(j, '$[1]', 'XYZW')

  Since the new string is one byte longer than the original string, we cannot
  simply overwrite the old one. But we can reuse the free byte from the
  previous update, which is immediately preceding the original value.

  To make use of this, we need to change the offset of element 1 to point to
  the free byte. Then we can overwrite the free byte and the original string
  data with the new length and string contents. Resulting binary
  representation:

              0x02 - type: small JSON array
              0x02 - number of elements (low byte)
              0x00 - number of elements (high byte)
              0x12 - number of bytes (low byte)
              0x00 - number of bytes (high byte)
              0x0C - type of element 0 (string)
              0x0A - offset of element 0 (low byte)
              0x00 - offset of element 0 (high byte)
              0x0C - type of element 1 (string)
      CHANGED 0x0D - offset of element 1 (low byte)
              0x00 - offset of element 1 (high byte)
              0x02 - length of element 0
              'X'  - content of element 0
              'Y'  - content of element 0
      CHANGED 0x04 - length of element 1
      CHANGED 'X'
      CHANGED 'Y'
      CHANGED 'Z'  - content of element 1
      CHANGED 'W'

  This change will be represented as two binary diffs. One diff for changing
  the offset, and one for changing the contents of the string.

  Then let's replace the string in element 1 with a small number:

      UPDATE t SET j = JSON_SET(j, '$[1]', 456)

  This will change the type of element 1 from string to int16. Such small
  numbers are inlined in the value entry, where we normally store the offset of
  the value. The offset section of the value entry is therefore changed to hold
  the number 456. The length and contents of the original value ("XYZW") are
  not touched, but they are now unused and free to be reused. Resulting binary
  representation:

              0x02 - type: small JSON array
              0x02 - number of elements (low byte)
              0x00 - number of elements (high byte)
              0x12 - number of bytes (low byte)
              0x00 - number of bytes (high byte)
              0x0C - type of element 0 (string)
              0x0A - offset of element 0 (low byte)
              0x00 - offset of element 0 (high byte)
      CHANGED 0x05 - type of element 1 (int16)
      CHANGED 0xC8 - value of element 1 (low byte)
      CHANGED 0x01 - value of element 1 (high byte)
              0x02 - length of element 0
              'X'  - content of element 0
              'Y'  - content of element 0
      (free)  0x04 - length of element 1
      (free)  'X'
      (free)  'Y'
      (free)  'Z'  - content of element 1
      (free)  'W'

  The change is represented as one binary diff that changes the value entry
  (type and inlined value).
*/
bool Value::update_in_shadow(const Field_json *field, size_t pos,
                             Json_wrapper *new_value, size_t data_offset,
                             size_t data_length, const char *original,
                             char *destination, bool *changed) const {
  DBUG_ASSERT(m_type == ARRAY || m_type == OBJECT);

  const bool inlined = (data_length == 0);

  // Assume no changes. Update the flag when the document is actually changed.
  *changed = false;

  /*
    Create a buffer large enough to hold the new value entry. (Plus one since
    some String functions insist on adding a terminating '\0'.)
  */
  StringBuffer<VALUE_ENTRY_SIZE_LARGE + 1> new_entry;

  if (inlined) {
    new_entry.length(value_entry_size(m_large));
    Json_dom *dom = new_value->to_dom(field->table->in_use);
    if (dom == nullptr) return true; /* purecov: inspected */
    attempt_inline_value(dom, &new_entry, 0, m_large);
  } else {
    new_entry.append('\0');  // type, to be filled in later
    append_offset_or_size(&new_entry, data_offset, m_large);

    const char *value = m_data + data_offset;
    const size_t value_offset = value - original;
    char *value_dest = destination + value_offset;

    StringBuffer<STRING_BUFFER_USUAL_SIZE> buffer;
    if (new_value->to_binary(field->table->in_use, &buffer))
      return true; /* purecov: inspected */

    DBUG_ASSERT(buffer.length() > 1);

    // The first byte is the type byte, which should be in the value entry.
    new_entry[0] = buffer[0];

    /*
      Create another diff for the changed data, but only if the new data is
      actually different from the old data.
    */
    const size_t length = buffer.length() - 1;
    DBUG_ASSERT(length == data_length);
    if (memcmp(value_dest, buffer.ptr() + 1, length) != 0) {
      memcpy(value_dest, buffer.ptr() + 1, length);
      if (field->table->add_binary_diff(field, value_offset, length))
        return true; /* purecov: inspected */
      *changed = true;
    }
  }

  DBUG_ASSERT(new_entry.length() == value_entry_size(m_large));

  /*
    Type and offset will often be unchanged. Don't create a change
    record unless they have actually changed.
  */
  const char *const entry = m_data + value_entry_offset(pos);
  if (memcmp(entry, new_entry.ptr(), new_entry.length()) != 0) {
    const size_t entry_offset = entry - original;
    memcpy(destination + entry_offset, new_entry.ptr(), new_entry.length());
    if (field->table->add_binary_diff(field, entry_offset, new_entry.length()))
      return true; /* purecov: inspected */
    *changed = true;
  }

  return false;
}

/**
  Remove a value from an array or object. The updated JSON document is written
  to a shadow copy. The original document is left unchanged, unless the shadow
  copy is actually a pointer to the array backing this Value object. It is
  assumed that the shadow copy is at least as big as the original document, and
  that there is enough space at the given position to hold the new value.

  Typically, if a document is modified multiple times in a single update
  statement, the first invocation of remove_in_shadow() will have a Value
  object that points into the binary data in the Field, and write to a separate
  destination buffer. Subsequent updates of the document will have a Value
  object that points to the partially updated value in the destination buffer,
  and write the new modifications to the same buffer.

  All changes made to the binary value are recorded as binary diffs using
  TABLE::add_binary_diff().

  @param field         the column that is updated
  @param pos           the element to remove
  @param original      pointer to the start of the JSON document
  @param destination   pointer to the shadow copy of the JSON document
                       (it could be the same as @a original, in which case the
                       original document will be modified)
  @return false on success, true if an error occurred

  @par Example of partial update

  Take the JSON document { "a": "x", "b": "y", "c": "z" }, whose serialized
  representation looks like the following:

              0x00 - type: JSONB_TYPE_SMALL_OBJECT
              0x03 - number of elements (low byte)
              0x00 - number of elements (high byte)
              0x22 - number of bytes (low byte)
              0x00 - number of bytes (high byte)
              0x19 - offset of key "a" (high byte)
              0x00 - offset of key "a" (low byte)
              0x01 - length of key "a" (high byte)
              0x00 - length of key "a" (low byte)
              0x1a - offset of key "b" (high byte)
              0x00 - offset of key "b" (low byte)
              0x01 - length of key "b" (high byte)
              0x00 - length of key "b" (low byte)
              0x1b - offset of key "c" (high byte)
              0x00 - offset of key "c" (low byte)
              0x01 - length of key "c" (high byte)
              0x00 - length of key "c" (low byte)
              0x0c - type of value "a": JSONB_TYPE_STRING
              0x1c - offset of value "a" (high byte)
              0x00 - offset of value "a" (low byte)
              0x0c - type of value "b": JSONB_TYPE_STRING
              0x1e - offset of value "b" (high byte)
              0x00 - offset of value "b" (low byte)
              0x0c - type of value "c": JSONB_TYPE_STRING
              0x20 - offset of value "c" (high byte)
              0x00 - offset of value "c" (low byte)
              0x61 - first key  ('a')
              0x62 - second key ('b')
              0x63 - third key  ('c')
              0x01 - length of value "a"
              0x78 - contents of value "a" ('x')
              0x01 - length of value "b"
              0x79 - contents of value "b" ('y')
              0x01 - length of value "c"
              0x7a - contents of value "c" ('z')

  We remove the member with name 'b' from the document, using a statement such
  as:

      UPDATE t SET j = JSON_REMOVE(j, '$.b')

  This function will then remove the element by moving the key entries and
  value entries that follow the removed member so that they overwrite the
  existing entries, and the element count is decremented.

  The resulting binary document will look like this:

              0x00 - type: JSONB_TYPE_SMALL_OBJECT
      CHANGED 0x02 - number of elements (low byte)
              0x00 - number of elements (high byte)
              0x22 - number of bytes (low byte)
              0x00 - number of bytes (high byte)
              0x19 - offset of key "a" (high byte)
              0x00 - offset of key "a" (low byte)
              0x01 - length of key "a" (high byte)
              0x00 - length of key "a" (low byte)
      CHANGED 0x1b - offset of key "c" (high byte)
      CHANGED 0x00 - offset of key "c" (low byte)
      CHANGED 0x01 - length of key "c" (high byte)
      CHANGED 0x00 - length of key "c" (low byte)
      CHANGED 0x0c - type of value "a": JSONB_TYPE_STRING
      CHANGED 0x1c - offset of value "a" (high byte)
      CHANGED 0x00 - offset of value "a" (low byte)
      CHANGED 0x0c - type of value "c": JSONB_TYPE_STRING
      CHANGED 0x20 - offset of value "c" (high byte)
      CHANGED 0x00 - offset of value "c" (low byte)
      (free)  0x00
      (free)  0x0c
      (free)  0x1e
      (free)  0x00
      (free)  0x0c
      (free)  0x20
      (free)  0x00
              0x61 - first key  ('a')
      (free)  0x62
              0x63 - third key  ('c')
              0x01 - length of value "a"
              0x78 - contents of value "a" ('x')
      (free)  0x01
      (free)  0x79
              0x01 - length of value "c"
              0x7a - contents of value "c" ('z')

  Two binary diffs will be created. One diff changes the element count, and one
  diff changes the key and value entries.
*/
bool Value::remove_in_shadow(const Field_json *field, size_t pos,
                             const char *original, char *destination) const {
  DBUG_ASSERT(m_type == ARRAY || m_type == OBJECT);

  const char *value_entry = m_data + value_entry_offset(pos);
  const char *next_value_entry = value_entry + value_entry_size(m_large);

  /*
    If it's an object, we first remove the key entry by shifting all subsequent
    key entries to the left, and also all value entries up to the one that's
    being removed.
  */
  if (m_type == OBJECT) {
    const char *key_entry = m_data + key_entry_offset(pos);
    const char *next_key_entry = key_entry + key_entry_size(m_large);
    size_t len = value_entry - next_key_entry;
    memmove(destination + (key_entry - original), next_key_entry, len);
    if (field->table->add_binary_diff(field, key_entry - original, len))
      return true; /* purecov: inspected */

    /*
      Adjust the destination of the value entry to account for the removed key
      entry.
    */
    value_entry -= key_entry_size(m_large);
  }

  /*
    Next, remove the value entry by shifting all subsequent value entries to
    the left.
  */
  const char *value_entry_end = m_data + value_entry_offset(m_element_count);
  size_t len = value_entry_end - next_value_entry;
  memmove(destination + (value_entry - original), next_value_entry, len);
  if (field->table->add_binary_diff(field, value_entry - original, len))
    return true; /* purecov: inspected */

  /*
    Finally, update the element count.
  */
  write_offset_or_size(destination + (m_data - original), m_element_count - 1,
                       m_large);
  return field->table->add_binary_diff(field, m_data - original,
                                       offset_size(m_large));
}

/**
  Get the amount of unused space in the binary representation of this value.

  @param      thd    THD handle
  @param[out] space  the amount of free space
  @return false on success, true on error
*/
bool Value::get_free_space(const THD *thd, size_t *space) const {
  *space = 0;

  switch (m_type) {
    case ARRAY:
    case OBJECT:
      break;
    default:
      // Scalars don't have any holes, so return immediately.
      return false;
  }

  if (m_type == OBJECT) {
    // The first key should come right after the last value entry.
    const char *next_key = m_data + value_entry_offset(m_element_count);

    // Sum up all unused space between keys.
    for (size_t i = 0; i < m_element_count; ++i) {
      Value key = this->key(i);
      if (key.type() == ERROR) {
        my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
        return true;
      }
      *space += key.get_data() - next_key;
      next_key = key.get_data() + key.get_data_length();
    }
  }

  size_t next_value_offset;
  if (first_value_offset(&next_value_offset)) {
    my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
    return true;
  }

  // Find the "holes" between and inside each element in the array or object.
  for (size_t i = 0; i < m_element_count; ++i) {
    size_t elt_start;
    size_t elt_end;
    bool inlined;
    if (element_offsets(i, &elt_start, &elt_end, &inlined)) {
      my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
      return true;
    }

    if (inlined) continue;

    if (elt_start < next_value_offset || elt_end > m_length) {
      my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
      return true;
    }

    *space += elt_start - next_value_offset;
    next_value_offset = elt_end;

    Value elt = element(i);
    switch (elt.type()) {
      case ARRAY:
      case OBJECT: {
        // Recursively process nested arrays or objects.
        if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr))
          return true; /* purecov: inspected */
        size_t elt_space;
        if (elt.get_free_space(thd, &elt_space)) return true;
        *space += elt_space;
        break;
      }
      case ERROR:
        /* purecov: begin inspected */
        my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
        return true;
        /* purecov: end */
      default:
        break;
    }
  }

  *space += m_length - next_value_offset;
  return false;
}

/**
  Check whether two binary JSON scalars are equal. This function is used by
  multi-valued index updating code. Unlike JSON comparator implemented in
  server, this code doesn't treat numeric types as the same, e.g. int 1 and
  uint 1 won't be treated as equal. This is fine as the mv index updating code
  compares old and new values of the same typed array field, i.e. all values
  being compared have the same type.

  Since MV index doesn't support indexing of arrays/objects in arrays, these
  two aren't supported and cause assert.
*/

int Value::eq(const Value &val) const {
  DBUG_ASSERT(is_valid() && val.is_valid());

  if (type() != val.type()) {
    return type() < val.type() ? -1 : 1;
  }
  switch (m_type) {
    case OBJECT:
    case ARRAY:
      DBUG_ASSERT(0);
      return -1;
    case OPAQUE:
      if (m_field_type != val.m_field_type)
        return m_field_type < val.m_field_type ? -1 : 1;
      /* Fall through */
    case STRING: {
      uint cmp_length = std::min(get_data_length(), val.get_data_length());
      int res;
      if (!(res = memcmp(get_data(), val.get_data(), cmp_length)))
        return (get_data_length() < val.get_data_length())
                   ? -1
                   : ((get_data_length() == val.get_data_length()) ? 0 : 1);
      return res;
    }
    case INT:
    case UINT:
      return (m_int_value == val.m_int_value)
                 ? 0
                 : ((m_int_value < val.m_int_value) ? -1 : 1);
    case DOUBLE:
      return (m_double_value == val.m_double_value)
                 ? 0
                 : ((m_double_value < val.m_double_value) ? -1 : 1);
    case LITERAL_NULL:
    case LITERAL_TRUE:
    case LITERAL_FALSE:
      return 0;
    default:
      DBUG_ASSERT(0);  // Shouldn't happen
      break;
  }
  return -1;
}
#endif  // ifdef MYSQL_SERVER

}  // end namespace json_binary
