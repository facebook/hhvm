#ifndef JSON_DIFF_INCLUDED
#define JSON_DIFF_INCLUDED

/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file

  Header file for the Json_diff class.

  The Json_diff class is used to represent a logical change in a JSON column,
  so that a replication master can send only what has changed, instead of
  sending the whole new value to the replication slave when a JSON column is
  updated.
*/

#include <stddef.h>
#include <algorithm>
#include <memory>  // std::unique_ptr
#include <vector>

#include "sql/json_path.h"
#include "sql/mem_root_allocator.h"

class Field_json;
class Json_dom;
class Json_wrapper;
class String;

/// Enum that describes what kind of operation a Json_diff object represents.
enum class enum_json_diff_operation {
  /**
    The JSON value in the given path is replaced with a new value.
    It has the same effect as `JSON_REPLACE(col, path, value)`.
  */
  REPLACE,

  /**
    Add a new element at the given path.

    If the path specifies an array element, it has the same effect as
    `JSON_ARRAY_INSERT(col, path, value)`.

    If the path specifies an object member, it has the same effect as
    `JSON_INSERT(col, path, value)`.
  */
  INSERT,

  /**
    The JSON value at the given path is removed from an array or object.
    It has the same effect as `JSON_REMOVE(col, path)`.
  */
  REMOVE,
};
/// The number of elements of the enumeration above.
static const int JSON_DIFF_OPERATION_COUNT = 3;

/**
  A class that represents a logical change to a JSON document. It is used by
  row-based replication to send information about changes in JSON documents
  without sending the whole updated document.
*/
class Json_diff final {
  /// The path that is changed.
  Json_path m_path;
  /// The operation to perform on the changed path.
  enum_json_diff_operation m_operation;
  /// The new value to add to the changed path.
  std::unique_ptr<Json_dom> m_value;

  /// The length of the operation when encoded in binary format.
  static const size_t ENCODED_OPERATION_BYTES = 1;

 public:
  /**
    Construct a Json_diff object.

    @param path       the path that is changed
    @param operation  the operation to perform on the path
    @param value      the new value in the path (the Json_diff object
                      takes over the ownership of the value)
  */
  Json_diff(const Json_seekable_path &path, enum_json_diff_operation operation,
            std::unique_ptr<Json_dom> value)
      : m_path(), m_operation(operation), m_value(std::move(value)) {
    for (const Json_path_leg *leg : path) m_path.append(*leg);
  }

  /// Get the path that is changed by this diff.
  const Json_path &path() const { return m_path; }

  /// Get the operation that is performed on the path.
  enum_json_diff_operation operation() const { return m_operation; }

  /**
    Get a Json_wrapper representing the new value to add to the path. The
    wrapper is an alias, so the ownership of the contained Json_dom is retained
    by the Json_diff object.
    @see Json_wrapper::set_alias()
  */
  Json_wrapper value() const;

  size_t binary_length() const;
  /**
    Serialize this Json_diff object and append to the given string

    @param to The String to append to
    @retval false Success
    @retval true Failure, meaning out of memory
  */
  bool write_binary(String *to) const;
};

/**
  Vector of logical diffs describing changes to a JSON column.
*/
class Json_diff_vector {
 public:
  /// Type of the allocator for the underlying invector.
  typedef Mem_root_allocator<Json_diff> allocator_type;
  /// Type of the underlying vector
  typedef std::vector<Json_diff, allocator_type> vector;
  /// Type of iterator over the underlying vector
  typedef vector::iterator iterator;
  /// Type of iterator over the underlying vector
  typedef vector::const_iterator const_iterator;
  /**
    Constructor
    @param arg Mem_root_allocator to use for the vector
  */
  Json_diff_vector(allocator_type arg);
  /**
    Append a new diff at the end of this vector.
    @param path Path to update
    @param operation Operation
    @param dom New value to insert
  */
  void add_diff(const Json_seekable_path &path,
                enum_json_diff_operation operation,
                std::unique_ptr<Json_dom> dom);
  /**
    Append a new diff at the end of this vector when operation == REMOVE.
    @param path Path to update
    @param operation Operation
  */
  void add_diff(const Json_seekable_path &path,
                enum_json_diff_operation operation);
  /// Clear the vector.
  void clear();
  /// Return the number of elements in the vector.
  inline size_t size() const { return m_vector.size(); }

  /**
    Return the element at the given position
    @param pos Position
    @return the pos'th element
  */
  inline Json_diff &at(size_t pos) { return m_vector.at(pos); }

  // Return forward iterator to the beginning
  inline const_iterator begin() const { return m_vector.begin(); }

  // Return forward iterator to the end
  const_iterator end() const { return m_vector.end(); }

  /**
    Return the length of the binary representation of this
    Json_diff_vector.

    The binary format has this form:

        +--------+--------+--------+     +--------+
        | length | diff_1 | diff_2 | ... | diff_N |
        +--------+--------+--------+     +--------+

    This function returns the length of only the diffs, if
    include_metadata==false.  It returns the length of the 'length'
    field plus the length of the diffs, if include_metadata=true.  The
    value of the 'length' field is exactly the return value from this
    function when include_metadata=false.

    @param include_metadata if true, include the length of the length
    field in the computation, otherwise don't.

    @return The computed length
  */
  size_t binary_length(bool include_metadata = true) const;

  /**
    Serialize this Json_diff_vector into the given String.

    @param to String to which the vector will be appended

    @retval false Success
    @retval true Failure (out of memory)
  */
  bool write_binary(String *to) const;

  /**
    De-serialize Json_diff objects from the given String into this
    Json_diff_vector.

    @param[in,out] from Pointer to buffer to read from. The function
    will move this to point to the next byte to read after those that
    were read.

    @param[in] table Table structure (used for error messages).

    @param[in] field_name Field name (used for error messages).

    @retval false Success
    @retval true Failure (bad format or out of memory)
  */
  bool read_binary(const char **from, const struct TABLE *table,
                   const char *field_name);

  /// An empty diff vector (having no diffs).
  static const Json_diff_vector EMPTY_JSON_DIFF_VECTOR;

 private:
  // The underlying vector
  vector m_vector;

  /// Length in bytes of the binary representation, not counting the 4 bytes
  /// length
  size_t m_binary_length;

  /// The length of the field where the total length is encoded.
  static const size_t ENCODED_LENGTH_BYTES = 4;
};

/**
  The result of applying JSON diffs on a JSON value using apply_json_diffs().
*/
enum class enum_json_diff_status {
  /**
     The JSON diffs were applied and the JSON value in the column was updated
     successfully.
  */
  SUCCESS,

  /**
    An error was raised while applying one of the diffs. The value in the
    column was not updated.
  */
  ERROR,

  /**
    One of the diffs was rejected. This could happen if the path specified in
    the diff does not exist in the JSON value, or if the diff is supposed to
    add a new value at a given path, but there already is a value at the path.

    This return code would usually indicate that the replication slave where
    the diff is applied, is out of sync with the replication master where the
    diff was created.

    The value in the column was not updated, but no error was raised.
  */
  REJECTED,
};

/**
  Apply a sequence of JSON diffs to the value stored in a JSON column.

  @param field  the column to update
  @param diffs  the diffs to apply
  @return an enum_json_diff_status value that tells if the diffs were
          applied successfully
*/
enum_json_diff_status apply_json_diffs(Field_json *field,
                                       const Json_diff_vector *diffs);

#endif /* JSON_DIFF_INCLUDED */
