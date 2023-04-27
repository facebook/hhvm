#ifndef JSON_DOM_INCLUDED
#define JSON_DOM_INCLUDED

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

#include <stddef.h>
#include <iterator>
#include <map>
#include <memory>  // unique_ptr
#include <new>
#include <string>
#include <type_traits>  // is_base_of
#include <utility>
#include <vector>

#include "field_types.h"  // enum_field_types
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/mysql_lex_string.h"
#include "mysql_time.h"            // MYSQL_TIME
#include "prealloced_array.h"      // Prealloced_array
#include "sql/json_binary.h"       // json_binary::Value
#include "sql/malloc_allocator.h"  // Malloc_allocator
#include "sql/my_decimal.h"        // my_decimal

class Field_json;
class Json_array;
class Json_container;
class Json_dom;
class Json_object;
class Json_path;
class Json_seekable_path;
class Json_wrapper;
class String;
class THD;

typedef Prealloced_array<Json_wrapper, 16> Json_wrapper_vector;
typedef Prealloced_array<Json_dom *, 16> Json_dom_vector;

using Json_dom_ptr = std::unique_ptr<Json_dom>;
using Json_array_ptr = std::unique_ptr<Json_array>;
using Json_object_ptr = std::unique_ptr<Json_object>;

/**
  @file sql/json_dom.h
  JSON DOM.

  When a JSON value is retrieved from a column, a prior it exists in
  a binary form, cf. Json_binary::Value class.

  However, when we need to manipulate the JSON values we mostly convert them
  from binary form to a structured in-memory from called DOM (from domain
  object model) which uses a recursive tree representation of the JSON value
  corresponding closely to a parse tree. This form is more suitable for
  manipulation.

  The JSON type is mostly represented internally as a Json_wrapper which hides
  if the representation is a binary or DOM one. This makes is possible to avoid
  building a DOM unless we really need one.

  The file defines two sets of classes: a) The Json_dom hierarchy and
  b) Json_wrapper and its companion classes Json_wrapper_object_iterator and
  Json_object_wrapper. For both sets, arrays are traversed using an operator[].
*/

/**
  Json values in MySQL comprises the stand set of JSON values plus a
  MySQL specific set. A Json _number_ type is subdivided into _int_,
  _uint_, _double_ and _decimal_.

  MySQL also adds four built-in date/time values: _date_, _time_,
  _datetime_ and _timestamp_.  An additional _opaque_ value can
  store any other MySQL type.

  The enumeration is common to Json_dom and Json_wrapper.

  The enumeration is also used by Json_wrapper::compare() to
  determine the ordering when comparing values of different types,
  so the order in which the values are defined in the enumeration,
  is significant. The expected order is null < number < string <
  object < array < boolean < date < time < datetime/timestamp <
  opaque.
*/
enum class enum_json_type {
  J_NULL,
  J_DECIMAL,
  J_INT,
  J_UINT,
  J_DOUBLE,
  J_STRING,
  J_OBJECT,
  J_ARRAY,
  J_BOOLEAN,
  J_DATE,
  J_TIME,
  J_DATETIME,
  J_TIMESTAMP,
  J_OPAQUE,
  J_ERROR
};

/**
  Allocate a new Json_dom object and return a std::unique_ptr which points to
  it.

  @param args  the arguments to pass to the constructor

  @tparam T     the type of Json_dom to create
  @tparam Args  the type of the arguments to pass to the constructor

  @return a pointer to the allocated object
*/
template <typename T, typename... Args>
inline std::unique_ptr<T> create_dom_ptr(Args &&... args) {
  return std::unique_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
}

/**
  JSON DOM abstract base class.

  MySQL representation of in-memory JSON objects used by the JSON type
  Supports access, deep cloning, and updates. See also Json_wrapper and
  json_binary::Value.
  Uses heap for space allocation for now. FIXME.

  Class hierarchy:
  <code><pre>
      Json_dom (abstract)
       Json_scalar (abstract)
         Json_string
         Json_number (abstract)
           Json_decimal
           Json_int
           Json_uint
           Json_double
         Json_boolean
         Json_null
         Json_datetime
         Json_opaque
       Json_container (abstract)
         Json_object
         Json_array
  </pre></code>
  At the outset, object and array add/insert/append operations takes
  a clone unless specified in the method, e.g. add_alias hands the
  responsibility for the passed in object over to the object.
*/
class Json_dom {
  // so that these classes can call set_parent()
  friend class Json_object;
  friend class Json_array;

 private:
  /**
    Set the parent dom to which this dom is attached.

    @param[in] parent the parent we're being attached to
  */
  void set_parent(Json_container *parent) { m_parent = parent; }

 public:
  virtual ~Json_dom() {}

  /**
    Allocate space on the heap for a Json_dom object.

    @return pointer to the allocated memory, or NULL if memory could
    not be allocated (in which case my_error() will have been called
    with the appropriate error message)
  */
  void *operator new(size_t size, const std::nothrow_t &) noexcept;

  /**
    Deallocate the space used by a Json_dom object.
  */
  void operator delete(void *ptr) noexcept;

  /**
    Nothrow delete.
  */
  void operator delete(void *ptr, const std::nothrow_t &)noexcept;

  /**
    Get the parent dom to which this dom is attached.

    @return the parent dom.
  */
  Json_container *parent() const { return m_parent; }

  /**
    @return the type corresponding to the actual Json_dom subclass
  */
  virtual enum_json_type json_type() const = 0;

  /**
    @return true if the object is a subclass of Json_scalar
  */
  virtual bool is_scalar() const { return false; }

  /**
    @return true of the object is a subclass of Json_number
  */
  virtual bool is_number() const { return false; }

#ifdef MYSQL_SERVER
  /**
    Compute the depth of a document. This is the value which would be
    returned by the JSON_DEPTH() system function.

    - for scalar values, empty array and empty object: 1
    - for non-empty array: 1+ max(depth of array elements)
    - for non-empty objects: 1+ max(depth of object values)

    For example:
    "abc", [] and {} have depth 1.
    ["abc", [3]] and {"a": "abc", "b": [3]} have depth 3.

    @return the depth of the document
  */
  virtual uint32 depth() const = 0;
#endif

  /**
    Make a deep clone. The ownership of the returned object is
    henceforth with the caller.

    @return a cloned Json_dom object.
  */
  virtual Json_dom_ptr clone() const = 0;

  /**
    Parse Json text to DOM (using rapidjson). The text must be valid JSON.
    The results when supplying an invalid document is undefined.
    The ownership of the returned object is henceforth with the caller.

    If the parsing fails because of a syntax error, the errmsg and
    offset arguments will be given values that point to a detailed
    error message and where the syntax error was located. The caller
    will have to generate an error message with my_error() in this
    case.

    If the parsing fails because of some other error (such as out of
    memory), errmsg will point to a location that holds the value
    NULL. In this case, parse() will already have called my_error(),
    and the caller doesn't need to generate an error message.

    @param[in]  text   the JSON text
    @param[in]  length the length of the text
    @param[in]  legacy_parsing  True if JSON is parsed in fb 5.6 json format.
    @param[out] errmsg any syntax error message (will be ignored if it is NULL)
    @param[out] offset the position in the parsed string a syntax error was
                       found (will be ignored if it is NULL)

    @result the built DOM if JSON text was parseable, else NULL
  */
  static Json_dom_ptr parse(const char *text, size_t length,
                            bool legacy_parsing, const char **errmsg,
                            size_t *offset);

  /**
    Construct a DOM object based on a binary JSON value. The ownership
    of the returned object is henceforth with the caller.

    @param thd  current session
    @param v    the binary value to parse
    @return a DOM representation of the binary value, or NULL on error
  */
  static Json_dom_ptr parse(const THD *thd, const json_binary::Value &v);

  /**
    Get the path location of this dom, measured from the outermost
    document it nests inside.
  */
  Json_path get_location();

  /**
    Finds all of the json sub-documents which match the path expression.
    Adds a vector element for each match.

    See the header comment for Json_wrapper.seek() for a discussion
    of complexities involving path expression with more than one
    ellipsis (**) token.

    @param[in]  path  the (possibly wildcarded) address of the sub-documents
    @param[in]  legs  the number of legs to use from @a path
    @param[out] hits  one element per match
    @param[in]  auto_wrap
                      if true, match a tailing [0] to scalar at that position.
    @param[in]  only_need_one True if we can stop after finding one match
    @return false on success, true on error
  */
  bool seek(const Json_seekable_path &path, size_t legs, Json_dom_vector *hits,
            bool auto_wrap, bool only_need_one);

 private:
  /** Parent pointer */
  Json_container *m_parent{nullptr};
};

/**
  Abstract base class of all JSON container types (Json_object and Json_array).
*/
class Json_container : public Json_dom {
 public:
#ifdef MYSQL_SERVER
  /**
    Replace oldv contained inside this container array or object) with newv. If
    this container does not contain oldv, calling the method is a no-op.

    @param[in] oldv the value to be replaced
    @param[in] newv the new value to put in the container
  */
  virtual void replace_dom_in_container(const Json_dom *oldv,
                                        Json_dom_ptr newv) = 0;
#endif  // ifdef MYSQL_SERVER
};

/**
  A comparator that is used for ordering keys in a Json_object. It
  orders the keys on length, and lexicographically if the keys have
  the same length. The ordering is ascending. This ordering was chosen
  for speed of look-up. See usage in Json_object_map.
*/
struct Json_key_comparator {
  bool operator()(const std::string &key1, const std::string &key2) const;
  bool operator()(const MYSQL_LEX_CSTRING &key1, const std::string &key2) const;
  bool operator()(const std::string &key1, const MYSQL_LEX_CSTRING &key2) const;
  // is_transparent must be defined in order to make std::map::find() accept
  // keys that are of a different type than the key_type of the map. In
  // particular, this is needed to make it possible to call find() with
  // MYSQL_LEX_CSTRING arguments and not only std::string arguments. It only has
  // to be defined, it doesn't matter which type it is set to.
  using is_transparent = void;
};

/**
  A type used to hold JSON object elements in a map, see the
  Json_object class.
*/
using Json_object_map =
    std::multimap<std::string, Json_dom_ptr, Json_key_comparator,
                  Malloc_allocator<std::pair<const std::string, Json_dom_ptr>>>;

/**
  Represents a JSON container value of type "object" (ECMA), type
  J_OBJECT here.
*/
class Json_object final : public Json_container {
 private:
  /**
    Map to hold the object elements.
  */
  Json_object_map m_map;

  /**
     Is object created by 5.6 json functions.
   */
  bool m_legacy_object;

 public:
  Json_object(bool m_legacy_object = false);
  enum_json_type json_type() const override { return enum_json_type::J_OBJECT; }

  /**
    Insert a clone of the value into the object. If the key already
    exists in the object, the existing value is replaced ("last value
    wins").
    Keeps duplicate values of the key in multimap if legacy object.

    @param[in]  key    the JSON element key of to be added
    @param[in]  value  a JSON value: the element key's value
    @retval false on success
    @retval true on failure
  */
  bool add_clone(const std::string &key, const Json_dom *value) {
    return value == nullptr || add_alias(key, value->clone());
  }

  /**
    Insert the value into the object. If the key already exists in the
    object, the existing value is replaced ("last value wins").
    Keeps duplicate values of the key in multimap if legacy object.

    Ownership of the value is effectively transferred to the
    object and the value will be deallocated by the object so only add
    values that can be deallocated safely (no stack variables please!)

    New code should prefer #add_alias(const std::string&, Json_dom_ptr)
    to this function, because that makes the transfer of ownership
    more explicit. This function might be removed in the future.

    @param[in]  key    the JSON key of to be added
    @param[in]  value  a JSON value: the key's value
    @retval false on success
    @retval true on failure
  */
  bool add_alias(const std::string &key, Json_dom *value) {
    return add_alias(key, Json_dom_ptr(value));
  }

  /**
    Insert the value into the object. If the key already exists in the
    object, the existing value is replaced ("last value wins").
    Keeps duplicate values of the key in multimap if legacy object.

    The ownership of the value is transferred to the object.

    @param[in] key    the key of the value to be added
    @param[in] value  the value to add
    @return false on success, true on failure
  */
  bool add_alias(const std::string &key, Json_dom_ptr value);

  /**
    Transfer all of the key/value pairs in the other object into this
    object. The other object is deleted. If this object and the other
    object share a key, then the two values of the key are merged.

    @param [in] other    a pointer to the object which will be consumed
    @retval false on success
    @retval true on failure
  */
  bool consume(Json_object_ptr other);

  /**
    Return the value at key. The value is not cloned, so make
    one if you need it. Do not delete the returned value, please!
    If the key is not present, return a null pointer.

    @param[in]  key the key of the element whose value we want
    @return the value associated with the key, or NULL if the key is not found
  */
  Json_dom *get(const std::string &key) const;
  Json_dom *get(const MYSQL_LEX_CSTRING &key) const;

  /**
    Remove the child element addressed by key. The removed child is deleted.

    @param key the key of the element to remove
    @retval true if an element was removed
    @retval false if there was no element with that key
  */
  bool remove(const std::string &key);

  /**
    @return The number of elements in the JSON object.
  */
  size_t cardinality() const;

#ifdef MYSQL_SERVER
  uint32 depth() const override;
#endif

  Json_dom_ptr clone() const override;

#ifdef MYSQL_SERVER
  void replace_dom_in_container(const Json_dom *oldv,
                                Json_dom_ptr newv) override;
#endif

  /**
    Remove all elements in the object.
  */
  void clear() { m_map.clear(); }

  /**
    Constant iterator over the elements in the JSON object. Each
    element is represented as a std::pair where first is a std::string
    that represents the key name, and second is a pointer to a
    Json_dom that represents the value.
  */
  typedef Json_object_map::const_iterator const_iterator;

  /// Returns a const_iterator that refers to the first element.
  const_iterator begin() const { return m_map.begin(); }

  /// Returns a const_iterator that refers past the last element.
  const_iterator end() const { return m_map.end(); }

  /**
    Implementation of the MergePatch function specified in RFC 7396:

        define MergePatch(Target, Patch):
          if Patch is an Object:
            if Target is not an Object:
              Target = {} # Ignore the contents and set it to an empty Object
            for each Key/Value pair in Patch:
              if Value is null:
                if Key exists in Target:
                  remove the Key/Value pair from Target
              else:
                Target[Key] = MergePatch(Target[Key], Value)
            return Target
          else:
            return Patch

    @param patch  the object that describes the patch
    @retval false on success
    @retval true on memory allocation error
  */
  bool merge_patch(Json_object_ptr patch);

  bool is_legacy_object() const { return m_legacy_object; }
};

/**
  Represents a JSON array container, i.e. type J_ARRAY here.
*/
class Json_array final : public Json_container {
 private:
  /// Holds the array values.
  std::vector<Json_dom_ptr, Malloc_allocator<Json_dom_ptr>> m_v;

 public:
  Json_array();

  enum_json_type json_type() const override { return enum_json_type::J_ARRAY; }

  /**
    Append a clone of the value to the end of the array.
    @param[in] value a JSON value to be appended
    @retval false on success
    @retval true on failure
  */
  bool append_clone(const Json_dom *value) {
    return insert_clone(size(), value);
  }

  /**
    Append the value to the end of the array.

    Ownership of the value is effectively transferred to the array and
    the value will be deallocated by the array so only append values
    that can be deallocated safely (no stack variables please!)

    New code should prefer #append_alias(Json_dom_ptr) to this
    function, because that makes the transfer of ownership more
    explicit. This function might be removed in the future.

    @param[in]  value a JSON value to be appended
    @retval false on success
    @retval true on failure
  */
  bool append_alias(Json_dom *value) {
    return append_alias(Json_dom_ptr(value));
  }

  /**
    Append the value to the end of the array and take over the
    ownership of the value.

    @param value  the JSON value to be appended
    @return false on success, true on failure
  */
  bool append_alias(Json_dom_ptr value) {
    return insert_alias(size(), std::move(value));
  }

  /**
    Moves all of the elements in the other array to the end of
    this array. The other array is deleted.

    @param [in] other     a pointer to the array which will be consumed
    @retval false on success
    @retval true on failure
  */
  bool consume(Json_array_ptr other);

  /**
    Insert a clone of the value at position index of the array. If beyond the
    end, insert at the end.

    @param[in]  index the position at which to insert
    @param[in]  value a JSON value to be inserted
    @retval false on success
    @retval true on failure
  */
  bool insert_clone(size_t index, const Json_dom *value) {
    return value == nullptr || insert_alias(index, value->clone());
  }

  /**
    Insert the value at position index of the array.
    If beyond the end, insert at the end.

    Ownership of the value is effectively transferred to the array and
    the value will be deallocated by the array so only append values
    that can be deallocated safely (no stack variables please!)

    @param[in]  index the position at which to insert
    @param[in]  value a JSON value to be inserted
    @retval false on success
    @retval true on failure
  */
  bool insert_alias(size_t index, Json_dom_ptr value);

  /**
    Remove the value at this index. A no-op if index is larger than
    size. Deletes the value.
    @param[in]  index  the index of the value to remove
    @return true if a value was removed, false otherwise.
  */
  bool remove(size_t index);

  /**
    The cardinality of the array (number of values).
    @return the size
  */
  size_t size() const { return m_v.size(); }

#ifdef MYSQL_SERVER
  uint32 depth() const override;
#endif

  Json_dom_ptr clone() const override;

  /**
    Get the value at position index. The value has not been cloned so
    it is the responsibility of the user to make a copy if needed.  Do
    not try to deallocate the returned value - it is owned by the array
    and will be deallocated by it in time.  It is admissible to modify
    its contents (in place; without a clone being taken) if it is a
    compound.

    @param[in] index  the array index
    @return the value at index
  */
  Json_dom *operator[](size_t index) const {
    DBUG_ASSERT(m_v[index]->parent() == this);
    return m_v[index].get();
  }

  /**
    Remove the values in the array.
  */
  void clear() { m_v.clear(); }

  /// Constant iterator over the elements in the JSON array.
  using const_iterator = decltype(m_v)::const_iterator;

  /// Returns a const_iterator that refers to the first element.
  const_iterator begin() const { return m_v.begin(); }

  /// Returns a const_iterator that refers past the last element.
  const_iterator end() const { return m_v.end(); }

#ifdef MYSQL_SERVER
  void replace_dom_in_container(const Json_dom *oldv,
                                Json_dom_ptr newv) override;
#endif

  /// Sort the array
  void sort(const CHARSET_INFO *cs = nullptr);
  /**
    Check if the given value appears in the array

    @param val  value to look for

    @returns
      true  value is found
      false otherwise
  */
  bool binary_search(Json_dom *val);

  /**
    Sort array and remove duplicate elements.
    Used by multi-value index implementation.
  */
  void remove_duplicates(const CHARSET_INFO *cs);

  friend Json_dom;
};

/**
  Abstract base class for all Json scalars.
*/
class Json_scalar : public Json_dom {
 public:
#ifdef MYSQL_SERVER
  uint32 depth() const final override { return 1; }
#endif

  bool is_scalar() const final override { return true; }
};

/**
  Represents a JSON string value (ECMA), of type J_STRING here.
*/
class Json_string final : public Json_scalar {
 private:
  std::string m_str;  //!< holds the string
 public:
  /*
    Construct a Json_string object.
    @param args any arguments accepted by std::string's constructors
  */
  template <typename... Args>
  explicit Json_string(Args &&... args)
      : Json_scalar(), m_str(std::forward<Args>(args)...) {}

  enum_json_type json_type() const override { return enum_json_type::J_STRING; }

  Json_dom_ptr clone() const override {
    return create_dom_ptr<Json_string>(m_str);
  }

  /**
    Get the reference to the value of the JSON string.
    @return the string reference
  */
  const std::string &value() const { return m_str; }

  /**
    Get the number of characters in the string.
    @return the number of characters
  */
  size_t size() const { return m_str.size(); }
};

/**
  Abstract base class of all JSON number (ECMA) types (subclasses
  represent MySQL extensions).
*/
class Json_number : public Json_scalar {
 public:
  bool is_number() const final override { return true; }
};

/**
  Represents a MySQL decimal number, type J_DECIMAL.
*/
class Json_decimal final : public Json_number {
 private:
  my_decimal m_dec;  //!< holds the decimal number

 public:
  static const int MAX_BINARY_SIZE = DECIMAL_MAX_FIELD_SIZE + 2;

  explicit Json_decimal(const my_decimal &value);

  /**
    Get the number of bytes needed to store this decimal in a Json_opaque.
    @return the number of bytes.
  */
  int binary_size() const;

  /**
    Get the binary representation of the wrapped my_decimal, so that this
    value can be stored inside of a Json_opaque.

    @param dest the destination buffer to which the binary representation
                is written
    @return false on success, true on error
  */
  bool get_binary(char *dest) const;

  enum_json_type json_type() const override {
    return enum_json_type::J_DECIMAL;
  }

  /**
    Get a pointer to the MySQL decimal held by this object. Ownership
    is _not_ transferred.
    @return the decimal
  */
  const my_decimal *value() const { return &m_dec; }

  Json_dom_ptr clone() const override {
    return create_dom_ptr<Json_decimal>(m_dec);
  }

  /**
    Convert a binary value produced by get_binary() back to a my_decimal.

    @details
      This and two next functions help storage engine to deal with
      decimal value in a serialized JSON document. This funciton converts
      serialized value to my_decimal. The later two functions extract the
      decimal value from serialized JSON, so SE can index it in multi-valued
      index.

    @param[in]   bin  decimal value in binary format
    @param[in]   len  length of the binary value
    @param[out]  dec  my_decimal object to store the value to
    @return  false on success, true on failure
  */
  static bool convert_from_binary(const char *bin, size_t len, my_decimal *dec);
  /**
    Returns stored DECIMAL binary

    @param  bin   serialized Json_decimal object

    @returns
      pointer to the binary decimal value

    @see #convert_from_binary
  */
  static const char *get_encoded_binary(const char *bin) {
    // Skip stored precision and scale
    return bin + 2;
  }
  /**
    Returns length of stored DECIMAL binary

    @param  length  length of serialized Json_decimal object

    @returns
      length of the binary decimal value

    @see #convert_from_binary
  */
  static size_t get_encoded_binary_len(size_t length) {
    // Skip stored precision and scale
    return length - 2;
  }
};

/**
  Represents a MySQL double JSON scalar (an extension of the ECMA
  number value), type J_DOUBLE.
*/
class Json_double final : public Json_number {
 private:
  double m_f;  //!< holds the double value
 public:
  explicit Json_double(double value) : Json_number(), m_f(value) {}

  enum_json_type json_type() const override { return enum_json_type::J_DOUBLE; }

  Json_dom_ptr clone() const override {
    return create_dom_ptr<Json_double>(m_f);
  }

  /**
    Return the double value held by this object.
    @return the value
  */
  double value() const { return m_f; }
};

/**
  Represents a MySQL integer (64 bits signed) JSON scalar (an extension
  of the ECMA number value), type J_INT.
*/
class Json_int final : public Json_number {
 private:
  longlong m_i;  //!< holds the value
 public:
  explicit Json_int(longlong value) : Json_number(), m_i(value) {}

  enum_json_type json_type() const override { return enum_json_type::J_INT; }

  /**
    Return the signed int held by this object.
    @return the value
  */
  longlong value() const { return m_i; }

  /**
    @return true if the number can be held by a 16 bit signed integer
  */
  bool is_16bit() const { return INT_MIN16 <= m_i && m_i <= INT_MAX16; }

  /**
    @return true if the number can be held by a 32 bit signed integer
  */
  bool is_32bit() const { return INT_MIN32 <= m_i && m_i <= INT_MAX32; }

  Json_dom_ptr clone() const override { return create_dom_ptr<Json_int>(m_i); }
};

/**
  Represents a MySQL integer (64 bits unsigned) JSON scalar (an extension
  of the ECMA number value), type J_UINT.
*/

class Json_uint final : public Json_number {
 private:
  ulonglong m_i;  //!< holds the value
 public:
  explicit Json_uint(ulonglong value) : Json_number(), m_i(value) {}

  enum_json_type json_type() const override { return enum_json_type::J_UINT; }

  /**
    Return the unsigned int held by this object.
    @return the value
  */
  ulonglong value() const { return m_i; }

  /**
    @return true if the number can be held by a 16 bit unsigned
    integer.
  */
  bool is_16bit() const { return m_i <= UINT_MAX16; }

  /**
    @return true if the number can be held by a 32 bit unsigned
    integer.
  */
  bool is_32bit() const { return m_i <= UINT_MAX32; }

  Json_dom_ptr clone() const override { return create_dom_ptr<Json_uint>(m_i); }
};

/**
  Represents a JSON null type (ECMA), type J_NULL here.
*/
class Json_null final : public Json_scalar {
 public:
  enum_json_type json_type() const override { return enum_json_type::J_NULL; }
  Json_dom_ptr clone() const override { return create_dom_ptr<Json_null>(); }
};

/**
  Represents a MySQL date/time value (DATE, TIME, DATETIME or
  TIMESTAMP) - an extension to the ECMA set of JSON scalar types, types
  J_DATE, J_TIME, J_DATETIME and J_TIMESTAMP respectively. The method
  field_type identifies which of the four it is.
*/
class Json_datetime final : public Json_scalar {
 private:
  MYSQL_TIME m_t;                 //!< holds the date/time value
  enum_field_types m_field_type;  //!< identifies which type of date/time

 public:
  /**
    Constructs a object to hold a MySQL date/time value.

    @param[in] t   the time/value
    @param[in] ft  the field type: must be one of MYSQL_TYPE_TIME,
                   MYSQL_TYPE_DATE, MYSQL_TYPE_DATETIME or
                   MYSQL_TYPE_TIMESTAMP.
  */
  Json_datetime(const MYSQL_TIME &t, enum_field_types ft)
      : Json_scalar(), m_t(t), m_field_type(ft) {}

  enum_json_type json_type() const override;

  Json_dom_ptr clone() const override;

  /**
    Return a pointer the date/time value. Ownership is _not_ transferred.
    To identify which time time the value represents, use @c field_type.
    @return the pointer
  */
  const MYSQL_TIME *value() const { return &m_t; }

  /**
    Return what kind of date/time value this object holds.
    @return One of MYSQL_TYPE_TIME, MYSQL_TYPE_DATE, MYSQL_TYPE_DATETIME
            or MYSQL_TYPE_TIMESTAMP.
  */
  enum_field_types field_type() const { return m_field_type; }

  /**
    Convert the datetime to the packed format used when storing
    datetime values.
    @param dest the destination buffer to write the packed datetime to
    (must at least have size PACKED_SIZE)
  */
  void to_packed(char *dest) const;

  /**
    Convert a packed datetime back to a MYSQL_TIME.
    @param from the buffer to read from (must have at least PACKED_SIZE bytes)
    @param ft   the field type of the value
    @param to   the MYSQL_TIME to write the value to
  */
  static void from_packed(const char *from, enum_field_types ft,
                          MYSQL_TIME *to);

#ifdef MYSQL_SERVER
  /**
    Convert a packed datetime to key string for indexing by SE
    @param from the buffer to read from
    @param ft   the field type of the value
    @param to   the destination buffer
    @param dec  value's decimals
  */
  static void from_packed_to_key(const char *from, enum_field_types ft,
                                 uchar *to, uint8 dec);
#endif

  /** Datetimes are packed in eight bytes. */
  static const size_t PACKED_SIZE = 8;
};

/**
  Represents a MySQL value opaquely, i.e. the Json DOM can not
  serialize or deserialize these values.  This should be used to store
  values that don't map to the other Json_scalar classes.  Using the
  "to_string" method on such values (via Json_wrapper) will yield a base
  64 encoded string tagged with the MySQL type with this syntax:

  "base64:typeXX:<base 64 encoded value>"
*/
class Json_opaque final : public Json_scalar {
 private:
  enum_field_types m_mytype;
  std::string m_val;

 public:
  /**
    An opaque MySQL value.

    @param[in] mytype  the MySQL type of the value
    @param[in] args    arguments to construct the binary value to be stored
                       in the DOM (anything accepted by the std::string
                       constructors)
    @see #enum_field_types
    @see Class documentation
  */
  template <typename... Args>
  explicit Json_opaque(enum_field_types mytype, Args &&... args)
      : Json_scalar(), m_mytype(mytype), m_val(std::forward<Args>(args)...) {}

  enum_json_type json_type() const override { return enum_json_type::J_OPAQUE; }

  /**
    @return a pointer to the opaque value. Use #size() to get its size.
  */
  const char *value() const { return m_val.data(); }

  /**
    @return the MySQL type of the value
  */
  enum_field_types type() const { return m_mytype; }
  /**
    @return the size in bytes of the value
  */
  size_t size() const { return m_val.size(); }

  Json_dom_ptr clone() const override;
};

/**
  Represents a JSON true or false value, type J_BOOLEAN here.
*/
class Json_boolean final : public Json_scalar {
 private:
  bool m_v;  //!< false or true: represents the eponymous JSON literal
 public:
  explicit Json_boolean(bool value) : Json_scalar(), m_v(value) {}

  enum_json_type json_type() const override {
    return enum_json_type::J_BOOLEAN;
  }

  /**
    @return false for JSON false, true for JSON true
  */
  bool value() const { return m_v; }

  Json_dom_ptr clone() const override {
    return create_dom_ptr<Json_boolean>(m_v);
  }
};

/**
  Function for double-quoting a string and escaping characters
  to make up a valid EMCA Json text.

  @param[in]     cptr    the unquoted character string
  @param[in]     length  its length
  @param[in,out] buf     the destination buffer

  @return false on success, true on error
*/
bool double_quote(const char *cptr, size_t length, String *buf);

/**
 Merge two doms. The right dom is either subsumed into the left dom
 or the contents of the right dom are transferred to the left dom
 and the right dom is deleted. After calling this function, the
 caller should not reference the right dom again. It has been
 deleted.

 Returns NULL if there is a memory allocation failure. In this case
 both doms are deleted.

 scalars - If any of the documents that are being merged is a scalar,
 each scalar document is autowrapped as a single value array before merging.

 arrays - When merging a left array with a right array,
 then the result is the left array concatenated
 with the right array. For instance, [ 1, 2 ] merged with [ 3, 4 ]
 is [ 1, 2, 3, 4 ].

 array and object - When merging an array with an object,
 the object is autowrapped as an array and then the rule above
 is applied. So [ 1, 2 ] merged with { "a" : true }
 is [ 1, 2, { "a": true } ].

 objects - When merging two objects, the two objects are concatenated
 into a single, larger object. So { "a" : "foo" } merged with { "b" : 5 }
 is { "a" : "foo", "b" : 5 }.

 duplicates - When two objects are merged and they share a key,
 the values associated with the shared key are merged.

 @param [in,out] left  The recipient dom.
 @param [in,out] right  The dom to be consumed

 @return A composite dom which subsumes the left and right doms, or NULL
 if a failure happened while merging
*/
Json_dom_ptr merge_doms(Json_dom_ptr left, Json_dom_ptr right);

/**
  How Json_wrapper would handle coercion error
*/

enum enum_coercion_error {
  CE_WARNING,  // Throw a warning, default
  CE_ERROR     // Throw an error
};

/**
   Control legacy json print behavior by Json_wrapper.
 */
enum legacy_json_print_behavior {
  LEGACY_JSON_DISABLED,
  LEGACY_JSON_EXTRACT,
  LEGACY_JSON_EXTRACT_VALUE
};

/**
  Abstraction for accessing JSON values irrespective of whether they
  are (started out as) binary JSON values or JSON DOM values. The
  purpose of this is to allow uniform access for callers. It allows us
  to access binary JSON values without necessarily building a DOM (and
  thus having to read the entire value unless necessary, e.g. for
  accessing only a single array slot or object field).

  Instances of this class are usually created on the stack. In some
  cases instances are cached in an Item and reused, in which case they
  are allocated from query-duration memory (by allocating them on a
  MEM_ROOT).
*/
class Json_wrapper {
 private:
  /*
    A Json_wrapper wraps either a Json_dom or a json_binary::Value,
    never both at the same time.
  */
  union {
    /// The DOM representation, only used if m_is_dom is true.
    struct {
      Json_dom *m_dom_value;
      /// If true, don't deallocate m_dom_value in destructor.
      bool m_dom_alias;
    };
    /// The binary representation, only used if m_is_dom is false.
    json_binary::Value m_value;
  };
  bool m_is_dom;  //!< Wraps a DOM iff true

  legacy_json_print_behavior m_legacy_json;  //!< Emulates 5.6 fb json functions

  /**
    Is json object created by legacy implementation of json_extract

    @return true if object created by legacy json_extract.
   */
  bool legacy_json_extract() const {
    return m_legacy_json == LEGACY_JSON_EXTRACT;
  }

  /**
     Should legacy json_extract comparion logic be used.

     @param[in] other Other json object used for comparison.
     @return true if legacy json_extract comparison function needs to be called.
   */
  bool use_legacy_json_extract_comparison(const Json_wrapper &other) const;

  /**
     Compare this JSON value to another JSON value using legacy json_extract
     semantics.

     @param[in] other the other JSON value
     @param[in] cs    if given, this charset will be used in comparison of
                     string values
     @retval -1 if this JSON value is less than the other JSON value
     @retval 0 if the two JSON values are equal
     @retval 1 if this JSON value is greater than the other JSON value
   */
  int legacy_json_extract_compare(const Json_wrapper &other,
                                  const CHARSET_INFO *cs) const;

 public:
  /**
    Get the wrapped datetime value in the packed format.

    @param[in,out] buffer a char buffer with space for at least
    Json_datetime::PACKED_SIZE characters
    @return a char buffer that contains the packed representation of the
    datetime (may or may not be the same as buffer)
  */
  const char *get_datetime_packed(char *buffer) const;

  /**
    Create an empty wrapper. Cf #empty().
  */
  Json_wrapper()
      : m_dom_value(nullptr),
        m_is_dom(true),
        m_legacy_json(LEGACY_JSON_DISABLED) {
    // Workaround for Solaris Studio, initialize in CTOR body.
    m_dom_alias = true;
  }

  /**
    Wrap the supplied DOM value (no copy taken). The wrapper takes
    ownership, unless alias is true or @c set_alias is called after
    construction.
    In the latter case the lifetime of the DOM is determined by
    the owner of the DOM, so clients need to ensure that that
    lifetime is sufficient, lest dead storage is attempted accessed.

    @param[in,out] dom_value  the DOM value
    @param         alias      Whether the wrapper is an alias to DOM
  */
  explicit Json_wrapper(Json_dom *dom_value, bool alias = false);

  /**
    Wrap the supplied DOM value. The wrapper takes over the ownership.
  */
  explicit Json_wrapper(Json_dom_ptr dom_value)
      : Json_wrapper(dom_value.release()) {}

  /**
    Only meaningful iff the wrapper encapsulates a DOM. Marks the
    wrapper as not owning the DOM object, i.e. it will not be
    deallocated in the wrapper's destructor. Useful if one wants a wrapper
    around a DOM owned by someone else.
  */
  void set_alias() { m_dom_alias = true; }

  /**
    Wrap a binary value. Does not copy the underlying buffer, so
    lifetime is limited the that of the supplied value.

    @param[in] value  the binary value
  */
  explicit Json_wrapper(const json_binary::Value &value);

  /**
    Copy constructor. Does a deep copy of any owned DOM. If a DOM
    os not owned (aliased), the copy will also be aliased.
  */
  Json_wrapper(const Json_wrapper &old);

  /**
    Move constructor. Take over the ownership of the other wrapper's
    DOM, unless it's aliased. If the other wrapper is aliased, this
    wrapper becomes an alias too. Any already owned DOM will be
    deallocated.

    @param old the wrapper whose contents to take over
  */
  Json_wrapper(Json_wrapper &&old) noexcept;

  /**
    Assignment operator. Does a deep copy of any owned DOM. If a DOM
    os not owned (aliased), the copy will also be aliased. Any owned
    DOM in the left side will be deallocated.
  */
  Json_wrapper &operator=(const Json_wrapper &old);

  /**
    Move-assignment operator. Take over the ownership of the other
    wrapper's DOM, unless it's aliased. If the other wrapper is
    aliased, this wrapper becomes an alias too. Any already owned DOM
    will be deallocated.

    @param old the wrapper whose contents to take over
  */
  Json_wrapper &operator=(Json_wrapper &&old) noexcept;

  ~Json_wrapper();

  /**
    A Wrapper is defined to be empty if it is passed a NULL value with the
    constructor for JSON dom, or if the default constructor is used.

    @return true if the wrapper is empty.
  */
  bool empty() const { return m_is_dom && !m_dom_value; }

  /**
    Does this wrapper contain a DOM?

    @retval true   if the wrapper contains a DOM representation
    @retval false  if the wrapper contains a binary representation
  */
  bool is_dom() const { return m_is_dom; }

  /**
    Get the wrapped contents in DOM form. The DOM is (still) owned by the
    wrapper. If this wrapper originally held a value, it is now converted
    to hold (and eventually release) the DOM version.

    @param thd current session (can be nullptr if is_dom() returns true)
    @return pointer to a DOM object, or NULL if the DOM could not be allocated
  */
  Json_dom *to_dom(const THD *thd);

  /**
    Gets a pointer to the wrapped Json_dom object, if this wrapper holds a DOM.
    If is_dom() returns false, the result of calling this function is undefined.
  */
  const Json_dom *get_dom() const {
    DBUG_ASSERT(m_is_dom);
    return m_dom_value;
  }

  /**
    Gets the wrapped json_binary::Value object, if this wrapper holds a binary
    JSON value. If is_dom() returns true, the result of calling this function is
    undefined.
  */
  const json_binary::Value &get_binary_value() const {
    DBUG_ASSERT(!m_is_dom);
    return m_value;
  }

#ifdef MYSQL_SERVER
  /**
    Get the wrapped contents in DOM form. Same as to_dom(), except it returns
    a clone of the original DOM instead of the actual, internal DOM tree.

    @param thd current session
    @return pointer to a DOM object, or NULL if the DOM could not be allocated
  */
  Json_dom_ptr clone_dom(const THD *thd) const;

  /**
    Get the wrapped contents in binary value form.

    @param[in]     thd  current session
    @param[in,out] str  a string that will be filled with the binary value
    @retval false on success
    @retval true  on error
  */
  bool to_binary(const THD *thd, String *str) const;
#endif

  /**
    Check if the wrapped JSON document is a binary value (a
    json_binary::Value), and if that binary is pointing to data stored in the
    given string.

    This function can be used to check if overwriting the data in the string
    might overwrite and corrupt the document contained in this wrapper.

    @param str    a string which contains JSON binary data
    @retval true  if the string contains data that the wrapped document
                  points to from its json_binary::Value representation
    @retval false otherwise
  */
  bool is_binary_backed_by(const String *str) const {
    return !m_is_dom && m_value.is_backed_by(str);
  }

  /**
    Format the JSON value to an external JSON string in buffer in
    the format of ISO/IEC 10646.

    @param[in,out] buffer      the formatted string is appended, so make sure
                               the length is set correctly before calling
    @param[in]     json_quoted if the JSON value is a string and json_quoted
                               is false, don't perform quoting on the string.
                               This is only used by JSON_UNQUOTE.
    @param[in]     func_name   The name of the function that called to_string().

    @return false formatting went well, else true
  */
  bool to_string(String *buffer, bool json_quoted, const char *func_name) const;

  /**
    Print this JSON document to the debug trace.

    @param[in] message If given, the JSON document is prefixed with
    this message.
  */
  void dbug_print(const char *message MY_ATTRIBUTE((unused)) = "") const;

  /**
    Format the JSON value to an external JSON string in buffer in the format of
    ISO/IEC 10646. Add newlines and indentation for readability.

    @param[in,out] buffer     the buffer that receives the formatted string
                              (the string is appended, so make sure the length
                              is set correctly before calling)
    @param[in]     func_name  the name of the calling function

    @retval false on success
    @retval true on error
  */
  bool to_pretty_string(String *buffer, const char *func_name) const;

  // Accessors

  /**
    Return the type of the wrapped JSON value

    @return the type, or Json_dom::J_ERROR if the wrapper does not contain
    a JSON value
  */
  enum_json_type type() const;

  /**
    Return the MYSQL type of the opaque value, see #type(). Valid for
    J_OPAQUE.  Calling this method if the type is not J_OPAQUE will give
    undefined results.

    @return the type
  */
  enum_field_types field_type() const;

  /**
    If this wrapper holds a JSON array, get an array value by indexing
    into the array. Valid for J_ARRAY.  Calling this method if the type is
    not J_ARRAY will give undefined results.

    @return the array value
  */
  Json_wrapper operator[](size_t index) const;

  /**
    If this wrapper holds a JSON object, get the value corresponding
    to the member key. Valid for J_OBJECT.  Calling this method if the type is
    not J_OBJECT will give undefined results.

    @param[in]     key name for identifying member

    @return The member value. If there is no member with the specified
    name, a value with type Json_dom::J_ERROR is returned.
  */
  Json_wrapper lookup(const MYSQL_LEX_CSTRING &key) const;

  /**
    Get a pointer to the data of a JSON string or JSON opaque value.
    The data is still owner by the wrapper. The data may not be null
    terminated, so use in conjunction with @c get_data_length.
    Valid for J_STRING and J_OPAQUE.  Calling this method if the type is
    not one of those will give undefined results.

    @return the pointer
  */
  const char *get_data() const;

  /**
    Get the length to the data of a JSON string or JSON opaque value.
    Valid for J_STRING and J_OPAQUE.  Calling this method if the type is
    not one of those will give undefined results.

    @return the length
  */
  size_t get_data_length() const;

  /**
    Get the MySQL representation of a JSON decimal value.
    Valid for J_DECIMAL.  Calling this method if the type is
    not J_DECIMAL will give undefined results.

    @param[out] d  the decimal value
    @return false on success, true on failure (which would indicate an
    internal error)
  */
  bool get_decimal_data(my_decimal *d) const;

  /**
    Get the value of a JSON double number.
    Valid for J_DOUBLE.  Calling this method if the type is
    not J_DOUBLE will give undefined results.

    @return the value
  */
  double get_double() const;

  /**
    Get the value of a JSON signed integer number.
    Valid for J_INT.  Calling this method if the type is
    not J_INT will give undefined results.

    @return the value
  */
  longlong get_int() const;

  /**
    Get the value of a JSON unsigned integer number.
    Valid for J_UINT.  Calling this method if the type is
    not J_UINT will give undefined results.

    @return the value
  */
  ulonglong get_uint() const;

  /**
    Get the value of a JSON date/time value.  Valid for J_TIME,
    J_DATETIME, J_DATE and J_TIMESTAMP.  Calling this method if the type
    is not one of those will give undefined results.

    @param[out] t  the date/time value
  */
  void get_datetime(MYSQL_TIME *t) const;

  /**
    Get a boolean value (a JSON true or false literal).
    Valid for J_BOOLEAN.  Calling this method if the type is
    not J_BOOLEAN will give undefined results.

    @return the value
  */
  bool get_boolean() const;

  /**
    Finds all of the json sub-documents which match the path expression.
    Puts the matches on an evolving vector of results.
    This is a bit inefficient for binary wrappers because you can't
    build up a binary array incrementally from its cells. Instead, you
    have to turn each cell into a dom and then add the doms to a
    dom array.

    Calling this if #empty() returns true is an error.

    Special care must be taken when the path expression contains more than one
    ellipsis (**) token. That is because multiple paths with ellipses may
    identify the same value. Consider the following document:

        { "a": { "x" : { "b": { "y": { "b": { "z": { "c": 100 } } } } } } }

    The innermost value (the number 100) has the following unique,
    non-wildcarded address:

        $.a.x.b.y.b.z.c

    That location is reached by both of the following paths which include
    the ellipsis token:

        $.a.x.b**.c
        $.a.x.b.y.b**.c

    And those addresses both satisfy the following path expression which has
    two ellipses:

        $.a**.b**.c

    In this case, we only want to return one instance of $.a.x.b.y.b.z.c

    Similarly, special care must be taken if an auto-wrapping array
    path leg follows an ellipsis. Consider the following document:

        { "a": { "b" : [ 1, 2, 3 ] } }

    The first element of the array (the number 1) can be reached with
    either of these two non-wildcarded addresses, due to array auto-wrapping:

        $.a.b[0]
        $.a.b[0][0]

    Both of those addresses match the following path expression, which
    has an ellipsis followed by an auto-wrapping path leg:

        $**[0]

    @param[in] path   the (possibly wildcarded) address of the sub-documents
    @param[in] legs   the number of legs to use from @a path
    @param[out] hits  the result of the search
    @param[in] auto_wrap true of we match a final scalar with search for [0]
    @param[in]  only_need_one True if we can stop after finding one match

    @retval false on success
    @retval true on error
  */
  bool seek(const Json_seekable_path &path, size_t legs,
            Json_wrapper_vector *hits, bool auto_wrap, bool only_need_one);

  /**
    Compute the length of a document. This is the value which would be
    returned by the JSON_LENGTH() system function. So, this returns

    - for scalar values: 1
    - for objects: the number of members
    - for arrays: the number of cells

    @returns 1, the number of members, or the number of cells
  */
  size_t length() const;

  /**
    Compare this JSON value to another JSON value.
    @param[in] other the other JSON value
    @param[in] cs    if given, this charset will be used in comparison of
                     string values
    @param[in] legacy_val_cmp Use compare semantics of values in 5.6
    @retval -1 if this JSON value is less than the other JSON value
    @retval 0 if the two JSON values are equal
    @retval 1 if this JSON value is greater than the other JSON value
  */
  int compare(const Json_wrapper &other, const CHARSET_INFO *cs = nullptr,
              bool legacy_val_cmp = false) const;

  /**
    Extract an int (signed or unsigned) from the JSON if possible
    coercing if need be.
    @param[in]  msgnam to use in error message in conversion failed
    @param[out] err    true <=> error occur during coercion
    @param[in]  cr_error Whether to raise an error or warning on
                         data truncation
    @returns json value coerced to int
  */
  longlong coerce_int(const char *msgnam, bool *err = nullptr,
                      enum_coercion_error cr_error = CE_WARNING) const;

  /**
    Extract a real from the JSON if possible, coercing if need be.

    @param[in]  msgnam to use in error message in conversion failed
    @param[out] err    true <=> error occur during coercion
    @param[in]  cr_error Whether to raise an error or warning on
                         data truncation
    @returns json value coerced to real
  */
  double coerce_real(const char *msgnam, bool *err = nullptr,
                     enum_coercion_error cr_error = CE_WARNING) const;

  /**
    Extract a decimal from the JSON if possible, coercing if need be.

    @param[in,out] decimal_value a value buffer
    @param[in]  msgnam to use in error message in conversion failed
    @param[out] err    true <=> error occur during coercion
    @param[in]  cr_error Whether to raise an error or warning on
                         data truncation
    @returns json value coerced to decimal
  */
  my_decimal *coerce_decimal(my_decimal *decimal_value, const char *msgnam,
                             bool *err = nullptr,
                             enum_coercion_error cr_error = CE_WARNING) const;

  /**
    Extract a date from the JSON if possible, coercing if need be.

    @param[in,out] ltime a value buffer
    @param msgnam
    @param[in]  cr_error Whether to raise an error or warning on
                         data truncation
    @returns json value coerced to date
   */
  bool coerce_date(MYSQL_TIME *ltime, const char *msgnam,
                   enum_coercion_error cr_error = CE_WARNING) const;

  /**
    Extract a time value from the JSON if possible, coercing if need be.

    @param[in,out] ltime a value buffer
    @param msgnam
    @param[in]  cr_error Whether to raise an error or warning on
                         data truncation

    @returns json value coerced to time
  */
  bool coerce_time(MYSQL_TIME *ltime, const char *msgnam,
                   enum_coercion_error cr_error = CE_WARNING) const;

  /**
    Make a sort key that can be used by filesort to order JSON values.

    @param[out] to      a buffer to which the sort key is written
    @param[in]  length  the length of the sort key

    @details Key storage format is following:
    @verbatim
      |<json type><   sort key    >|
       1 byte    / variable length /
    @endverbatim

    JSON is assumed to be non-sql-null and valid (checked by caller).
    Key length contains full length - the len prefix itself, json type and the
    sort key.
    All numeric types are stored as a number, without distinction to
    double/decimal/int/etc. See @c make_json_numeric_sort_key().
    Same is done to DATETIME and TIMESTAMP types.
    For string and opaque types only the prefix that fits into the output buffer
    is stored.
    For JSON objects and arrays only their length (number of elements) is
    stored, this is a limitation of current implementation.
  */
  size_t make_sort_key(uchar *to, size_t length) const;

  /**
    Make a hash key that can be used by sql_executor.cc/unique_hash
    in order to support SELECT DISTINCT

    @param[in]  hash_val  An initial hash value.
  */
  ulonglong make_hash_key(ulonglong hash_val) const;

  /**
    Calculate the amount of unused space inside a JSON binary value.

    @param[out] space  the amount of unused space, or zero if this is a DOM
    @return false on success
    @return true if the JSON binary value was invalid
  */
  bool get_free_space(size_t *space) const;

  /**
    Attempt a binary partial update by replacing the value at @a path with @a
    new_value. On successful completion, the updated document will be available
    in @a result, and this Json_wrapper will point to @a result instead of the
    original binary representation. The modifications that have been applied,
    will also be collected as binary diffs, which can be retrieved via
    TABLE::get_binary_diffs().

    @param field           the column being updated
    @param path            the path of the value to update
    @param new_value       the new value
    @param replace         true if we use JSON_REPLACE semantics
    @param[in,out] result  buffer that holds the updated JSON document (is
                           empty if no partial update has been performed on
                           this Json_wrapper so far, or contains the binary
                           representation of the document in this wrapper
                           otherwise)
    @param[out] partially_updated gets set to true if partial update was
                                  successful, also if it was a no-op
    @param[out] replaced_path     gets set to true if the path was replaced,
                                  will be false if this update is a no-op

    @retval false     if the update was successful, or if it was determined
                      that a full update was needed
    @retval true      if an error occurred
  */
  bool attempt_binary_update(const Field_json *field,
                             const Json_seekable_path &path,
                             Json_wrapper *new_value, bool replace,
                             String *result, bool *partially_updated,
                             bool *replaced_path);

  /**
    Remove a path from a binary JSON document. On successful completion, the
    updated document will be available in @a result, and this Json_wrapper will
    point to @a result instead of the original binary representation. The
    modifications that have been applied, will also be collected as binary
    diffs, which can be retrieved via TABLE::get_binary_diffs().

    @param field   the column being updated
    @param path    the path to remove from the document
    @param[in,out] result  buffer that holds the updated JSON document (is
                           empty if no partial update has been performed on
                           this Json_wrapper so far, or contains the binary
                           representation of the document in this wrapper
                           otherwise)
    @param[out] found_path gets set to true if the path is found in the
                           document, false otherwise

    @retval false   if the value was successfully updated
    @retval true    if an error occurred
  */
  bool binary_remove(const Field_json *field, const Json_seekable_path &path,
                     String *result, bool *found_path);
#ifdef MYSQL_SERVER
  /**
    Sort contents. Applicable to JSON arrays only.
  */
  void sort(const CHARSET_INFO *cs = nullptr);
  /**
    Remove duplicate values. Applicable to JSON arrays only, array will be
    sorted.
  */
  void remove_duplicates(const CHARSET_INFO *cs = nullptr);
#endif

  /**
     Sets the json format of the json contained inside wrapper.
     m_legacy_json inside Json_wrapper takes care of the printing behavior
     of 5.6 json functions.

     @param [in] legacy_json  enum governing print behavior of 5.6 json.
   */
  void set_legacy_json(legacy_json_print_behavior legacy_json) {
    m_legacy_json = legacy_json;
  }

  bool is_legacy_json() const { return m_legacy_json != LEGACY_JSON_DISABLED; }
};

/**
  Class that iterates over all members of a JSON object that is wrapped in a
  Json_wrapper instance.
*/
class Json_wrapper_object_iterator {
 public:
  // Type aliases required by ForwardIterator.
  using value_type = std::pair<MYSQL_LEX_CSTRING, Json_wrapper>;
  using reference = const value_type &;
  using pointer = const value_type *;
  using difference_type = ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  /**
    Creates an iterator that iterates over all members of the given
    Json_wrapper, if it wraps a JSON object. If the wrapper does not wrap a JSON
    object, the result is undefined.

    @param wrapper the Json_wrapper to iterate over
    @param begin   true to construct an iterator that points to the first member
                   of the object, false to construct a past-the-end iterator
  */
  Json_wrapper_object_iterator(const Json_wrapper &wrapper, bool begin);

  /// Forward iterators must be default constructible.
  Json_wrapper_object_iterator() = default;

  /// Advances the iterator to the next element.
  Json_wrapper_object_iterator &operator++() {
    if (is_dom())
      ++m_iter;
    else
      ++m_current_element_index;
    m_current_member_initialized = false;
    return *this;
  }

  /**
    Advances the iterator to the next element and returns an iterator that
    points to the current element (post-increment operator).
  */
  const Json_wrapper_object_iterator operator++(int) {
    Json_wrapper_object_iterator copy = *this;
    ++(*this);
    return copy;
  }

  /// Checks two iterators for equality.
  bool operator==(const Json_wrapper_object_iterator &other) const {
    return is_dom() ? m_iter == other.m_iter
                    : m_current_element_index == other.m_current_element_index;
  }

  /// Checks two iterators for inequality.
  bool operator!=(const Json_wrapper_object_iterator &other) const {
    return !(*this == other);
  }

  pointer operator->() {
    if (!m_current_member_initialized) initialize_current_member();
    return &m_current_member;
  }

  reference operator*() { return *this->operator->(); }

 private:
  /// Pair holding the key and value of the member pointed to by the iterator.
  value_type m_current_member;
  /// True if #m_current_member is initialized.
  bool m_current_member_initialized{false};
  /// The binary JSON object being iterated over, or nullptr for DOMs.
  const json_binary::Value *m_binary_value;
  /// The index of the current member in the binary JSON object.
  size_t m_current_element_index;
  /// Iterator pointing to the current member in the JSON DOM object.
  Json_object::const_iterator m_iter;
  /// Returns true if iterating over a DOM.
  bool is_dom() const { return m_binary_value == nullptr; }
  /// Fill #m_current_member with the key and value of the current member.
  void initialize_current_member();
};

/**
  A wrapper over a JSON object which provides an interface that can be iterated
  over with a for-each loop.
*/
class Json_object_wrapper {
 public:
  using const_iterator = Json_wrapper_object_iterator;
  explicit Json_object_wrapper(const Json_wrapper &wrapper)
      : m_wrapper(wrapper) {}
  const_iterator cbegin() const { return const_iterator(m_wrapper, true); }
  const_iterator cend() const { return const_iterator(m_wrapper, false); }
  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }

 private:
  const Json_wrapper &m_wrapper;
};

/**
  Check if a string contains valid JSON text, without generating a
  Json_dom representation of the document.

  @param[in] text    pointer to the beginning of the string
  @param[in] length  the length of the string
  @return true if the string is valid JSON text, false otherwise
*/
bool is_valid_json_syntax(const char *text, size_t length);

/**
  A class that is capable of holding objects of any sub-type of
  Json_scalar. Used for pre-allocating space in query-duration memory
  for JSON scalars that are to be returned by get_json_atom_wrapper().

  This class should be replaced by std::variant when moving to C++17.
*/
class Json_scalar_holder {
  /// Union of all concrete subclasses of Json_scalar.
  union Any_json_scalar {
    Json_string m_string;
    Json_decimal m_decimal;
    Json_int m_int;
    Json_uint m_uint;
    Json_double m_double;
    Json_boolean m_boolean;
    Json_null m_null;
    Json_datetime m_datetime;
    Json_opaque m_opaque;
    /// Constructor which initializes the union to hold a Json_null value.
    Any_json_scalar() : m_null() {}
    /// Destructor which delegates to Json_scalar's virtual destructor.
    ~Any_json_scalar() {
      // All members have the same address, and all members are sub-types of
      // Json_scalar, so we can take the address of an arbitrary member and
      // convert it to Json_scalar.
      Json_scalar *scalar = &m_null;
      scalar->~Json_scalar();
    }
  };

  /// The buffer in which the Json_scalar value is stored.
  Any_json_scalar m_buffer;

  /// Pointer to the held scalar, or nullptr if no value is held.
  Json_scalar *m_scalar_ptr{nullptr};

 public:
  /// Get a pointer to the held object, or nullptr if there is none.
  Json_scalar *get() { return m_scalar_ptr; }

  /**
    Construct a new Json_scalar value in this Json_scalar_holder.
    If a value is already held, the old value is destroyed and replaced.
    @tparam T which type of Json_scalar to create
    @param args the arguments to T's constructor
  */
  template <typename T, typename... Args>
  void emplace(Args &&... args) {
    static_assert(std::is_base_of<Json_scalar, T>::value, "Not a Json_scalar");
    static_assert(sizeof(T) <= sizeof(m_buffer), "Buffer is too small");
    m_scalar_ptr = &m_buffer.m_null;
    m_scalar_ptr->~Json_scalar();
    ::new (m_scalar_ptr) T(std::forward<Args>(args)...);
  }
};

#endif /* JSON_DOM_INCLUDED */
