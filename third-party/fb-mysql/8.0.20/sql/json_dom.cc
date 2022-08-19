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

#include "sql/json_dom.h"

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>   // std::min, std::max
#include <cmath>       // std::isfinite
#include <functional>  // std::function
#include <new>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/reader.h>

#include "base64.h"
#include "decimal.h"
#include "json_binary.h"
#include "m_ctype.h"
#include "m_string.h"  // my_gcvt, _dig_vec_lower
#include "malloc_allocator.h"
#include "my_byteorder.h"
#include "my_compare.h"
#include "my_dbug.h"
#include "my_decimal.h"
#include "my_double2ulonglong.h"
#include "my_sys.h"
#include "my_time.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_com.h"
#include "mysqld_error.h"  // ER_*
#ifdef MYSQL_SERVER
#include "sql/check_stack.h"
#endif
#include "sql/current_thd.h"  // current_thd
#include "sql/derror.h"       // ER_THD
#include "sql/field.h"
#include "sql/json_path.h"
#include "sql/json_syntax_check.h"
#include "sql/psi_memory_key.h"  // key_memory_JSON
#include "sql/sql_class.h"       // THD
#include "sql/sql_const.h"       // STACK_MIN_SIZE
#include "sql/sql_error.h"
#include "sql/sql_sort.h"
#include "sql/sql_time.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql_string.h"
#include "template_utils.h"  // down_cast, pointer_cast

#ifdef MYSQL_SERVER
static Json_dom *json_binary_to_dom_template(const json_binary::Value &v);
static bool populate_object_or_array(const THD *thd, Json_dom *dom,
                                     const json_binary::Value &v);
static bool populate_object(const THD *thd, Json_object *jo,
                            const json_binary::Value &v);
static bool populate_array(const THD *thd, Json_array *ja,
                           const json_binary::Value &v);

/**
  Auto-wrap a dom in an array if it is not already an array. Delete
  the dom if there is a memory allocation failure.
*/
static Json_array_ptr wrap_in_array(Json_dom_ptr dom) {
  if (dom->json_type() == enum_json_type::J_ARRAY)
    return Json_array_ptr(down_cast<Json_array *>(dom.release()));

  Json_array_ptr a = create_dom_ptr<Json_array>();
  if (a == nullptr || a->append_alias(std::move(dom)))
    return nullptr; /* purecov: inspected */
  return a;
}

Json_dom_ptr merge_doms(Json_dom_ptr left, Json_dom_ptr right) {
  if (left->json_type() == enum_json_type::J_OBJECT &&
      right->json_type() == enum_json_type::J_OBJECT) {
    Json_object_ptr left_object(down_cast<Json_object *>(left.release()));
    Json_object_ptr right_object(down_cast<Json_object *>(right.release()));
    if (left_object->consume(std::move(right_object)))
      return nullptr; /* purecov: inspected */
#ifdef __SUNPRO_CC
    return std::move(left_object);
#else
    return left_object;
#endif
  }

  Json_array_ptr left_array = wrap_in_array(std::move(left));
  Json_array_ptr right_array = wrap_in_array(std::move(right));
  if (left_array == nullptr || right_array == nullptr ||
      left_array->consume(std::move(right_array)))
    return nullptr; /* purecov: inspected */

#ifdef __SUNPRO_CC
  return std::move(left_array);
#else
  return left_array;
#endif
}
#endif  // ifdef MYSQL_SERVER

void *Json_dom::operator new(size_t size, const std::nothrow_t &) noexcept {
  /*
    Call my_malloc() with the MY_WME flag to make sure that it will
    write an error message if the memory could not be allocated.
  */
  return my_malloc(
#ifdef MYSQL_SERVER
      key_memory_JSON,
#else
      PSI_NOT_INSTRUMENTED,
#endif
      size, MYF(MY_WME));
}

void Json_dom::operator delete(void *ptr) noexcept { my_free(ptr); }

/*
  This operator is included in order to silence warnings on some
  compilers. It is called if the constructor throws an exception when
  an object is allocated with nothrow new. This is not supposed to
  happen and is therefore hard to test, so annotate it to avoid
  cluttering the test coverage reports.
*/
/* purecov: begin inspected */
void Json_dom::operator delete(void *ptr, const std::nothrow_t &)noexcept {
  operator delete(ptr);
}
/* purecov: end */

#ifdef MYSQL_SERVER
/**
  Add a value to a vector if it isn't already there.

  This is used for removing duplicate matches for daisy-chained
  ellipsis tokens in #find_child_doms(). The problem with
  daisy-chained ellipses is that the candidate set may contain the
  same Json_dom object multiple times at different nesting levels
  after matching the first ellipsis. That is, the candidate set may
  contain a Json_dom and its parent, grandparent and so on. When
  matching the next ellipsis in the path, each value in the candidate
  set and all its children will be inspected, so the nested Json_dom
  will be seen multiple times, as its grandparent, parent and finally
  itself are inspected. We want it to appear only once in the result.

  The same problem occurs if a possibly auto-wrapping array path leg
  comes after an ellipsis. If the candidate set contains both an array
  element and its parent array due to the ellipsis, the auto-wrapping
  path leg may match the array element twice, and we only want it once
  in the result.

  @param[in] candidate value to add
  @param[in,out] duplicates set of values added, or `nullptr` if duplicate
                            checking is not needed
  @param[in,out] result vector
  @return false on success, true on error
*/
static bool add_if_missing(Json_dom *candidate, Json_dom_vector *duplicates,
                           Json_dom_vector *result) {
  /*
    If we are not checking duplicates, or if the candidate is not
    already in the duplicate set, add the candidate to the result
    vector.
  */
  if (duplicates == nullptr || duplicates->insert_unique(candidate).second) {
    return result->push_back(candidate);
  }
  return false;
}

/**
  Check if a seek operation performed by find_child_doms()
  or Json_dom::seek() is done.

  @return true if only one result is needed and a result has been found
*/
static inline bool is_seek_done(const Json_dom_vector *hits,
                                bool only_need_one) {
  return only_need_one && hits->size() > 0;
}

/**
  Find the child Json_dom objects identified by the given path.
  The child doms are added to a vector.

  See the header comment for #Json_wrapper::seek() for a discussion
  of complexities involving path expressions with more than one
  ellipsis (**) token, or a combination of ellipsis and auto-wrapping
  path legs.

  @param[in]     dom the DOM to search
  @param[in]     current_leg iterator to the path leg to look at
  @param[in]     last_leg    iterator to the last path leg (exclusive)
  @param[in]     auto_wrap if true, auto-wrap non-arrays when matching against
                           array path legs
  @param[in]     only_need_one true if we can stop after finding one match
  @param[in,out] duplicates set of values collected, which helps to identify
                 duplicate arrays and objects introduced by daisy-chained
                 ** tokens or auto-wrapping, or `nullptr` if duplicate
                 elimination is not needed for this path leg
  @param[in,out] result the vector of qualifying children
  @return false on success, true on error
*/
static bool find_child_doms(Json_dom *dom,
                            const Json_path_iterator &current_leg,
                            const Json_path_iterator &last_leg, bool auto_wrap,
                            bool only_need_one, Json_dom_vector *duplicates,
                            Json_dom_vector *result) {
  if (current_leg == last_leg) return add_if_missing(dom, duplicates, result);

  const enum_json_type dom_type = dom->json_type();
  const Json_path_leg *const path_leg = *current_leg;
  const Json_path_iterator next_leg = current_leg + 1;

  switch (path_leg->get_type()) {
    case jpl_array_cell:
      if (dom_type == enum_json_type::J_ARRAY) {
        const auto array = down_cast<const Json_array *>(dom);
        const Json_array_index idx = path_leg->first_array_index(array->size());
        return idx.within_bounds() &&
               find_child_doms((*array)[idx.position()], next_leg, last_leg,
                               auto_wrap, only_need_one, duplicates, result);
      }
      // Handle auto-wrapping of non-arrays.
      return auto_wrap && path_leg->is_autowrap() &&
             find_child_doms(dom, next_leg, last_leg, auto_wrap, only_need_one,
                             duplicates, result);
    case jpl_array_range:
    case jpl_array_cell_wildcard:
      if (dom_type == enum_json_type::J_ARRAY) {
        const auto array = down_cast<const Json_array *>(dom);
        const auto range = path_leg->get_array_range(array->size());
        for (size_t i = range.m_begin; i < range.m_end; ++i) {
          if (find_child_doms((*array)[i], next_leg, last_leg, auto_wrap,
                              only_need_one, duplicates, result))
            return true; /* purecov: inspected */
          if (is_seek_done(result, only_need_one)) return false;
        }
        return false;
      }
      // Handle auto-wrapping of non-arrays.
      return auto_wrap && path_leg->is_autowrap() &&
             find_child_doms(dom, next_leg, last_leg, auto_wrap, only_need_one,
                             duplicates, result);
    case jpl_ellipsis: {
      // The ellipsis matches the value on which it is called ...
      if (find_child_doms(dom, next_leg, last_leg, auto_wrap, only_need_one,
                          duplicates, result))
        return true; /* purecov: inspected */

      // ... and, recursively, all the values contained in it.
      if (dom_type == enum_json_type::J_ARRAY) {
        for (const Json_dom_ptr &child : *down_cast<const Json_array *>(dom)) {
          if (is_seek_done(result, only_need_one)) return false;

          // Now recurse and add the child and values under it.
          if (find_child_doms(child.get(), current_leg, last_leg, auto_wrap,
                              only_need_one, duplicates, result))
            return true; /* purecov: inspected */
        }                // end of loop through children
      } else if (dom_type == enum_json_type::J_OBJECT) {
        for (const auto &member : *down_cast<const Json_object *>(dom)) {
          if (is_seek_done(result, only_need_one)) return false;

          // Now recurse and add the child and values under it.
          if (find_child_doms(member.second.get(), current_leg, last_leg,
                              auto_wrap, only_need_one, duplicates, result))
            return true; /* purecov: inspected */
        }                // end of loop through children
      }

      return false;
    }
    case jpl_member: {
      if (dom_type == enum_json_type::J_OBJECT) {
        const auto object = down_cast<const Json_object *>(dom);
        Json_dom *child = object->get(path_leg->get_member_name());
        return child != nullptr &&
               find_child_doms(child, next_leg, last_leg, auto_wrap,
                               only_need_one, duplicates, result);
      }

      return false;
    }
    case jpl_member_wildcard: {
      if (dom_type == enum_json_type::J_OBJECT) {
        for (const auto &member : *down_cast<const Json_object *>(dom)) {
          if (find_child_doms(member.second.get(), next_leg, last_leg,
                              auto_wrap, only_need_one, duplicates, result))
            return true; /* purecov: inspected */
          if (is_seek_done(result, only_need_one)) return false;
        }
      }

      return false;
    }
  }

  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return true;
  /* purecov: end */
}

/**
  Does a search on this path, using Json_dom::seek() or
  Json_wrapper::seek(), need duplicate elimination?

  Duplicate elimination is needed if the path contains multiple
  ellipses, or if it contains an auto-wrapping array path leg after an
  ellipses. See #Json_wrapper::seek() for more details.

  @param begin      the beginning of the path
  @param end        the end of the path (exclusive)
  @param auto_wrap  true if array auto-wrapping is used

  @retval true if duplicate elimination is needed
  @retval false if the path won't produce duplicates
*/
static bool path_gives_duplicates(const Json_path_iterator &begin,
                                  const Json_path_iterator &end,
                                  bool auto_wrap) {
  auto it = std::find_if(begin, end, [](const Json_path_leg *leg) {
    return leg->get_type() == jpl_ellipsis;
  });

  // If no ellipsis, no duplicates.
  if (it == end) return false;

  // Otherwise, possibly duplicates if ellipsis or autowrap leg follows.
  return std::any_of(it + 1, end, [auto_wrap](const Json_path_leg *leg) {
    return leg->get_type() == jpl_ellipsis || (auto_wrap && leg->is_autowrap());
  });
}
#endif  // ifdef MYSQL_SERVER

Json_object::Json_object(bool legacy_object)
    : m_map(Json_object_map::key_compare(), Json_object_map::allocator_type(
#ifdef MYSQL_SERVER
                                                key_memory_JSON
#else
                                                PSI_NOT_INSTRUMENTED
#endif
                                                )),
      m_legacy_object(legacy_object) {
}

namespace {

/**
  This class implements rapidjson's Handler concept to make our own handler
  which will construct our DOM from the parsing of the JSON text.
  <code>
  bool Null() {   }
  bool Bool(bool) {   }
  bool Int(int) {   }
  bool Uint(unsigned) {   }
  bool Int64(int64_t) {   }
  bool Uint64(uint64_t) {   }
  bool Double(double) {   }
  bool RawNumber(const Ch*, SizeType, bool) {   }
  bool String(const Ch*, SizeType, bool) {   }
  bool StartObject() {   }
  bool Key() {   }
  bool EndObject(SizeType) {   }
  bool StartArray() {   }
  bool EndArray(SizeType) {   }
  </code>
  @see Json_dom::parse
*/
#ifdef MYSQL_SERVER
class Rapid_json_handler {
 private:
// std::cerr << "callback " << name << ':' << state << '\n'; std::cerr.flush()
#define DUMP_CALLBACK(name, state)

  enum enum_state {
    expect_anything,
    expect_array_value,
    expect_object_key,
    expect_object_value,
    expect_eof
  };

  enum_state m_state;           ///< Tells what kind of value to expect next.
  Json_dom_ptr m_dom_as_built;  ///< Root of the DOM being built.
  Json_dom *m_current_element;  ///< The current object/array being parsed.
  size_t m_depth;     ///< The depth at which parsing currently happens.
  std::string m_key;  ///< The name of the current member of an object.

  bool m_legacy_parse;  ///< Parse json document compatible with 5.6 fb json
                        ///< API's >

 public:
  Rapid_json_handler(bool legacy_parsing = false)
      : m_state(expect_anything),
        m_dom_as_built(nullptr),
        m_current_element(nullptr),
        m_depth(0),
        m_key(),
        m_legacy_parse(legacy_parsing) {}

  /**
    @returns The built JSON DOM object.
    Deallocation of the returned value is the responsibility of the caller.
  */
  Json_dom_ptr get_built_doc() { return std::move(m_dom_as_built); }

 private:
  /**
    Function which is called on each value found in the JSON
    document being parsed.

    @param[in] value the value that was seen
    @return true if parsing should continue, false if an error was
            found and parsing should stop
  */
  bool seeing_value(Json_dom_ptr value) {
    if (value == nullptr) return false; /* purecov: inspected */
    switch (m_state) {
      case expect_anything:
        m_dom_as_built = std::move(value);
        m_state = expect_eof;
        return true;
      case expect_array_value: {
        auto array = down_cast<Json_array *>(m_current_element);
        if (array->append_alias(std::move(value)))
          return false; /* purecov: inspected */
        return true;
      }
      case expect_object_value: {
        m_state = expect_object_key;
        auto object = down_cast<Json_object *>(m_current_element);
        return !object->add_alias(m_key, std::move(value));
      }
      default:
        /* purecov: begin inspected */
        DBUG_ASSERT(false);
        return false;
        /* purecov: end */
    }
  }

 public:
  bool Null() {
    DUMP_CALLBACK("null", state);
    return seeing_value(create_dom_ptr<Json_null>());
  }

  bool Bool(bool b) {
    DUMP_CALLBACK("bool", state);
    return seeing_value(create_dom_ptr<Json_boolean>(b));
  }

  bool Int(int i) {
    DUMP_CALLBACK("int", state);
    return seeing_value(create_dom_ptr<Json_int>(i));
  }

  bool Uint(unsigned u) {
    DUMP_CALLBACK("uint", state);
    return seeing_value(create_dom_ptr<Json_int>(static_cast<longlong>(u)));
  }

  bool Int64(int64_t i) {
    DUMP_CALLBACK("int64", state);
    return seeing_value(create_dom_ptr<Json_int>(i));
  }

  bool Uint64(uint64_t ui64) {
    DUMP_CALLBACK("uint64", state);
    return seeing_value(create_dom_ptr<Json_uint>(ui64));
  }

  bool Double(double d) {
    DUMP_CALLBACK("double", state);
    /*
      We only accept finite values. RapidJSON normally stops non-finite values
      from getting here, but sometimes +/-inf values could end up here anyway.
    */
    if (!std::isfinite(d)) return false;
    return seeing_value(create_dom_ptr<Json_double>(d));
  }

  /* purecov: begin deadcode */
  bool RawNumber(const char *, rapidjson::SizeType, bool) {
    /*
      Never called, since we don't instantiate the parser with
      kParseNumbersAsStringsFlag.
    */
    DBUG_ASSERT(false);
    return false;
  }
  /* purecov: end */

  bool String(const char *str, rapidjson::SizeType length, bool) {
    DUMP_CALLBACK("string", state);
    return seeing_value(create_dom_ptr<Json_string>(str, length));
  }

  bool StartObject() {
    DUMP_CALLBACK("start object {", state);
    return start_object_or_array(create_dom_ptr<Json_object>(m_legacy_parse),
                                 expect_object_key);
  }

  bool EndObject(rapidjson::SizeType) {
    DUMP_CALLBACK("} end object", state);
    DBUG_ASSERT(m_state == expect_object_key);
    end_object_or_array();
    return true;
  }

  bool StartArray() {
    DUMP_CALLBACK("start array [", state);
    return start_object_or_array(create_dom_ptr<Json_array>(),
                                 expect_array_value);
  }

  bool EndArray(rapidjson::SizeType) {
    DUMP_CALLBACK("] end array", state);
    DBUG_ASSERT(m_state == expect_array_value);
    end_object_or_array();
    return true;
  }

  bool Key(const char *str, rapidjson::SizeType len, bool) {
    DBUG_ASSERT(m_state == expect_object_key);
    m_state = expect_object_value;
    m_key.assign(str, len);
    return true;
  }

 private:
  bool start_object_or_array(Json_dom_ptr value, enum_state next_state) {
    Json_dom *dom = value.get();
    bool success =
        seeing_value(std::move(value)) && !check_json_depth(++m_depth);
    m_current_element = dom;
    m_state = next_state;
    return success;
  }

  void end_object_or_array() {
    m_depth--;
    m_current_element = m_current_element->parent();
    if (m_current_element == nullptr) {
      DBUG_ASSERT(m_depth == 0);
      m_state = expect_eof;
    } else if (m_current_element->json_type() == enum_json_type::J_OBJECT)
      m_state = expect_object_key;
    else {
      DBUG_ASSERT(m_current_element->json_type() == enum_json_type::J_ARRAY);
      m_state = expect_array_value;
    }
  }
};
#endif  // ifdef MYSQL_SERVER

}  // namespace

#ifdef MYSQL_SERVER
Json_dom_ptr Json_dom::parse(const char *text, size_t length,
                             bool legacy_parsing, const char **syntaxerr,
                             size_t *offset) {
  Rapid_json_handler handler(legacy_parsing);
  rapidjson::MemoryStream ss(text, length);
  rapidjson::Reader reader;
  bool success = reader.Parse<rapidjson::kParseDefaultFlags>(ss, handler);

  if (success) {
    Json_dom_ptr dom = handler.get_built_doc();
    if (dom == nullptr && syntaxerr != nullptr) {
      // The parsing failed for some other reason than a syntax error.
      *syntaxerr = nullptr;
    }
    return dom;
  }

  // Report the error offset and the error message if requested by the caller.
  if (offset != nullptr) *offset = reader.GetErrorOffset();
  if (syntaxerr != nullptr)
    *syntaxerr = rapidjson::GetParseError_En(reader.GetParseErrorCode());

  return nullptr;
}
#endif  // ifdef MYSQL_SERVER

/**
  Map the JSON type used by the binary representation to the type
  used by Json_dom and Json_wrapper.

  Note: Does not look into opaque values to determine if they
  represent decimal or date/time values. For that, look into the
  Value an retrive field_type.

  @param[in]  bintype
  @returns the JSON_dom JSON type.
*/
static enum_json_type bjson2json(const json_binary::Value::enum_type bintype) {
  enum_json_type res = enum_json_type::J_ERROR;

  switch (bintype) {
    case json_binary::Value::STRING:
      res = enum_json_type::J_STRING;
      break;
    case json_binary::Value::INT:
      res = enum_json_type::J_INT;
      break;
    case json_binary::Value::UINT:
      res = enum_json_type::J_UINT;
      break;
    case json_binary::Value::DOUBLE:
      res = enum_json_type::J_DOUBLE;
      break;
    case json_binary::Value::LITERAL_TRUE:
    case json_binary::Value::LITERAL_FALSE:
      res = enum_json_type::J_BOOLEAN;
      break;
    case json_binary::Value::LITERAL_NULL:
      res = enum_json_type::J_NULL;
      break;
    case json_binary::Value::ARRAY:
      res = enum_json_type::J_ARRAY;
      break;
    case json_binary::Value::OBJECT:
      res = enum_json_type::J_OBJECT;
      break;
    case json_binary::Value::ERROR:
      res = enum_json_type::J_ERROR;
      break;
    case json_binary::Value::OPAQUE:
      res = enum_json_type::J_OPAQUE;
      break;
  }

  return res;
}

#ifdef MYSQL_SERVER
Json_dom_ptr Json_dom::parse(const THD *thd, const json_binary::Value &v) {
  Json_dom_ptr dom(json_binary_to_dom_template(v));
  if (dom == nullptr || populate_object_or_array(thd, dom.get(), v))
    return nullptr; /* purecov: inspected */
  return dom;
}

/// Get string data as std::string from a json_binary::Value.
static std::string get_string_data(const json_binary::Value &v) {
  return std::string(v.get_data(), v.get_data_length());
}

/**
  Create a DOM template for the provided json_binary::Value.

  If the binary value represents a scalar, create a Json_dom object
  that represents the scalar and return a pointer to it.

  If the binary value represents an object or an array, create an
  empty Json_object or Json_array object and return a pointer to it.

  @param v  the binary value to convert to DOM

  @return a DOM template for the top-level the binary value, or NULL
  if an error is detected.
*/
static Json_dom *json_binary_to_dom_template(const json_binary::Value &v) {
  switch (v.type()) {
    case json_binary::Value::OBJECT:
      return new (std::nothrow) Json_object();
    case json_binary::Value::ARRAY:
      return new (std::nothrow) Json_array();
    case json_binary::Value::DOUBLE:
      return new (std::nothrow) Json_double(v.get_double());
    case json_binary::Value::INT:
      return new (std::nothrow) Json_int(v.get_int64());
    case json_binary::Value::UINT:
      return new (std::nothrow) Json_uint(v.get_uint64());
    case json_binary::Value::LITERAL_FALSE:
      return new (std::nothrow) Json_boolean(false);
    case json_binary::Value::LITERAL_TRUE:
      return new (std::nothrow) Json_boolean(true);
    case json_binary::Value::LITERAL_NULL:
      return new (std::nothrow) Json_null();
    case json_binary::Value::OPAQUE: {
      const enum_field_types ftyp = v.field_type();

      if (ftyp == MYSQL_TYPE_NEWDECIMAL) {
        my_decimal m;
        if (Json_decimal::convert_from_binary(v.get_data(), v.get_data_length(),
                                              &m))
          return nullptr; /* purecov: inspected */
        return new (std::nothrow) Json_decimal(m);
      }

      if (ftyp == MYSQL_TYPE_DATE || ftyp == MYSQL_TYPE_TIME ||
          ftyp == MYSQL_TYPE_DATETIME || ftyp == MYSQL_TYPE_TIMESTAMP) {
        MYSQL_TIME t;
        Json_datetime::from_packed(v.get_data(), ftyp, &t);
        return new (std::nothrow) Json_datetime(t, ftyp);
      }

      return new (std::nothrow)
          Json_opaque(v.field_type(), v.get_data(), v.get_data_length());
    }
    case json_binary::Value::STRING:
      return new (std::nothrow) Json_string(v.get_data(), v.get_data_length());
    case json_binary::Value::ERROR:
      break; /* purecov: inspected */
  }

  /* purecov: begin inspected */
  my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
  return nullptr;
  /* purecov: end */
}

/**
  Populate the DOM representation of a JSON object or array with the
  elements found in a binary JSON object or array. If the supplied
  value does not represent an object or an array, do nothing.

  @param[in]     thd    THD handle
  @param[in,out] dom    the Json_dom object to populate
  @param[in]     v      the binary JSON value to read from

  @retval true on error
  @retval false on success
*/
static bool populate_object_or_array(const THD *thd, Json_dom *dom,
                                     const json_binary::Value &v) {
  switch (v.type()) {
    case json_binary::Value::OBJECT:
      // Check that we haven't run out of stack before we dive into the object.
      return check_stack_overrun(thd, STACK_MIN_SIZE, nullptr) ||
             populate_object(thd, down_cast<Json_object *>(dom), v);
    case json_binary::Value::ARRAY:
      // Check that we haven't run out of stack before we dive into the array.
      return check_stack_overrun(thd, STACK_MIN_SIZE, nullptr) ||
             populate_array(thd, down_cast<Json_array *>(dom), v);
    default:
      return false;
  }
}

/**
  Populate the DOM representation of a JSON object with the key/value
  pairs found in a binary JSON object.

  @param[in]     thd    THD handle
  @param[in,out] jo     the JSON object to populate
  @param[in]     v      the binary JSON object to read from

  @retval true on error
  @retval false on success
*/
static bool populate_object(const THD *thd, Json_object *jo,
                            const json_binary::Value &v) {
  for (uint32 i = 0; i < v.element_count(); i++) {
    auto key = get_string_data(v.key(i));
    auto val = v.element(i);
    auto dom = json_binary_to_dom_template(val);
    if (jo->add_alias(key, dom) || populate_object_or_array(thd, dom, val))
      return true; /* purecov: inspected */
  }
  return false;
}

/**
  Populate the DOM representation of a JSON array with the elements
  found in a binary JSON array.

  @param[in]     thd    THD handle
  @param[in,out] ja     the JSON array to populate
  @param[in]     v      the binary JSON array to read from

  @retval true on error
  @retval false on success
*/
static bool populate_array(const THD *thd, Json_array *ja,
                           const json_binary::Value &v) {
  for (uint32 i = 0; i < v.element_count(); i++) {
    auto elt = v.element(i);
    auto dom = json_binary_to_dom_template(elt);
    if (ja->append_alias(dom)) return true; /* purecov: inspected */
    if (populate_object_or_array(thd, dom, elt))
      return true; /* purecov: inspected */
  }
  return false;
}

namespace {

/**
  Functor which compares a child DOM of a JSON array or JSON object
  for equality.
*/
struct Json_child_equal {
  const Json_dom *const m_ptr;
  bool operator()(const Json_dom_ptr &dom) const { return dom.get() == m_ptr; }
  bool operator()(const Json_object_map::value_type &member) const {
    return member.second.get() == m_ptr;
  }
};

}  // namespace

void Json_array::replace_dom_in_container(const Json_dom *oldv,
                                          Json_dom_ptr newv) {
  auto it = std::find_if(m_v.begin(), m_v.end(), Json_child_equal{oldv});
  if (it != m_v.end()) {
    newv->set_parent(this);
    *it = std::move(newv);
  }
}

void Json_object::replace_dom_in_container(const Json_dom *oldv,
                                           Json_dom_ptr newv) {
  auto it = std::find_if(m_map.begin(), m_map.end(), Json_child_equal{oldv});
  if (it != m_map.end()) {
    newv->set_parent(this);
    it->second = std::move(newv);
  }
}
#endif  // ifdef MYSQL_SERVER

bool Json_object::add_alias(const std::string &key, Json_dom_ptr value) {
  if (!value) return true; /* purecov: inspected */

  // We have taken over the ownership of this value.
  value->set_parent(this);

  /*
    Insert the key and the value into the map. If we have already an element
    with this key, the old value is replaced. Note we compare utf-8 bytes
    directly here. It's complicated when you take into account composed
    and decomposed forms of accented characters and ligatures: different
    sequences might encode the same glyphs but we ignore that for now.  For
    example, the code point U+006E (the Latin lowercase "n") followed by
    U+0303 (the combining tilde) is defined by Unicode to be canonically
    equivalent to the single code point U+00F1 (the lowercase letter of the
    Spanish alphabet).  For now, users must normalize themselves to avoid
    element dups.

    This is what ECMAscript does also: "Two IdentifierName that are
    canonically equivalent according to the Unicode standard are not equal
    unless they are represented by the exact same sequence of code units (in
    other words, conforming ECMAScript implementations are only required to
    do bitwise comparison on IdentifierName values). The intent is that the
    incoming source text has been converted to normalised form C before it
    reaches the compiler." (ECMA-262 5.1 edition June 2011)

    See WL-2048 Add function for Unicode normalization
  */
  auto it = m_map.find(key);
  if (it != m_map.end() && !m_legacy_object) {
    it->second = std::move(value);
  } else {
    m_map.emplace(key, nullptr)->second = std::move(value);
  }
  return false;
}

#ifdef MYSQL_SERVER
bool Json_object::consume(Json_object_ptr other) {
  for (auto &other_member : other->m_map) {
    auto &key = other_member.first;
    auto &other_value = other_member.second;

    auto it = m_map.find(key);
    if (it == m_map.end()) {
      // The key does not exist in this object, so add the key/value pair.
      if (add_alias(key, std::move(other_value)))
        return true; /* purecov: inspected */
    } else {
      /*
        Oops. Duplicate key. Merge the values.
        This is where the recursion in JSON_MERGE() occurs.
      */
      it->second = merge_doms(std::move(it->second), std::move(other_value));
      if (it->second == nullptr) return true; /* purecov: inspected */
      it->second->set_parent(this);
    }
  }

  return false;
}

template <typename Key>
static Json_dom *json_object_get(const Json_dom *object MY_ATTRIBUTE((unused)),
                                 const Json_object_map &map, const Key &key) {
  const Json_object_map::const_iterator iter = map.find(key);

  if (iter != map.end()) {
    DBUG_ASSERT(iter->second->parent() == object);
    return iter->second.get();
  }

  return nullptr;
}

Json_dom *Json_object::get(const std::string &key) const {
  return json_object_get(this, m_map, key);
}

Json_dom *Json_object::get(const MYSQL_LEX_CSTRING &key) const {
  return json_object_get(this, m_map, key);
}

#endif  // ifdef MYSQL_SERVER

bool Json_object::remove(const std::string &key) {
  auto it = m_map.find(key);
  if (it == m_map.end()) return false;

  m_map.erase(it);
  return true;
}

size_t Json_object::cardinality() const { return m_map.size(); }

#ifdef MYSQL_SERVER
uint32 Json_object::depth() const {
  uint deepest_child = 0;

  for (Json_object_map::const_iterator iter = m_map.begin();
       iter != m_map.end(); ++iter) {
    deepest_child = std::max(deepest_child, iter->second->depth());
  }
  return 1 + deepest_child;
}
#endif  // ifdef MYSQL_SERVER

Json_dom_ptr Json_object::clone() const {
  Json_object_ptr o = create_dom_ptr<Json_object>(m_legacy_object);
  if (o == nullptr) return nullptr; /* purecov: inspected */

  for (const auto &member : m_map) {
    if (o->add_clone(member.first, member.second.get()))
      return nullptr; /* purecov: inspected */
  }

#ifdef __SUNPRO_CC
  return std::move(o);
#else
  return o;
#endif
}

bool Json_object::merge_patch(Json_object_ptr patch) {
  for (auto &member : patch->m_map) {
    // Remove the member if the value in the patch is the null literal.
    if (member.second->json_type() == enum_json_type::J_NULL) {
      remove(member.first);
      continue;
    }

    auto it = m_map.find(member.first);
    // See if the target has this member, add it if not.
    Json_dom_ptr &target = it != m_map.end()
                               ? it->second
                               : m_map.emplace(member.first, nullptr)->second;

    /*
      If the value in the patch is not an object and not the null
      literal, the new value is the patch.
    */
    if (member.second->json_type() != enum_json_type::J_OBJECT) {
      target = std::move(member.second);
      target->set_parent(this);
      continue;
    }

    /*
      If there is no target value, or if the target value is not an
      object, use an empty object as the target value.
    */
    if (target == nullptr || target->json_type() != enum_json_type::J_OBJECT)
      target = create_dom_ptr<Json_object>();

    // Recursively merge the target value with the patch.
    Json_object *target_obj = down_cast<Json_object *>(target.get());
    Json_object_ptr patch_obj(
        down_cast<Json_object *>(member.second.release()));
    if (target_obj == nullptr || target_obj->merge_patch(std::move(patch_obj)))
      return true; /* purecov: inspected */

    target->set_parent(this);
  }

  return false;
}

/**
  Compare two keys from a JSON object and determine whether or not the
  first key is less than the second key. key1 is considered less than
  key2 if

  a) key1 is shorter than key2, or if

  b) key1 and key2 have the same length, but different contents, and
  the first byte that differs has a smaller value in key1 than in key2

  Otherwise, key1 is not less than key2.

  @param key1 the first key to compare
  @param length1 the length of the first key
  @param key2 the second key to compare
  @param length2 the length of the second key
  @return true if key1 is considered less than key2, false otherwise
*/
static bool json_key_less(const char *key1, size_t length1, const char *key2,
                          size_t length2) {
  if (length1 != length2) return length1 < length2;
  return memcmp(key1, key2, length1) < 0;
}

bool Json_key_comparator::operator()(const std::string &key1,
                                     const std::string &key2) const {
  return json_key_less(key1.data(), key1.length(), key2.data(), key2.length());
}

bool Json_key_comparator::operator()(const MYSQL_LEX_CSTRING &key1,
                                     const std::string &key2) const {
  return json_key_less(key1.str, key1.length, key2.data(), key2.length());
}

bool Json_key_comparator::operator()(const std::string &key1,
                                     const MYSQL_LEX_CSTRING &key2) const {
  return json_key_less(key1.data(), key1.length(), key2.str, key2.length);
}

Json_array::Json_array()
    : m_v(Malloc_allocator<Json_dom *>(
#ifdef MYSQL_SERVER
          key_memory_JSON
#else
          PSI_NOT_INSTRUMENTED
#endif
          )) {
}

bool Json_array::consume(Json_array_ptr other) {
  // We've promised to delete other before returning.
  m_v.reserve(size() + other->size());
  for (auto &elt : other->m_v) {
    if (append_alias(std::move(elt))) return true; /* purecov: inspected */
  }

  return false;
}

bool Json_array::insert_alias(size_t index, Json_dom_ptr value) {
  if (!value) return true; /* purecov: inspected */
  /*
    Insert the value at the given index, or at the end of the array if the
    index points past the end of the array.
  */
  auto pos = m_v.begin() + std::min(m_v.size(), index);
  value->set_parent(this);
  m_v.emplace(pos, std::move(value));
  return false;
}

bool Json_array::remove(size_t index) {
  if (index < m_v.size()) {
    m_v.erase(m_v.begin() + index);
    return true;
  }

  return false;
}

#ifdef MYSQL_SERVER
uint32 Json_array::depth() const {
  uint deepest_child = 0;

  for (const auto &child : m_v) {
    deepest_child = std::max(deepest_child, child->depth());
  }
  return 1 + deepest_child;
}
#endif  // ifdef MYSQL_SERVER

Json_dom_ptr Json_array::clone() const {
  Json_array_ptr vv = create_dom_ptr<Json_array>();
  if (vv == nullptr) return nullptr; /* purecov: inspected */

  vv->m_v.reserve(size());
  for (const auto &child : m_v) {
    if (vv->append_clone(child.get())) return nullptr; /* purecov: inspected */
  }

#ifdef __SUNPRO_CC
  return std::move(vv);
#else
  return vv;
#endif
}

#ifdef MYSQL_SERVER
namespace {
class Cmp_json {
  const CHARSET_INFO *m_charset;

 public:
  Cmp_json(const CHARSET_INFO *cs = nullptr) : m_charset(cs) {}

  bool operator()(const Json_dom_ptr &a, Json_dom *b) const {
    return operator()(a.get(), b);
  }
  bool operator()(Json_dom *a, const Json_dom_ptr &b) const {
    return operator()(a, b.get());
  }
  bool operator()(const Json_dom_ptr &a, const Json_dom_ptr &b) const {
    return operator()(a.get(), b.get());
  }
  bool operator()(Json_dom *a, Json_dom *b) const {
    Json_wrapper wa(a, true);
    Json_wrapper wb(b, true);
    return wa.compare(wb, m_charset) < 0;
  }
};
}  // namespace

namespace {
class Eq_json {
  const CHARSET_INFO *m_charset;

 public:
  Eq_json(const CHARSET_INFO *cs) : m_charset(cs) {}
  bool operator()(const Json_dom_ptr &a, Json_dom *b) const {
    return operator()(a.get(), b);
  }
  bool operator()(Json_dom *a, const Json_dom_ptr &b) const {
    return operator()(a, b.get());
  }
  bool operator()(const Json_dom_ptr &a, const Json_dom_ptr &b) const {
    return operator()(a.get(), b.get());
  }
  bool operator()(Json_dom *a, Json_dom *b) const {
    Json_wrapper wa(a, true);
    Json_wrapper wb(b, true);
    return wa.compare(wb, m_charset) == 0;
  }
};
}  // namespace

void Json_array::sort(const CHARSET_INFO *cs) {
  std::sort(m_v.begin(), m_v.end(), Cmp_json(cs));
}

void Json_array::remove_duplicates(const CHARSET_INFO *cs) {
  sort(cs);
  m_v.erase(std::unique(m_v.begin(), m_v.end(), Eq_json(cs)), m_v.end());
}

bool Json_array::binary_search(Json_dom *val) {
  DBUG_ASSERT(std::is_sorted(m_v.begin(), m_v.end(), Cmp_json()));
  return std::binary_search(m_v.begin(), m_v.end(), val, Cmp_json());
}
#endif  // MYSQL_SERVER

/**
  Reserve space in a string buffer. If reallocation is needed,
  increase the size of the buffer exponentially.

  @param buffer the string buffer
  @param needed the number of bytes needed
  @return true on error, false on success
*/
static bool reserve(String *buffer, size_t needed) {
  return buffer->reserve(needed, buffer->length());
}

/**
  Escape a special character in a JSON string, as described in
  #double_quote(), and append it to a buffer.

  @param c    the special character to escape
  @param buf  the destination buffer

  @retval false on success
  @retval true on memory allocation failure
*/
static bool escape_character(char c, String *buf) {
  if (buf->append('\\')) return true; /* purecov: inspected */

  switch (c) {
    case '\b':
      return buf->append('b');
    case '\t':
      return buf->append('t');
    case '\n':
      return buf->append('n');
    case '\f':
      return buf->append('f');
    case '\r':
      return buf->append('r');
    case '"':
    case '\\':
      return buf->append(c);
  }

  /*
    Unprintable control character, use a hexadecimal number.
    The meaning of such a number determined by ISO/IEC 10646.
  */
  return buf->append("u00", 3) ||
         buf->append(_dig_vec_lower[(c & 0xf0) >> 4]) ||
         buf->append(_dig_vec_lower[(c & 0x0f)]);
}

/**
  Perform quoting on a JSON string to make an external representation
  of it. It wraps double quotes (text quotes) around the string (cptr)
  and also performs escaping according to the following table:
  <pre>
  @verbatim
  Common name     C-style  Original unescaped     Transformed to
                  escape   UTF-8 bytes            escape sequence
                  notation                        in UTF-8 bytes
  ---------------------------------------------------------------
  quote           \"       %x22                    %x5C %x22
  backslash       \\       %x5C                    %x5C %x5C
  backspace       \b       %x08                    %x5C %x62
  formfeed        \f       %x0C                    %x5C %x66
  linefeed        \n       %x0A                    %x5C %x6E
  carriage-return \r       %x0D                    %x5C %x72
  tab             \t       %x09                    %x5C %x74
  unicode         \uXXXX  A hex number in the      %x5C %x75
                          range of 00-1F,          followed by
                          except for the ones      4 hex digits
                          handled above (backspace,
                          formfeed, linefeed,
                          carriage-return,
                          and tab).
  ---------------------------------------------------------------
  @endverbatim
  </pre>

  @param[in] cptr pointer to string data
  @param[in] length the length of the string
  @param[in,out] buf the destination buffer
  @retval false on success
  @retval true on error
*/
bool double_quote(const char *cptr, size_t length, String *buf) {
  if (reserve(buf, length + 2) || buf->append('"'))
    return true; /* purecov: inspected */

  const char *const end = cptr + length;

  while (true) {
    /*
      We assume that most characters do not need escaping, so append
      segments of such characters with memcpy().
    */
    const char *next_special = std::find_if(cptr, end, [](const char c) {
      const unsigned char uc = static_cast<unsigned char>(c);
      return uc <= 0x1f || uc == '"' || uc == '\\';
    });

    if (buf->append(cptr, next_special - cptr))
      return true; /* purecov: inspected */

    cptr = next_special;

    if (cptr == end) break;

    // We've found a special character. Escape it.
    if (escape_character(*cptr++, buf)) return true; /* purecov: inspected */
  }

  DBUG_ASSERT(cptr == end);

  return buf->append('"');
}

Json_decimal::Json_decimal(const my_decimal &value)
    : Json_number(), m_dec(value) {}

#ifdef MYSQL_SERVER
int Json_decimal::binary_size() const {
  /*
    We need two bytes for the precision and the scale, plus whatever
    my_decimal2binary() needs.
  */
  return 2 + my_decimal_get_binary_size(m_dec.precision(), m_dec.frac);
}

bool Json_decimal::get_binary(char *dest) const {
  DBUG_ASSERT(binary_size() <= MAX_BINARY_SIZE);
  /*
    my_decimal2binary() loses the precision and the scale, so store them
    in the first two bytes.
  */
  dest[0] = static_cast<char>(m_dec.precision());
  dest[1] = static_cast<char>(m_dec.frac);
  // Then store the decimal value.
  return my_decimal2binary(E_DEC_ERROR, &m_dec, pointer_cast<uchar *>(dest) + 2,
                           m_dec.precision(), m_dec.frac) != E_DEC_OK;
}
#endif  // ifdef MYSQL_SERVER

bool Json_decimal::convert_from_binary(const char *bin, size_t len,
                                       my_decimal *dec) {
  // Expect at least two bytes, which contain precision and scale.
  bool error = (len < 2);

  if (!error) {
    int precision = bin[0];
    int scale = bin[1];

    // The decimal value is encoded after the two precision/scale bytes.
    size_t bin_size = my_decimal_get_binary_size(precision, scale);
    error =
        (bin_size != len - 2) ||
        (binary2my_decimal(E_DEC_ERROR, pointer_cast<const uchar *>(bin) + 2,
                           dec, precision, scale, true) != E_DEC_OK);
  }

  if (error)
    my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0)); /* purecov: inspected */

  return error;
}

enum_json_type Json_datetime::json_type() const {
  switch (m_field_type) {
    case MYSQL_TYPE_TIME:
      return enum_json_type::J_TIME;
    case MYSQL_TYPE_DATETIME:
      return enum_json_type::J_DATETIME;
    case MYSQL_TYPE_DATE:
      return enum_json_type::J_DATE;
    case MYSQL_TYPE_TIMESTAMP:
      return enum_json_type::J_TIMESTAMP;
    default:;
  }
  /* purecov: begin inspected */
  DBUG_ASSERT(false);
  return enum_json_type::J_NULL;
  /* purecov: end inspected */
}

Json_dom_ptr Json_datetime::clone() const {
  return create_dom_ptr<Json_datetime>(m_t, m_field_type);
}

#ifdef MYSQL_SERVER
void Json_datetime::to_packed(char *dest) const {
  longlong packed = TIME_to_longlong_packed(m_t);
  int8store(dest, packed);
}
#endif  // ifdef MYSQL_SERVER

void Json_datetime::from_packed(const char *from, enum_field_types ft,
                                MYSQL_TIME *to) {
  TIME_from_longlong_packed(to, ft, sint8korr(from));
}

#ifdef MYSQL_SERVER
void Json_datetime::from_packed_to_key(const char *from, enum_field_types ft,
                                       uchar *to, uint8 dec) {
  MYSQL_TIME ltime;
  TIME_from_longlong_packed(&ltime, ft, sint8korr(from));

  switch (ft) {
    case MYSQL_TYPE_TIME:
      my_time_packed_to_binary(sint8korr(from), to, dec);
      break;
    case MYSQL_TYPE_DATETIME:
      my_datetime_packed_to_binary(sint8korr(from), to, dec);
      break;
    case MYSQL_TYPE_DATE:
      TIME_from_longlong_date_packed(&ltime, sint8korr(from));
      my_date_to_binary(&ltime, to);
      break;
    case MYSQL_TYPE_TIMESTAMP: {
      struct timeval tm;
      int warnings = 0;
      TIME_from_longlong_datetime_packed(&ltime, sint8korr(from));
      datetime_with_no_zero_in_date_to_timeval(current_thd, &ltime, &tm,
                                               &warnings);
      // Assume that since the value was properly stored, there're no warnings
      DBUG_ASSERT(!warnings);
      my_timestamp_to_binary(&tm, to, dec);
      break;
    }
    default:
      break;
  }
}
#endif  // MYSQL_SERVER

Json_dom_ptr Json_opaque::clone() const {
  return create_dom_ptr<Json_opaque>(m_mytype, value(), size());
}

Json_wrapper_object_iterator::Json_wrapper_object_iterator(
    const Json_wrapper &wrapper, bool begin) {
  DBUG_ASSERT(wrapper.type() == enum_json_type::J_OBJECT);
  if (wrapper.is_dom()) {
    m_binary_value = nullptr;
    auto object = down_cast<const Json_object *>(wrapper.get_dom());
    m_iter = begin ? object->begin() : object->end();
  } else {
    m_binary_value = &wrapper.get_binary_value();
    m_current_element_index = begin ? 0 : m_binary_value->element_count();
  }
}

void Json_wrapper_object_iterator::initialize_current_member() {
  if (is_dom()) {
    m_current_member.first = {m_iter->first.c_str(), m_iter->first.length()};
    // The previous member is either empty or an alias, so there is nothing that
    // needs destruction. Just construct a new wrapper in its place.
    new (&m_current_member.second) Json_wrapper(m_iter->second.get());
    // DOM possibly owned by object and we don't want to make a clone
    m_current_member.second.set_alias();
  } else {
    DBUG_ASSERT(m_current_element_index < m_binary_value->element_count());
    json_binary::Value key = m_binary_value->key(m_current_element_index);
    m_current_member.first = {key.get_data(), key.get_data_length()};
    // There is no DOM to destruct in the previous member when iterating over a
    // binary value, so just construct a new wrapper in its place.
    new (&m_current_member.second)
        Json_wrapper(m_binary_value->element(m_current_element_index));
  }
  m_current_member_initialized = true;
}

Json_wrapper::Json_wrapper(Json_dom *dom_value, bool alias)
    : m_dom_value(dom_value),
      m_is_dom(true),
      m_legacy_json(LEGACY_JSON_DISABLED) {
  // Workaround for Solaris Studio, initialize in CTOR body
  m_dom_alias = alias;
  if (!dom_value) {
    m_dom_alias = true;  //!< no deallocation, make us empty
  }
}

Json_wrapper::Json_wrapper(Json_wrapper &&old) noexcept
    : m_is_dom(old.m_is_dom), m_legacy_json(old.m_legacy_json) {
  if (m_is_dom) {
    m_dom_alias = old.m_dom_alias;
    m_dom_value = old.m_dom_value;
    // Mark old as aliased. Any ownership is effectively transferred to this.
    old.set_alias();
  } else {
    m_value = std::move(old.m_value);
  }
}

Json_wrapper::Json_wrapper(const json_binary::Value &value)
    : m_value(value), m_is_dom(false), m_legacy_json(LEGACY_JSON_DISABLED) {}

Json_wrapper::Json_wrapper(const Json_wrapper &old)
    : m_is_dom(old.m_is_dom), m_legacy_json(old.m_legacy_json) {
  if (m_is_dom) {
    m_dom_alias = old.m_dom_alias;
    m_dom_value =
        m_dom_alias ? old.m_dom_value : old.m_dom_value->clone().release();
  } else {
    m_value = old.m_value;
  }
}

Json_wrapper::~Json_wrapper() {
  if (m_is_dom && !m_dom_alias) {
    // we own our own copy, so we are responsible for deallocation
    delete m_dom_value;
  }
}

#ifdef MYSQL_SERVER
/**
  Common implementation of move-assignment and copy-assignment for
  Json_wrapper. If @a from is an rvalue, its contents are moved into
  @a to, otherwise the contents are copied over.
*/
template <typename T>
static Json_wrapper &assign_json_wrapper(T &&from, Json_wrapper *to) {
  if (&from == to) {
    return *to;  // self assignment: no-op
  }

  // Deallocate DOM if needed.
  to->~Json_wrapper();

  // Move or copy the value into the destination.
  new (to) Json_wrapper(std::forward<T>(from));

  return *to;
}

Json_wrapper &Json_wrapper::operator=(const Json_wrapper &from) {
  return assign_json_wrapper(from, this);
}

Json_wrapper &Json_wrapper::operator=(Json_wrapper &&from) noexcept {
  return assign_json_wrapper(std::move(from), this);
}

Json_dom *Json_wrapper::to_dom(const THD *thd) {
  if (!m_is_dom) {
    // Build a DOM from the binary JSON value and
    // convert this wrapper to hold the DOM instead
    m_dom_value = Json_dom::parse(thd, m_value).release();
    m_is_dom = true;
    m_dom_alias = false;
  }

  return m_dom_value;
}

Json_dom_ptr Json_wrapper::clone_dom(const THD *thd) const {
  // If we already have a DOM, return a clone of it.
  if (m_is_dom) return m_dom_value ? m_dom_value->clone() : nullptr;

  // Otherwise, produce a new DOM tree from the binary representation.
  return Json_dom::parse(thd, m_value);
}

bool Json_wrapper::to_binary(const THD *thd, String *str) const {
  if (empty()) {
    /* purecov: begin inspected */
    my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
    return true;
    /* purecov: end */
  }

  if (is_legacy_json()) {
    const char *msg =
        "Serializing FB json values can lead to unexpected results";
    if (thd->variables.serialize_fb_json_raise_error) {
      my_printf_error(ER_INVALID_JSON_BINARY_DATA, "%s", MYF(0), msg);
      return true;
    } else {
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_INVALID_JSON_BINARY_DATA, "%s", msg);
    }
  }

  if (m_is_dom) return json_binary::serialize(thd, m_dom_value, str);

  return m_value.raw_binary(thd, str);
}
#endif  // ifdef MYSQL_SERVER

/**
  Possibly append a single quote to a buffer.
  @param[in,out] buffer receiving buffer
  @param[in] json_quoted whether or not a quote should be appended
  @return false if successful, true on error
*/
static inline bool single_quote(String *buffer, bool json_quoted) {
  return json_quoted && buffer->append('"');
}

/**
   Pretty-print a string to an evolving buffer, double-quoting if
   requested.

   @param[in] buffer the buffer to print to
   @param[in] json_quoted true if we should double-quote
   @param[in] data the string to print
   @param[in] length the string's length
   @return false on success, true on failure
*/
static int print_string(String *buffer, bool json_quoted, const char *data,
                        size_t length) {
  return json_quoted ? double_quote(data, length, buffer)
                     : buffer->append(data, length);
}

/**
  Helper function for wrapper_to_string() which adds a newline and indentation
  up to the specified level.

  @param[in,out] buffer  the buffer to write to
  @param[in]     level   how many nesting levels to add indentation for
  @retval false on success
  @retval true on error
*/
static bool newline_and_indent(String *buffer, size_t level) {
  // Append newline and two spaces per indentation level.
  return buffer->append('\n') ||
         buffer->fill(buffer->length() + level * 2, ' ');
}

/**
  Append a comma to separate elements in JSON arrays and objects.

  @param buffer the string buffer
  @param pretty true if pretty printing is enabled
  @param print_space true if blank space can be appended.
  @return true on error, false on success
*/
static bool append_comma(String *buffer, bool pretty, bool print_space) {
  // Append a comma followed by a blank space. If pretty printing is
  // enabled, a newline will be added in front of the next element, so
  // the blank space can be omitted.
  // Do not append blank space if its a legacy json format.
  return buffer->append(',') || (!pretty && print_space && buffer->append(' '));
}

/**
  Determine if blank spaces should be printed when printing json

  @param legacy_json enum governing print behavior of json
  @return true if blank space should be printed.
*/
static bool legacy_json_print_space(legacy_json_print_behavior legacy_json) {
  return legacy_json == LEGACY_JSON_DISABLED;
}

/**
  Determine if need to be printed in json_extract_value format

  @param legacy_json enum governing print behavior of json
  @return true if need to be in printed in json_extract_value format .
*/
static bool legacy_json_extract_value(legacy_json_print_behavior legacy_json) {
  return legacy_json == LEGACY_JSON_EXTRACT_VALUE;
}

/**
  Helper function which does all the heavy lifting for
  Json_wrapper::to_string(). It processes the Json_wrapper
  recursively. The depth parameter keeps track of the current nesting
  level. When it reaches JSON_DOCUMENT_MAX_DEPTH (see json_syntax_check.cc for
  definition), it gives up in order to avoid running out of stack space.

  @param[in]     wr          the value to convert to a string
  @param[in,out] buffer      the buffer to write to
  @param[in]     json_quoted quote strings if true
  @param[in]     pretty      add newlines and indentation if true
  @param[in]     func_name   the name of the calling function
  @param[in]     depth       the nesting level of @a wr
  @param[in]     legacy_json printing behavior for 5.6 fb json functions.

  @retval false on success
  @retval true on error
*/
static bool wrapper_to_string(const Json_wrapper &wr, String *buffer,
                              bool json_quoted, bool pretty,
                              const char *func_name, size_t depth,
                              legacy_json_print_behavior legacy_json) {
  enum_json_type type = wr.type();
  // Treat strings saved in opaque as plain json strings
  // @see val_json_func_field_subselect()
  if (type == enum_json_type::J_OPAQUE &&
      wr.field_type() == MYSQL_TYPE_VAR_STRING)
    type = enum_json_type::J_STRING;

  switch (type) {
    case enum_json_type::J_TIME:
    case enum_json_type::J_DATE:
    case enum_json_type::J_DATETIME:
    case enum_json_type::J_TIMESTAMP: {
      // Make sure the buffer has space for the datetime and the quotes.
      if (reserve(buffer, MAX_DATE_STRING_REP_LENGTH + 2))
        return true; /* purecov: inspected */
      MYSQL_TIME t;
      wr.get_datetime(&t);
      if (single_quote(buffer, json_quoted))
        return true; /* purecov: inspected */
      char *ptr = buffer->ptr() + buffer->length();
      const int size = my_TIME_to_str(t, ptr, 6);
      buffer->length(buffer->length() + size);
      if (single_quote(buffer, json_quoted))
        return true; /* purecov: inspected */
      break;
    }
    case enum_json_type::J_ARRAY: {
      if (check_json_depth(++depth)) return true;

      if (buffer->append('[')) return true; /* purecov: inspected */

      size_t array_len = wr.length();
      for (uint32 i = 0; i < array_len; ++i) {
        if (i > 0 &&
            append_comma(buffer, pretty, legacy_json_print_space(legacy_json)))
          return true; /* purecov: inspected */

        if (pretty && newline_and_indent(buffer, depth))
          return true; /* purecov: inspected */

        if (wrapper_to_string(wr[i], buffer, true, pretty, func_name, depth,
                              legacy_json))
          return true; /* purecov: inspected */
      }

      if (pretty && array_len > 0 && newline_and_indent(buffer, depth - 1))
        return true; /* purecov: inspected */

      if (buffer->append(']')) return true; /* purecov: inspected */

      break;
    }
    case enum_json_type::J_BOOLEAN:
      if (legacy_json_extract_value(legacy_json) && depth == 0) {
        if (wr.get_boolean() ? buffer->append(STRING_WITH_LEN("1"))
                             : buffer->append(STRING_WITH_LEN("0")))
          return true;
      } else {
        if (wr.get_boolean() ? buffer->append(STRING_WITH_LEN("true"))
                             : buffer->append(STRING_WITH_LEN("false")))
          return true; /* purecov: inspected */
      }
      break;
    case enum_json_type::J_DECIMAL: {
      int length = DECIMAL_MAX_STR_LENGTH + 1;
      if (reserve(buffer, length)) return true;
      char *ptr = buffer->ptr() + buffer->length();
      my_decimal m;
      if (wr.get_decimal_data(&m) || decimal2string(&m, ptr, &length))
        return true; /* purecov: inspected */
      buffer->length(buffer->length() + length);
      break;
    }
    case enum_json_type::J_DOUBLE: {
      if (reserve(buffer, MY_GCVT_MAX_FIELD_WIDTH + 1))
        return true; /* purecov: inspected */
      double d = wr.get_double();
      char *start = buffer->ptr() + buffer->length();
      size_t len = my_gcvt(d, MY_GCVT_ARG_DOUBLE, MY_GCVT_MAX_FIELD_WIDTH,
                           start, nullptr);
      buffer->length(buffer->length() + len);
      /*
        my_gcvt() doesn't preserve trailing zeros after the decimal point,
        so for floating-point values with no fractional part we get 1
        instead of 1.0. We want the string representation to preserve the
        information that this is a floating-point number, so append ".0" if
        my_gcvt() neither used scientific notation nor included a decimal
        point. This makes it distinguishable from integers.
      */
      if (std::none_of(start, start + len,
                       [](char c) { return c == '.' || c == 'e'; }) &&
          (buffer->append('.') || buffer->append('0')))
        return true; /* purecov: inspected */
      break;
    }
    case enum_json_type::J_INT: {
      if (buffer->append_longlong(wr.get_int()))
        return true; /* purecov: inspected */
      break;
    }
    case enum_json_type::J_NULL:
      if (legacy_json_extract_value(legacy_json)) {
        // Return empty buffer that will be translated as null
        return false;
      } else {
        if (buffer->append(STRING_WITH_LEN("null")))
          return true; /* purecov: inspected */
      }
      break;
    case enum_json_type::J_OBJECT: {
      if (check_json_depth(++depth)) return true;

      if (buffer->append('{')) return true; /* purecov: inspected */

      bool first = true;
      for (const auto &iter : Json_object_wrapper(wr)) {
        if (!first &&
            append_comma(buffer, pretty, legacy_json_print_space(legacy_json)))
          return true; /* purecov: inspected */

        first = false;

        if (pretty && newline_and_indent(buffer, depth))
          return true; /* purecov: inspected */

        const MYSQL_LEX_CSTRING &key = iter.first;
        if (print_string(buffer, true, key.str, key.length) ||
            buffer->append(':') ||
            // Do not append blank space if the contained json is created by
            // 5.6 json functions.
            (legacy_json_print_space(legacy_json) && buffer->append(' ')) ||
            wrapper_to_string(iter.second, buffer, true, pretty, func_name,
                              depth, legacy_json))
          return true; /* purecov: inspected */
      }

      if (pretty && wr.length() > 0 && newline_and_indent(buffer, depth - 1))
        return true; /* purecov: inspected */

      if (buffer->append('}')) return true; /* purecov: inspected */

      break;
    }
    case enum_json_type::J_OPAQUE: {
      if (wr.get_data_length() > base64_encode_max_arg_length()) {
        /* purecov: begin inspected */
        my_error(ER_INTERNAL_ERROR, MYF(0),
                 "JSON: could not decode opaque data");
        return true;
        /* purecov: end */
      }

      const size_t needed = static_cast<size_t>(
          base64_needed_encoded_length(wr.get_data_length()));

      // base64:typeXX:<binary data>
      StringBuffer<STRING_BUFFER_USUAL_SIZE> base64_buffer;
      if (base64_buffer.append(STRING_WITH_LEN("base64:type")) ||
          base64_buffer.append_ulonglong(wr.field_type()) ||
          base64_buffer.append(':') || base64_buffer.reserve(needed) ||
          base64_encode(wr.get_data(), wr.get_data_length(),
                        &base64_buffer[base64_buffer.length()]))
        return true; /* purecov: inspected */

      base64_buffer.length(base64_buffer.length() + needed -
                           1);  // drop zero terminator space

      // Append the encoded string to the buffer. Quote and escape it
      // first if json_quoted is true. The encoded string may contain
      // special characters, specifically newline characters.
      if (json_quoted ? double_quote(base64_buffer.ptr(),
                                     base64_buffer.length(), buffer)
                      : buffer->append(base64_buffer))
        return true; /* purecov: inspected */
      break;
    }
    case enum_json_type::J_STRING: {
      const char *data = wr.get_data();
      size_t length = wr.get_data_length();

      if (print_string(buffer, json_quoted, data, length))
        return true; /* purecov: inspected */
      break;
    }
    case enum_json_type::J_UINT: {
      if (buffer->append_ulonglong(wr.get_uint()))
        return true; /* purecov: inspected */
      break;
    }
    default:
      /* purecov: begin inspected */
      DBUG_PRINT("info", ("JSON wrapper: unexpected type %d",
                          static_cast<int>(wr.type())));

      DBUG_ASSERT(false);
      my_error(ER_INTERNAL_ERROR, MYF(0), "JSON wrapper: unexpected type");
      return true;
      /* purecov: end inspected */
  }

#ifdef MYSQL_SERVER
  if (buffer->length() > current_thd->variables.max_allowed_packet) {
    push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                        ER_WARN_ALLOWED_PACKET_OVERFLOWED,
                        ER_THD(current_thd, ER_WARN_ALLOWED_PACKET_OVERFLOWED),
                        func_name, current_thd->variables.max_allowed_packet);
    return true;
  }
#endif  // ifdef MYSQL_SERVER

  return false;
}

bool Json_wrapper::to_string(String *buffer, bool json_quoted,
                             const char *func_name) const {
  buffer->set_charset(&my_charset_utf8mb4_bin);
  return wrapper_to_string(*this, buffer, json_quoted, false, func_name, 0,
                           m_legacy_json);
}

bool Json_wrapper::to_pretty_string(String *buffer,
                                    const char *func_name) const {
  buffer->set_charset(&my_charset_utf8mb4_bin);
  return wrapper_to_string(*this, buffer, true, true, func_name, 0,
                           m_legacy_json);
}

void Json_wrapper::dbug_print(
    const char *message MY_ATTRIBUTE((unused))) const {
#ifndef DBUG_OFF
  StringBuffer<STRING_BUFFER_USUAL_SIZE> buf;
  if (to_string(&buf, false, "Json_wrapper::dbug_print"))
    DBUG_ASSERT(0); /* purecov: inspected */  // OOM
  DBUG_PRINT("info", ("%s[length=%zu]%s%.*s", message, buf.length(),
                      message[0] ? ": " : "", static_cast<int>(buf.length()),
                      buf.ptr()));
#endif
}

enum_json_type Json_wrapper::type() const {
  if (empty()) {
    return enum_json_type::J_ERROR;
  }

  if (m_is_dom) {
    return m_dom_value->json_type();
  }

  json_binary::Value::enum_type typ = m_value.type();

  if (typ == json_binary::Value::OPAQUE) {
    const enum_field_types ftyp = m_value.field_type();

    switch (ftyp) {
      case MYSQL_TYPE_NEWDECIMAL:
        return enum_json_type::J_DECIMAL;
      case MYSQL_TYPE_DATETIME:
        return enum_json_type::J_DATETIME;
      case MYSQL_TYPE_DATE:
        return enum_json_type::J_DATE;
      case MYSQL_TYPE_TIME:
        return enum_json_type::J_TIME;
      case MYSQL_TYPE_TIMESTAMP:
        return enum_json_type::J_TIMESTAMP;
      default:;
        // ok, fall through
    }
  }

  return bjson2json(typ);
}

enum_field_types Json_wrapper::field_type() const {
  if (m_is_dom) {
    return down_cast<Json_opaque *>(m_dom_value)->type();
  }

  return m_value.field_type();
}

#ifdef MYSQL_SERVER
Json_wrapper Json_wrapper::lookup(const MYSQL_LEX_CSTRING &key) const {
  DBUG_ASSERT(type() == enum_json_type::J_OBJECT);
  if (m_is_dom) {
    const Json_object *object = down_cast<const Json_object *>(m_dom_value);
    Json_wrapper wr(object->get(key));
    wr.set_alias();  // wr doesn't own the supplied DOM: part of array DOM
    return wr;
  }

  return Json_wrapper(m_value.lookup(key.str, key.length));
}
#endif  // ifdef MYSQL_SERVER

Json_wrapper Json_wrapper::operator[](size_t index) const {
  // Non-arrays can be accessed only as the first element of array
  DBUG_ASSERT(type() == enum_json_type::J_ARRAY || index == 0);
  if (type() != enum_json_type::J_ARRAY) return *this;
  if (m_is_dom) {
    const Json_array *o = down_cast<const Json_array *>(m_dom_value);
    Json_wrapper wr((*o)[index]);
    wr.set_alias();  // wr doesn't own the supplied DOM: part of array DOM
    return wr;
  }

  return Json_wrapper(m_value.element(index));
}

const char *Json_wrapper::get_data() const {
  if (m_is_dom) {
    return type() == enum_json_type::J_STRING
               ? down_cast<Json_string *>(m_dom_value)->value().c_str()
               : down_cast<Json_opaque *>(m_dom_value)->value();
  }

  return m_value.get_data();
}

size_t Json_wrapper::get_data_length() const {
  if (m_is_dom) {
    return type() == enum_json_type::J_STRING
               ? down_cast<Json_string *>(m_dom_value)->size()
               : down_cast<Json_opaque *>(m_dom_value)->size();
  }

  return m_value.get_data_length();
}

bool Json_wrapper::get_decimal_data(my_decimal *d) const {
  if (m_is_dom) {
    *d = *down_cast<Json_decimal *>(m_dom_value)->value();
    return false;
  }

  return Json_decimal::convert_from_binary(m_value.get_data(),
                                           m_value.get_data_length(), d);
}

double Json_wrapper::get_double() const {
  if (m_is_dom) {
    return down_cast<Json_double *>(m_dom_value)->value();
  }

  return m_value.get_double();
}

longlong Json_wrapper::get_int() const {
  if (m_is_dom) {
    return down_cast<Json_int *>(m_dom_value)->value();
  }

  return m_value.get_int64();
}

ulonglong Json_wrapper::get_uint() const {
  if (m_is_dom) {
    return down_cast<Json_uint *>(m_dom_value)->value();
  }

  return m_value.get_uint64();
}

void Json_wrapper::get_datetime(MYSQL_TIME *t) const {
  if (m_is_dom) {
    *t = *down_cast<Json_datetime *>(m_dom_value)->value();
  } else {
    Json_datetime::from_packed(m_value.get_data(), m_value.field_type(), t);
  }
}

#ifdef MYSQL_SERVER
const char *Json_wrapper::get_datetime_packed(char *buffer) const {
  if (m_is_dom) {
    down_cast<Json_datetime *>(m_dom_value)->to_packed(buffer);
    return buffer;
  }

  DBUG_ASSERT(m_value.get_data_length() == Json_datetime::PACKED_SIZE);
  return m_value.get_data();
}
#endif  // ifdef MYSQL_SERVER

bool Json_wrapper::get_boolean() const {
  if (m_is_dom) {
    return down_cast<Json_boolean *>(m_dom_value)->value();
  }

  return m_value.type() == json_binary::Value::LITERAL_TRUE;
}

#ifdef MYSQL_SERVER
Json_path Json_dom::get_location() {
  if (m_parent == nullptr) {
    Json_path result;
    return result;
  }

  Json_path result = m_parent->get_location();

  if (m_parent->json_type() == enum_json_type::J_OBJECT) {
    Json_object *object = down_cast<Json_object *>(m_parent);
    auto it =
        std::find_if(object->begin(), object->end(), Json_child_equal{this});
    DBUG_ASSERT(it != object->end());
    result.append(Json_path_leg(it->first));
  } else {
    DBUG_ASSERT(m_parent->json_type() == enum_json_type::J_ARRAY);
    Json_array *array = down_cast<Json_array *>(m_parent);
    auto it =
        std::find_if(array->begin(), array->end(), Json_child_equal{this});
    DBUG_ASSERT(it != array->end());
    size_t idx = it - array->begin();
    result.append(Json_path_leg(idx));
  }

  return result;
}

bool Json_dom::seek(const Json_seekable_path &path, size_t legs,
                    Json_dom_vector *hits, bool auto_wrap, bool only_need_one) {
  const auto begin = path.begin();
  const auto end = begin + legs;

  Json_dom_vector duplicates(key_memory_JSON);
  Json_dom_vector *dup_vector =
      path_gives_duplicates(begin, end, auto_wrap) ? &duplicates : nullptr;

  return find_child_doms(this, begin, end, auto_wrap, only_need_one, dup_vector,
                         hits);
}

namespace {

/**
  Input and output parameters to #seek_no_dup_elimination that remain
  constant in recursive calls.
*/
struct Json_seek_params {
  Json_seek_params(const Json_path_iterator &last_leg,
                   Json_wrapper_vector *hits, bool auto_wrap,
                   bool only_need_one)
      : m_last_leg(last_leg),
        m_hits(hits),
        m_auto_wrap(auto_wrap),
        m_only_need_one(only_need_one),
        m_is_done(false) {}

  /// Iterator pointing just after the last path leg to search for.
  Json_path_iterator m_last_leg;
  /// Pointer to the result vector.
  Json_wrapper_vector *m_hits;
  /// Should auto-wrapping be used in this search?
  bool m_auto_wrap;
  /// Should the search stop as soon as a match is found?
  bool m_only_need_one;
  /// Should the search stop now?
  mutable bool m_is_done;
};

}  // namespace

static bool seek_no_dup_elimination(const json_binary::Value &value,
                                    const Json_path_iterator &current_leg,
                                    const Json_seek_params &params);
static std::function<bool(const json_binary::Value &,
                          const Json_path_iterator &, const Json_seek_params &)>
get_seek_func(const Json_path_iterator &it, const Json_seek_params &params);

/**
  Helper function for #seek_no_dup_elimination which handles
  #jpl_member path legs.
*/
static bool seek_member(const json_binary::Value &value,
                        const Json_path_iterator &current_leg,
                        const Json_seek_params &params) {
  DBUG_ASSERT((*current_leg)->get_type() == jpl_member);

  if (!value.is_object() || value.element_count() == 0) return false;

  size_t pos = value.lookup_index((*current_leg)->get_member_name());
  if (pos == value.element_count()) return false;

  return seek_no_dup_elimination(value.element(pos), current_leg + 1, params);
}

/**
  Helper function for #seek_no_dup_elimination which handles
  #jpl_member_wildcard path legs.
*/
static bool seek_member_wildcard(const json_binary::Value &value,
                                 const Json_path_iterator &current_leg,
                                 const Json_seek_params &params) {
  DBUG_ASSERT((*current_leg)->get_type() == jpl_member_wildcard);

  if (!value.is_object() || value.element_count() == 0) return false;

  const auto next_leg = current_leg + 1;
  const auto seek = get_seek_func(next_leg, params);
  for (size_t i = 0, size = value.element_count(); i < size; ++i) {
    if (params.m_is_done) return false;

    if (seek(value.element(i), next_leg, params))
      return true; /* purecov: inspected */
  }
  return false;
}

/**
  Helper function for #seek_no_dup_elimination which handles
  #jpl_array_cell path legs.
*/
static bool seek_array_cell(const json_binary::Value &value,
                            const Json_path_iterator &current_leg,
                            const Json_seek_params &params) {
  DBUG_ASSERT((*current_leg)->get_type() == jpl_array_cell);

  if (value.is_array()) {
    const Json_array_index idx =
        (*current_leg)->first_array_index(value.element_count());
    return idx.within_bounds() &&
           seek_no_dup_elimination(value.element(idx.position()),
                                   current_leg + 1, params);
  }

  // Possibly auto-wrap non-arrays.
  return params.m_auto_wrap && (*current_leg)->is_autowrap() &&
         seek_no_dup_elimination(value, current_leg + 1, params);
}

/**
  Helper function for #seek_no_dup_elimination which handles
  #jpl_array_cell_wildcard and #jpl_array_range path legs.
*/
static bool seek_array_range(const json_binary::Value &value,
                             const Json_path_iterator &current_leg,
                             const Json_seek_params &params) {
  DBUG_ASSERT((*current_leg)->get_type() == jpl_array_cell_wildcard ||
              (*current_leg)->get_type() == jpl_array_range);

  if (!value.is_array()) {
    // Possibly auto-wrap non-arrays.
    if (params.m_auto_wrap && (*current_leg)->is_autowrap())
      return seek_no_dup_elimination(value, current_leg + 1, params);
    return false;
  }

  const auto range = (*current_leg)->get_array_range(value.element_count());
  if (range.m_begin == range.m_end) return false;

  const auto next_leg = current_leg + 1;
  const auto seek = get_seek_func(next_leg, params);
  for (size_t i = range.m_begin; i < range.m_end; ++i) {
    if (params.m_is_done) return false;

    if (seek(value.element(i), next_leg, params))
      return true; /* purecov: inspected */
  }
  return false;
}

/**
  Helper function for #seek_no_dup_elimination which handles
  #jpl_ellipsis path legs.
*/
static bool seek_ellipsis(const json_binary::Value &value,
                          const Json_path_iterator &current_leg,
                          const Json_seek_params &params) {
  DBUG_ASSERT((*current_leg)->get_type() == jpl_ellipsis);
  const auto next_leg = current_leg + 1;
  const auto seek = get_seek_func(next_leg, params);
  bool error = false;
  json_binary::for_each_node(value,
                             [&](const json_binary::Value &child) -> bool {
                               error = seek(child, next_leg, params);
                               return error || params.m_is_done;
                             });
  return error;
}

/**
  Helper function for #seek_no_dup_elimination which handles
  the end of the path.
*/
static bool seek_end(const json_binary::Value &value,
                     const Json_path_iterator &current_leg,
                     const Json_seek_params &params) {
  DBUG_ASSERT(current_leg == params.m_last_leg);
  (void)current_leg;  // unused in non-debug builds
  params.m_is_done = params.m_only_need_one;
  // An empty path matches the root. Add it to the result vector.
  return params.m_hits->emplace_back(value);
}

/**
  Finds all of the JSON sub-documents which match the path expression.
  Puts the matches on an evolving vector of results. This is a
  fast-track method for paths which don't need duplicate elimination
  due to multiple ellipses or the combination of ellipses and
  auto-wrapping. Those paths can take advantage of the efficient
  positioning logic of json_binary::Value.

  @param[in] value the JSON value to search
  @param[in] current_leg iterator to the first path leg to look at.
             Usually called on the root document with an iterator pointing to
             the beginning of the path, and then incremented in recursive calls
             within this function.
  @param[in,out] params  the seek parameters

  @returns false if there was no error, otherwise true on error
*/
static bool seek_no_dup_elimination(const json_binary::Value &value,
                                    const Json_path_iterator &current_leg,
                                    const Json_seek_params &params) {
  return get_seek_func(current_leg, params)(value, current_leg, params);
}

/**
  Get which helper function of #seek_no_dup_elimination() should be
  used for this path leg.
*/
static std::function<bool(const json_binary::Value &,
                          const Json_path_iterator &, const Json_seek_params &)>
get_seek_func(const Json_path_iterator &it, const Json_seek_params &params) {
  using Val = const json_binary::Value &;
  using It = const Json_path_iterator &;
  using Param = const Json_seek_params &;

  if (it != params.m_last_leg) {
    switch ((*it)->get_type()) {
      case jpl_member:
        return [](Val v, It i, Param p) { return seek_member(v, i, p); };
      case jpl_array_cell:
        return [](Val v, It i, Param p) { return seek_array_cell(v, i, p); };
      case jpl_array_range:
      case jpl_array_cell_wildcard:
        return [](Val v, It i, Param p) { return seek_array_range(v, i, p); };
      case jpl_member_wildcard:
        return
            [](Val v, It i, Param p) { return seek_member_wildcard(v, i, p); };
      case jpl_ellipsis:
        return [](Val v, It i, Param p) { return seek_ellipsis(v, i, p); };
    }
  }

  return [](Val v, It i, Param p) { return seek_end(v, i, p); };
}

bool Json_wrapper::seek(const Json_seekable_path &path, size_t legs,
                        Json_wrapper_vector *hits, bool auto_wrap,
                        bool only_need_one) {
  DBUG_ASSERT(!empty());

  const auto begin = path.begin();
  const auto end = begin + legs;

  /*
    If the wrapper wraps a DOM, let's call Json_dom::seek() directly,
    to avoid the overhead of going through the Json_wrapper interface.

    If ellipsis and auto-wrapping are used in a way that requires
    duplicate elimination, convert to DOM since duplicate detection is
    difficult on binary values.
  */
  if (is_dom() || path_gives_duplicates(begin, end, auto_wrap)) {
    Json_dom *dom = to_dom(current_thd);
    if (dom == nullptr) return true; /* purecov: inspected */

    Json_dom_vector dom_hits(key_memory_JSON);
    if (dom->seek(path, legs, &dom_hits, auto_wrap, only_need_one))
      return true; /* purecov: inspected */

    for (const Json_dom *hit : dom_hits) {
      if (hits->emplace_back(hit->clone()) || hits->back().empty())
        return true; /* purecov: inspected */
    }

    return false;
  }

  return seek_no_dup_elimination(
      m_value, begin, Json_seek_params(end, hits, auto_wrap, only_need_one));
}
#endif  // ifdef MYSQL_SERVER

size_t Json_wrapper::length() const {
  if (empty()) {
    return 0;
  }

  if (m_is_dom) {
    switch (m_dom_value->json_type()) {
      case enum_json_type::J_ARRAY:
        return down_cast<Json_array *>(m_dom_value)->size();
      case enum_json_type::J_OBJECT:
        return down_cast<Json_object *>(m_dom_value)->cardinality();
      default:
        return 1;
    }
  }

  switch (m_value.type()) {
    case json_binary::Value::ARRAY:
    case json_binary::Value::OBJECT:
      return m_value.element_count();
    default:
      return 1;
  }
}

#ifdef MYSQL_SERVER
/**
  Compare a decimal value to a double by converting the double to a
  decimal.
  @param a the decimal value
  @param b the double value
  @return -1 if a is less than b,
          0 if a is equal to b,
          1 if a is greater than b
*/
static int compare_json_decimal_double(const my_decimal &a, double b) {
  /*
    First check the sign of the two values. If they differ, the
    negative value is the smaller one.
  */
  const bool a_is_zero = my_decimal_is_zero(&a);
  const bool a_is_negative = a.sign() && !a_is_zero;
  const bool b_is_negative = (b < 0);
  if (a_is_negative != b_is_negative) return a_is_negative ? -1 : 1;

  // Both arguments have the same sign. Compare their values.

  const bool b_is_zero = b == 0;
  if (a_is_zero)
    // b is non-negative, so it is either equal to or greater than a.
    return b_is_zero ? 0 : -1;

  if (b_is_zero)
    // a is positive and non-zero, so it is greater than b.
    return 1;

  my_decimal b_dec;
  switch (double2decimal(b, &b_dec)) {
    case E_DEC_OK:
      return my_decimal_cmp(&a, &b_dec);
    case E_DEC_OVERFLOW:
      /*
        b is too big to fit in a DECIMAL, so it must have a
        larger absolute value than a, which is a DECIMAL.
      */
      return a_is_negative ? 1 : -1;
    case E_DEC_TRUNCATED:
      /*
        b was truncated to fit in a DECIMAL, which means that b_dec is
        closer to zero than b.
      */
      {
        int cmp = my_decimal_cmp(&a, &b_dec);

        /*
          If the truncated b_dec is equal to a, a must be closer to zero
          than b.
        */
        if (cmp == 0) return a_is_negative ? 1 : -1;

        return cmp;
      }
    default:
      /*
        double2decimal() is not supposed to return anything other than
        E_DEC_OK, E_DEC_OVERFLOW or E_DEC_TRUNCATED, so this should
        never happen.
      */
      DBUG_ASSERT(false); /* purecov: inspected */
      return 1;           /* purecov: inspected */
  }
}

/**
  Compare a decimal value to a signed integer by converting the
  integer to a decimal.
  @param a the decimal value
  @param b the signed integer value
  @return -1 if a is less than b,
          0 if a is equal to b,
          1 if a is greater than b
*/
static int compare_json_decimal_int(const my_decimal &a, longlong b) {
  if (my_decimal_is_zero(&a)) return (b == 0) ? 0 : (b > 0 ? -1 : 1);

  if (b == 0) return a.sign() ? -1 : 1;

  // Different signs. The negative number is the smallest one.
  if (a.sign() != (b < 0)) return (b < 0) ? 1 : -1;

  // Couldn't tell the difference by looking at the signs. Compare as decimals.
  my_decimal b_dec;
  longlong2decimal(b, &b_dec);
  return my_decimal_cmp(&a, &b_dec);
}

/**
  Compare a decimal value to an unsigned integer by converting the
  integer to a decimal.
  @param a the decimal value
  @param b the unsigned integer value
  @return -1 if a is less than b,
          0 if a is equal to b,
          1 if a is greater than b
*/
static int compare_json_decimal_uint(const my_decimal &a, ulonglong b) {
  if (my_decimal_is_zero(&a)) return (b == 0) ? 0 : -1;

  // If a is negative, it must be smaller than the unsigned value b.
  if (a.sign()) return -1;

  // When we get here, we know that a is greater than zero.
  if (b == 0) return 1;

  // Couldn't tell the difference by looking at the signs. Compare as decimals.
  my_decimal b_dec;
  ulonglong2decimal(b, &b_dec);
  return my_decimal_cmp(&a, &b_dec);
}

/**
  Compare a JSON double to a JSON signed integer.
  @param a the double value
  @param b the integer value
  @return -1 if a is less than b,
          0 if a is equal to b,
          1 if a is greater than b
*/
static int compare_json_double_int(double a, longlong b) {
  double b_double = static_cast<double>(b);
  if (a < b_double) return -1;
  if (a > b_double) return 1;

  /*
    The two numbers were equal when compared as double. Since
    conversion from longlong to double isn't lossless, they could
    still be different. Convert to decimal to compare their exact
    values.
  */
  my_decimal b_dec;
  longlong2decimal(b, &b_dec);
  return -compare_json_decimal_double(b_dec, a);
}

/**
  Compare a JSON double to a JSON unsigned integer.
  @param a the double value
  @param b the unsigned integer value
  @return -1 if a is less than b,
          0 if a is equal to b,
          1 if a is greater than b
*/
static int compare_json_double_uint(double a, ulonglong b) {
  double b_double = ulonglong2double(b);
  if (a < b_double) return -1;
  if (a > b_double) return 1;

  /*
    The two numbers were equal when compared as double. Since
    conversion from longlong to double isn't lossless, they could
    still be different. Convert to decimal to compare their exact
    values.
  */
  my_decimal b_dec;
  ulonglong2decimal(b, &b_dec);
  return -compare_json_decimal_double(b_dec, a);
}

/**
  Compare a JSON signed integer to a JSON unsigned integer.
  @param a the signed integer
  @param b the unsigned integer
  @return -1 if a is less than b,
          0 if a is equal to b,
          1 if a is greater than b
*/
static int compare_json_int_uint(longlong a, ulonglong b) {
  // All negative values are less than the unsigned value b.
  if (a < 0) return -1;

  // If a is not negative, it is safe to cast it to ulonglong.
  return compare_numbers(static_cast<ulonglong>(a), b);
}

/**
  Compare the contents of two strings in a JSON value. The strings
  could be either JSON string scalars encoded in utf8mb4, or binary
  strings from JSON opaque scalars. In either case they are compared
  byte by byte.

  @param str1 the first string
  @param str1_len the length of str1
  @param str2 the second string
  @param str2_len the length of str2
  @param cs       If given, this charset will be used for comparison

  @retval -1 if str1 is less than str2,
  @retval 0 if str1 is equal to str2,
  @retval 1 if str1 is greater than str2
*/
static int compare_json_strings(const char *str1, size_t str1_len,
                                const char *str2, size_t str2_len,
                                const CHARSET_INFO *cs = nullptr) {
  if (cs != nullptr && cs != &my_charset_bin) {
    // Charsets with padding aren't supported
    DBUG_ASSERT(cs->pad_attribute == NO_PAD);
    return cs->coll->strnncollsp(cs, (const uchar *)str1, str1_len,
                                 (const uchar *)str2, str2_len);
  }
  int cmp = memcmp(str1, str2, std::min(str1_len, str2_len));
  if (cmp != 0) return cmp;
  return compare_numbers(str1_len, str2_len);
}

/// The number of enumerators in the enum_json_type enum.
static constexpr int num_json_types =
    static_cast<int>(enum_json_type::J_ERROR) + 1;

static bool legacy_value_type_number(enum_json_type type) {
  switch (type) {
    case enum_json_type::J_BOOLEAN:
    case enum_json_type::J_UINT:
    case enum_json_type::J_INT:
    case enum_json_type::J_DOUBLE:
    case enum_json_type::J_DECIMAL:
      return true;
    default:
      return false;
  }
}

/**
  The following matrix tells how two JSON values should be compared
  based on their types. If type_comparison[type_of_a][type_of_b] is
  -1, it means that a is smaller than b. If it is 1, it means that a
  is greater than b. If it is 0, it means it cannot be determined
  which value is the greater one just by looking at the types.
*/
// clang-format off
static constexpr int type_comparison[num_json_types][num_json_types] = {
  /* NULL */      {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  /* DECIMAL */   {1,  0,  0,  0,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  /* INT */       {1,  0,  0,  0,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  /* UINT */      {1,  0,  0,  0,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  /* DOUBLE */    {1,  0,  0,  0,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  /* STRING */    {1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  /* OBJECT */    {1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1},
  /* ARRAY */     {1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1},
  /* BOOLEAN */   {1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1},
  /* DATE */      {1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1},
  /* TIME */      {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1},
  /* DATETIME */  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0, -1, -1},
  /* TIMESTAMP */ {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0, -1, -1},
  /* OPAQUE */    {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1},
  /* ERROR */     {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
};
// clang-format on

int Json_wrapper::compare(const Json_wrapper &other, const CHARSET_INFO *cs,
                          bool legacy_val_cmp) const {
  const enum_json_type this_type = type();
  const enum_json_type other_type = other.type();

  DBUG_ASSERT(this_type != enum_json_type::J_ERROR);
  DBUG_ASSERT(other_type != enum_json_type::J_ERROR);

  if (use_legacy_json_extract_comparison(other)) {
    return legacy_json_extract_compare(other, cs);
  }

  // Check if the type tells us which value is bigger.
  int type_cmp = type_comparison[static_cast<int>(this_type)]
                                [static_cast<int>(other_type)];
  if (legacy_val_cmp && legacy_value_type_number(this_type) &&
      legacy_value_type_number(other_type)) {
    type_cmp = 0;
  }
  if (type_cmp != 0) return type_cmp;

  // Same or similar type. Go on and inspect the values.

  switch (this_type) {
    case enum_json_type::J_ARRAY:
      /*
        Two arrays are equal if they have the same length, and all
        elements in one array are equal to the corresponding elements in
        the other array.

        The array that has the smallest value on the first position that
        contains different values in the two arrays, is considered
        smaller than the other array. If the two arrays are of different
        size, and all values in the shorter array are equal to the
        corresponding values in the longer array, the shorter array is
        considered smaller.
      */
      {
        const size_t size_a = length();
        const size_t size_b = other.length();
        const size_t min_size = std::min(size_a, size_b);
        for (size_t i = 0; i < min_size; i++) {
          int cmp = (*this)[i].compare(other[i], cs);
          if (cmp != 0) return cmp;
        }
        return compare_numbers(size_a, size_b);
      }
    case enum_json_type::J_OBJECT:
      /*
        An object is equal to another object if they have the same set
        of keys, and all values in one objects are equal to the values
        associated with the same key in the other object.
      */
      {
        /*
          If their sizes are different, the object with the smallest
          number of elements is smaller than the other object.
        */
        int cmp = compare_numbers(length(), other.length());
        if (cmp != 0) return cmp;

        /*
          Otherwise, compare each key/value pair in the two objects.
          Return on the first difference that is found.
        */
        Json_object_wrapper this_object(*this);
        Json_object_wrapper other_object(other);
        Json_object_wrapper::const_iterator it1 = this_object.begin();
        Json_object_wrapper::const_iterator it2 = other_object.begin();
        for (; it1 != this_object.end(); ++it1, ++it2) {
          const MYSQL_LEX_CSTRING &key1 = it1->first;
          const MYSQL_LEX_CSTRING &key2 = it2->first;

          // Compare the keys of the two members.
          cmp = compare_json_strings(key1.str, key1.length, key2.str,
                                     key2.length);
          if (cmp != 0) return cmp;

          // Compare the values of the two members.
          cmp = it1->second.compare(it2->second, cs);
          if (cmp != 0) return cmp;
        }

        DBUG_ASSERT(it1 == this_object.end());
        DBUG_ASSERT(it2 == other_object.end());

        // No differences found. The two objects must be equal.
        return 0;
      }
    case enum_json_type::J_STRING:
      if (other_type == enum_json_type::J_OPAQUE) {
        // String might be stored as J_OPAQUE, check this case
        if (other.field_type() == MYSQL_TYPE_VARCHAR ||
            other.field_type() == MYSQL_TYPE_VAR_STRING) {
          return compare_json_strings(get_data(), get_data_length(),
                                      other.get_data(), other.get_data_length(),
                                      cs);
        }
        // Otherwise values can't be equal
        return -1;  // Treat string as less than opaque
      }
      return compare_json_strings(get_data(), get_data_length(),
                                  other.get_data(), other.get_data_length(),
                                  cs);
    case enum_json_type::J_INT:
      // Signed integers can be compared to all other numbers.
      switch (other_type) {
        case enum_json_type::J_INT:
          return compare_numbers(get_int(), other.get_int());
        case enum_json_type::J_UINT:
          return compare_json_int_uint(get_int(), other.get_uint());
        case enum_json_type::J_DOUBLE:
          return -compare_json_double_int(other.get_double(), get_int());
        case enum_json_type::J_DECIMAL: {
          my_decimal b_dec;
          if (other.get_decimal_data(&b_dec)) return 1; /* purecov: inspected */
          return -compare_json_decimal_int(b_dec, get_int());
        }
        case enum_json_type::J_BOOLEAN:
          DBUG_ASSERT(legacy_val_cmp);
          return compare_numbers(get_int(), (longlong)other.get_boolean());
        default:;
      }
      break;
    case enum_json_type::J_UINT:
      // Unsigned integers can be compared to all other numbers.
      switch (other_type) {
        case enum_json_type::J_UINT:
          return compare_numbers(get_uint(), other.get_uint());
        case enum_json_type::J_INT:
          return -compare_json_int_uint(other.get_int(), get_uint());
        case enum_json_type::J_DOUBLE:
          return -compare_json_double_uint(other.get_double(), get_uint());
        case enum_json_type::J_DECIMAL: {
          my_decimal b_dec;
          if (other.get_decimal_data(&b_dec)) return 1; /* purecov: inspected */
          return -compare_json_decimal_uint(b_dec, get_uint());
        }
        case enum_json_type::J_BOOLEAN:
          DBUG_ASSERT(legacy_val_cmp);
          return -compare_json_int_uint(other.get_boolean(), get_uint());
        default:;
      }
      break;
    case enum_json_type::J_DOUBLE:
      // Doubles can be compared to all other numbers.
      switch (other_type) {
        case enum_json_type::J_DOUBLE:
          return compare_numbers(get_double(), other.get_double());
        case enum_json_type::J_INT:
          return compare_json_double_int(get_double(), other.get_int());
        case enum_json_type::J_UINT:
          return compare_json_double_uint(get_double(), other.get_uint());
        case enum_json_type::J_DECIMAL: {
          my_decimal other_dec;
          if (other.get_decimal_data(&other_dec))
            return 1; /* purecov: inspected */
          return -compare_json_decimal_double(other_dec, get_double());
        }
        case enum_json_type::J_BOOLEAN:
          DBUG_ASSERT(legacy_val_cmp);
          return compare_json_double_int(get_double(), other.get_boolean());
        default:;
      }
      break;
    case enum_json_type::J_DECIMAL:
      // Decimals can be compared to all other numbers.
      {
        my_decimal a_dec;
        my_decimal b_dec;
        if (get_decimal_data(&a_dec)) return 1; /* purecov: inspected */
        switch (other_type) {
          case enum_json_type::J_DECIMAL:
            if (other.get_decimal_data(&b_dec))
              return 1; /* purecov: inspected */
            /*
              my_decimal_cmp() treats -0 and 0 as not equal, so check for
              zero first.
            */
            if (my_decimal_is_zero(&a_dec) && my_decimal_is_zero(&b_dec))
              return 0;
            return my_decimal_cmp(&a_dec, &b_dec);
          case enum_json_type::J_INT:
            return compare_json_decimal_int(a_dec, other.get_int());
          case enum_json_type::J_UINT:
            return compare_json_decimal_uint(a_dec, other.get_uint());
          case enum_json_type::J_DOUBLE:
            return compare_json_decimal_double(a_dec, other.get_double());
          case enum_json_type::J_BOOLEAN:
            DBUG_ASSERT(legacy_val_cmp);
            return compare_json_decimal_int(a_dec, other.get_boolean());
          default:;
        }
        break;
      }
    case enum_json_type::J_BOOLEAN:
      // Booleans are only equal to other booleans. false is less than true.
      if (other_type == enum_json_type::J_BOOLEAN) {
        return compare_numbers(get_boolean(), other.get_boolean());
      } else {
        DBUG_ASSERT(legacy_val_cmp);
        switch (other_type) {
          case enum_json_type::J_INT:
            return compare_numbers((longlong)get_boolean(), other.get_int());
          case enum_json_type::J_UINT:
            return compare_json_int_uint(get_boolean(), other.get_uint());
          case enum_json_type::J_DOUBLE:
            return -compare_json_double_int(other.get_double(), get_boolean());
          case enum_json_type::J_DECIMAL: {
            my_decimal b_dec;
            if (other.get_decimal_data(&b_dec)) return 1;
            return -compare_json_decimal_int(b_dec, get_boolean());
          }
          default:
            DBUG_ASSERT(false);
        }
      }
      break;
    case enum_json_type::J_DATETIME:
    case enum_json_type::J_TIMESTAMP:
      // Timestamps and datetimes can be equal to each other.
      {
        MYSQL_TIME val_a;
        get_datetime(&val_a);
        MYSQL_TIME val_b;
        other.get_datetime(&val_b);
        return compare_numbers(TIME_to_longlong_packed(val_a),
                               TIME_to_longlong_packed(val_b));
      }
    case enum_json_type::J_TIME:
    case enum_json_type::J_DATE:
      // Dates and times can only be equal to values of the same type.
      {
        DBUG_ASSERT(this_type == other_type);
        MYSQL_TIME val_a;
        get_datetime(&val_a);
        MYSQL_TIME val_b;
        other.get_datetime(&val_b);
        return compare_numbers(TIME_to_longlong_packed(val_a),
                               TIME_to_longlong_packed(val_b));
      }
    case enum_json_type::J_OPAQUE: {
      if (other_type == enum_json_type::J_STRING) {
        // String might be stored as J_OPAQUE, check this case
        if (field_type() == MYSQL_TYPE_VARCHAR ||
            field_type() == MYSQL_TYPE_VAR_STRING) {
          return compare_json_strings(get_data(), get_data_length(),
                                      other.get_data(), other.get_data_length(),
                                      cs);
        }
        // Otherwise values can't be equal
        return 1;  // Treat string as less than opaque
      }
      /*
        Opaque values are equal to other opaque values with the same
        field type and the same binary representation.
      */
      int cmp = compare_numbers(field_type(), other.field_type());
      if (cmp == 0)
        cmp = compare_json_strings(get_data(), get_data_length(),
                                   other.get_data(), other.get_data_length());
      return cmp;
    }
    case enum_json_type::J_NULL:
      // Null is always equal to other nulls.
      DBUG_ASSERT(this_type == other_type);
      return 0;
    case enum_json_type::J_ERROR:
      break;
  }

  DBUG_ASSERT(false); /* purecov: inspected */
  return 1;           /* purecov: inspected */
}

/*
  Use legacy json_extract comparison semantics if either of the object
  is created by legacy json_extract and one of the operand is string
  type. In case none, of the objects are string type, we will
  let Json_wrapper::compare do the comparison.

  5.6 json_extract returns a string type and this string type is then used to
  compare against other operands.
  8.0 version of json_extract however return a json type. If any operand
  is compared against json_extract, the operand itself is converted to
  json type, eventually calling into Json_wrapper::compare.

  In case any of the operands is a string, it is not safe to use
  Json_wrapper::compare since it stores string differently (without
  quotes). Also Json_wrapper::compare does not compare string with
  any other types. However 5.6 implementation did. This function is
  used to detect this anamoly.
 */
bool Json_wrapper::use_legacy_json_extract_comparison(
    const Json_wrapper &other) const {
  if (legacy_json_extract() != other.legacy_json_extract()) {
    return other.type() == enum_json_type::J_STRING ||
           other.type() == enum_json_type::J_OPAQUE ||
           type() == enum_json_type::J_STRING ||
           type() == enum_json_type::J_OPAQUE;
  }

  return false;
}

/*
  Implementation of comparison semantics for legacy json_extract object
  against other json type. This assumes that atleast one of the
  json operands is string type.

  The function converts both the operands into string in 5.6 compatible
  way and compares them. This causes the behavior similar to 5.6 json_extract
  comparison semantics.

  The reasoning for converting both operands to string if any of the operand
  is string is as follows:

  1. In case the string object was in json object created by legacy
     json_extract, then it must necessarily be wrapped under quotes for
     comparison. Now that the string represenation of json object contains
     quotes, the other operand must necessarily be converted to a string.

  2. In case non json_extract json object is a string type, we convert legacy
     object created by 5.6 json_extract into string type. This is because
     5.6 json_extract used to return a string type. In case the other operand
     is string, 5.6 type comparison would have done string comparison on
     both the operands.
 */
int Json_wrapper::legacy_json_extract_compare(const Json_wrapper &other,
                                              const CHARSET_INFO *cs) const {
  const enum_json_type this_type = type();
  const enum_json_type other_type = other.type();

  switch (this_type) {
    case enum_json_type::J_ARRAY:
    case enum_json_type::J_OBJECT:
    case enum_json_type::J_INT:
    case enum_json_type::J_UINT:
    case enum_json_type::J_DOUBLE:
    case enum_json_type::J_DECIMAL:
    case enum_json_type::J_BOOLEAN:
    case enum_json_type::J_DATETIME:
    case enum_json_type::J_TIMESTAMP:
    case enum_json_type::J_TIME:
    case enum_json_type::J_DATE:
    case enum_json_type::J_NULL: {
      /*
        For all the types here, convert this object to string and do
        the comparison. The other json object is guaranteed to be
        either string or opaque type.
       */
      if (other_type == enum_json_type::J_OPAQUE) {
        // String might be stored as J_OPAQUE, check this case
        if (other.field_type() != MYSQL_TYPE_VARCHAR &&
            other.field_type() != MYSQL_TYPE_VAR_STRING) {
          return -1;
        }
      }

      String this_value;
      this_value.length(0);
      if (to_string(&this_value, true /* json_quoted */,
                    "legacy_json_extract_compare")) {
        return type_comparison[static_cast<int>(this_type)]
                              [static_cast<int>(other_type)];
      }

      String other_value;
      other_value.length(0);
      const char *other_data = other.get_data();
      size_t other_data_length = other.get_data_length();
      /*
        In case the other object was created by legacy json_extact,
        convert into string that will encapsulate the string
        value in double quotes.
       */
      if (other.legacy_json_extract()) {
        if (other.to_string(&other_value, true /* json_quoted */,
                            "legacy_json_extract_compare")) {
          return type_comparison[static_cast<int>(this_type)]
                                [static_cast<int>(other_type)];
        }
        other_data = other_value.ptr();
        other_data_length = other_value.length();
      }

      return compare_json_strings(this_value.ptr(), this_value.length(),
                                  other_data, other_data_length, cs);
    } break;
    case enum_json_type::J_OPAQUE: {
      if (other_type == enum_json_type::J_OPAQUE) {
        /*
          Opaque values are equal to other opaque values with the same
          field type and the same binary representation.
        */

        int cmp = compare_numbers(field_type(), other.field_type());
        if (cmp == 0)
          cmp = compare_json_strings(get_data(), get_data_length(),
                                     other.get_data(), other.get_data_length());

        return cmp;
      }

      if (field_type() != MYSQL_TYPE_VARCHAR &&
          field_type() != MYSQL_TYPE_VAR_STRING) {
        return 1;
      }
    }
    /* Fall through */
    case enum_json_type::J_STRING: {
      if (other_type == enum_json_type::J_OPAQUE) {
        // String might be stored as J_OPAQUE, check this case
        if (other.field_type() != MYSQL_TYPE_VARCHAR &&
            other.field_type() != MYSQL_TYPE_VAR_STRING) {
          return -1;
        }
      }

      String this_value;
      this_value.length(0);
      const char *this_data = get_data();
      size_t this_data_length = get_data_length();
      if (legacy_json_extract()) {
        /*
          Here this object is created by legacy json_extract.
          So we need to explicitly convert this json object to string. This
          ensures that current json object encapsulates the string in double
          quotes and does the comparison similar to 5.6.
         */
        if (to_string(&this_value, true /* json_quoted */,
                      "legacy_json_extract_compare")) {
          return type_comparison[static_cast<int>(this_type)]
                                [static_cast<int>(other_type)];
        }

        this_data = this_value.ptr();
        this_data_length = this_value.length();
      }

      String other_value;
      other_value.length(0);
      const char *other_data;
      size_t other_data_length;
      /*
        Convert the other object to string for comparison. In
        case the other json object is already string, check if
        it is created by legacy json extract. If so, we need to
        encapsulated the other json object into quotes. So
        convert it into string.
       */
      if ((other_type != enum_json_type::J_OPAQUE &&
           other_type != enum_json_type::J_STRING) ||
          other.legacy_json_extract()) {
        if (other.to_string(&other_value, true /* json_quoted */,
                            "legacy_json_extract_compare")) {
          return type_comparison[static_cast<int>(this_type)]
                                [static_cast<int>(other_type)];
        }
        other_data = other_value.ptr();
        other_data_length = other_value.length();
      } else {
        other_data = other.get_data();
        other_data_length = other.get_data_length();
      }

      return compare_json_strings(this_data, this_data_length, other_data,
                                  other_data_length, cs);
    }

    default:
      break;
  }

  return -1;
}

/**
  Push a warning/error about a problem encountered when coercing a JSON
  value to some other data type.

  @param[in] cr_error     what to issue: a warning or an error
  @param[in] target_type  the name of the target type of the coercion
  @param[in] error_code   the error code to use for the warning
  @param[in] msgnam       the name of the field/expression being coerced
*/

static void handle_coercion_error(enum_coercion_error cr_error,
                                  const char *target_type, int error_code,
                                  const char *msgnam) {
  switch (cr_error) {
    case CE_WARNING: {
      /*
        One argument is no longer used (the empty string), but kept to avoid
        changing error message format.
      */
      push_warning_printf(
          current_thd, Sql_condition::SL_WARNING, error_code,
          ER_THD_NONCONST(current_thd, error_code), target_type, "", msgnam,
          current_thd->get_stmt_da()->current_row_for_condition());
      return;
    }
    case CE_ERROR: {
      my_error(error_code, MYF(0), target_type, "", msgnam,
               current_thd->get_stmt_da()->current_row_for_condition());
      return;
    }
  }
}

longlong Json_wrapper::coerce_int(const char *msgnam, bool *err,
                                  enum_coercion_error cr_error) const {
  if (err) *err = false;
  switch (type()) {
    case enum_json_type::J_UINT:
      return static_cast<longlong>(get_uint());
    case enum_json_type::J_INT:
      return get_int();
    case enum_json_type::J_STRING: {
      /*
        For a string result, we must first get the string and then convert it
        to a longlong.
      */
      const char *start = get_data();
      size_t length = get_data_length();
      const char *end = start + length;
      const CHARSET_INFO *cs = &my_charset_utf8mb4_bin;

      int error;
      longlong value = cs->cset->strtoll10(cs, start, &end, &error);

      if (error > 0 || end != start + length) {
        int code =
            (error == MY_ERRNO_ERANGE ? ER_NUMERIC_JSON_VALUE_OUT_OF_RANGE
                                      : ER_INVALID_JSON_VALUE_FOR_CAST);
        handle_coercion_error(cr_error, "INTEGER", code, msgnam);
        if (err) *err = true;
      }

      return value;
    }
    case enum_json_type::J_BOOLEAN:
      return get_boolean() ? 1 : 0;
    case enum_json_type::J_DECIMAL: {
      longlong i;
      my_decimal decimal_value;
      get_decimal_data(&decimal_value);
      /*
        We do not know if this int is destined for signed or unsigned usage, so
        just get longlong from the value using the sign in the decimal.
      */
      my_decimal2int(E_DEC_FATAL_ERROR, &decimal_value, !decimal_value.sign(),
                     &i);
      return i;
    }
    case enum_json_type::J_DOUBLE: {
      // logic here is borrowed from Field_double::val_int
      double j = get_double();
      longlong res;

      if (j <= (double)LLONG_MIN) {
        res = LLONG_MIN;
      } else if (j >= (double)(ulonglong)LLONG_MAX) {
        res = LLONG_MAX;
      } else {
        return (longlong)rint(j);
      }

      handle_coercion_error(cr_error, "INTEGER",
                            ER_NUMERIC_JSON_VALUE_OUT_OF_RANGE, msgnam);
      if (err) *err = true;
      return res;
    }
    default:;
  }

  handle_coercion_error(cr_error, "INTEGER", ER_INVALID_JSON_VALUE_FOR_CAST,
                        msgnam);
  if (err) *err = true;
  return 0;
}

double Json_wrapper::coerce_real(const char *msgnam, bool *err,
                                 enum_coercion_error cr_error) const {
  if (err) *err = false;
  switch (type()) {
    case enum_json_type::J_DECIMAL: {
      double dbl;
      my_decimal decimal_value;
      get_decimal_data(&decimal_value);
      my_decimal2double(E_DEC_FATAL_ERROR, &decimal_value, &dbl);
      return dbl;
    }
    case enum_json_type::J_STRING: {
      /*
        For a string result, we must first get the string and then convert it
        to a double.
      */
      const char *start = get_data();
      size_t length = get_data_length();
      const char *end = start + length;
      const CHARSET_INFO *cs = &my_charset_utf8mb4_bin;

      int error;
      double value = my_strntod(cs, start, length, &end, &error);

      if (error || end != start + length) {
        int code = (error == EOVERFLOW ? ER_NUMERIC_JSON_VALUE_OUT_OF_RANGE
                                       : ER_INVALID_JSON_VALUE_FOR_CAST);
        handle_coercion_error(cr_error, "DOUBLE", code, msgnam);
        if (err) *err = true;
      }
      return value;
    }
    case enum_json_type::J_DOUBLE:
      return get_double();
    case enum_json_type::J_INT:
      return static_cast<double>(get_int());
    case enum_json_type::J_UINT:
      return static_cast<double>(get_uint());
    case enum_json_type::J_BOOLEAN:
      return static_cast<double>(get_boolean());
    default:;
  }

  handle_coercion_error(cr_error, "DOUBLE", ER_INVALID_JSON_VALUE_FOR_CAST,
                        msgnam);
  if (err) *err = true;
  return 0.0;
}

my_decimal *Json_wrapper::coerce_decimal(my_decimal *decimal_value,
                                         const char *msgnam, bool *err,
                                         enum_coercion_error cr_error) const {
  if (err) *err = false;
  switch (type()) {
    case enum_json_type::J_DECIMAL:
      get_decimal_data(decimal_value);
      return decimal_value;
    case enum_json_type::J_STRING: {
      /*
        For a string result, we must first get the string and then convert it
        to a decimal.
        it has own error reporting, but not very informative, disable it, except
        for OOM
      */
      int error = str2my_decimal(E_DEC_OOM, get_data(), get_data_length(),
                                 &my_charset_utf8mb4_bin, decimal_value);
      if (error) {
        int code = (error == E_DEC_OVERFLOW ? ER_NUMERIC_JSON_VALUE_OUT_OF_RANGE
                                            : ER_INVALID_JSON_VALUE_FOR_CAST);
        handle_coercion_error(cr_error, "DECIMAL", code, msgnam);
        if (err) *err = true;
      }
      return decimal_value;
    }
    case enum_json_type::J_DOUBLE:
      if (double2my_decimal(E_DEC_FATAL_ERROR, get_double(), decimal_value)) {
        handle_coercion_error(cr_error, "DECIMAL",
                              ER_NUMERIC_JSON_VALUE_OUT_OF_RANGE, msgnam);
        if (err) *err = true;
      }
      return decimal_value;
    case enum_json_type::J_INT:
      if (longlong2decimal(get_int(), decimal_value)) {
        handle_coercion_error(cr_error, "DECIMAL",
                              ER_NUMERIC_JSON_VALUE_OUT_OF_RANGE, msgnam);
        if (err) *err = true;
      }
      return decimal_value;
    case enum_json_type::J_UINT:
      if (longlong2decimal(get_uint(), decimal_value)) {
        handle_coercion_error(cr_error, "DECIMAL",
                              ER_NUMERIC_JSON_VALUE_OUT_OF_RANGE, msgnam);
        if (err) *err = true;
      }
      return decimal_value;
    case enum_json_type::J_BOOLEAN:
      // no danger of overflow, so void result
      (void)int2my_decimal(E_DEC_FATAL_ERROR, get_boolean(),
                           true /* unsigned */, decimal_value);
      return decimal_value;
    default:;
  }

  handle_coercion_error(cr_error, "DECIMAL", ER_INVALID_JSON_VALUE_FOR_CAST,
                        msgnam);
  if (err) *err = true;

  my_decimal_set_zero(decimal_value);
  return decimal_value;
}

bool Json_wrapper::coerce_date(MYSQL_TIME *ltime, const char *msgnam,
                               enum_coercion_error cr_error) const {
  switch (type()) {
    case enum_json_type::J_DATETIME:
    case enum_json_type::J_DATE:
    case enum_json_type::J_TIMESTAMP:
      set_zero_time(ltime, MYSQL_TIMESTAMP_DATETIME);
      get_datetime(ltime);
      return false;
    case enum_json_type::J_STRING: {
      MYSQL_TIME_STATUS status;
      THD *thd = current_thd;
      // @see Field_datetime::date_flags
      my_time_flags_t date_flags = TIME_FUZZY_DATE;
      if (thd->variables.sql_mode & MODE_NO_ZERO_DATE)
        date_flags |= TIME_NO_ZERO_DATE;
      if (thd->variables.sql_mode & MODE_NO_ZERO_IN_DATE)
        date_flags |= TIME_NO_ZERO_IN_DATE;
      if (thd->variables.sql_mode & MODE_INVALID_DATES)
        date_flags |= TIME_INVALID_DATES;
      if (thd->variables.sql_mode & MODE_TIME_TRUNCATE_FRACTIONAL)
        date_flags |= TIME_FRAC_TRUNCATE;
      if (!str_to_datetime(get_data(), get_data_length(), ltime, date_flags,
                           &status) &&
          !status.warnings)
        break;
    }
    /* Fall through */
    default:
      handle_coercion_error(cr_error, "DATE/TIME/DATETIME/TIMESTAMP",
                            ER_INVALID_JSON_VALUE_FOR_CAST, msgnam);
      return true;
  }
  return false;
}

bool Json_wrapper::coerce_time(MYSQL_TIME *ltime, const char *msgnam,
                               enum_coercion_error cr_error) const {
  switch (type()) {
    case enum_json_type::J_TIME:
      set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
      get_datetime(ltime);
      return false;
    case enum_json_type::J_STRING: {
      MYSQL_TIME_STATUS status;
      set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
      if (!str_to_time(get_data(), get_data_length(), ltime, &status,
                       TIME_STRICT_COLON) &&
          !status.warnings)
        break;
    }
    /* Fall through */
    default:
      handle_coercion_error(cr_error, "DATE/TIME/DATETIME/TIMESTAMP",
                            ER_INVALID_JSON_VALUE_FOR_CAST, msgnam);
      return true;
  }
  return false;
}

namespace {

/// Wrapper around a sort key buffer.
class Wrapper_sort_key {
 private:
  uchar *m_buffer;  ///< the buffer into which to write
  size_t m_length;  ///< the length of the buffer
  size_t m_pos;     ///< the current position in the buffer

 public:
  Wrapper_sort_key(uchar *buf, size_t len)
      : m_buffer(buf), m_length(len), m_pos(0) {}

  /// Get the remaining space in the buffer.
  size_t remaining() const { return m_length - m_pos; }

  /// Get how much space we've used so far.
  size_t pos() const { return m_pos; }

  /// Append a character to the buffer.
  void append(uchar ch) {
    if (m_pos < m_length) m_buffer[m_pos++] = ch;
  }

  /**
     Pad the buffer with the specified character till given position.
     @note This function is intended to be used to make numbers of equal length
     without occupying the whole buffer.
   */
  void pad_till(uchar pad_character, size_t pos) {
    longlong num_chars = pos - m_pos;
    DBUG_ASSERT(num_chars >= 0);
    num_chars = std::min(remaining(), static_cast<size_t>(num_chars));
    memset(m_buffer + m_pos, pad_character, num_chars);
    m_pos += num_chars;
  }

  /**
    Copy an integer to the buffer and format it in a way that makes it
    possible to sort the integers with memcpy().

    @param target_length  the number of bytes to write to the buffer
    @param from           the buffer to copy the integer from (in little-endian
                          format)
    @param from_length    the size of the from buffer
    @param is_unsigned    true if the from buffer contains an unsigned integer,
                          false otherwise
  */
  void copy_int(size_t target_length, const uchar *from, size_t from_length,
                bool is_unsigned) {
    size_t to_length = std::min(remaining(), target_length);
    copy_integer<false>(m_buffer + m_pos, to_length, from, from_length,
                        is_unsigned);
    m_pos += to_length;
  }

  /**
    Append a string to the buffer, and add the length of the string to
    the end of the buffer. The space between the end of the string and
    the beginning of the length field is padded with zeros.
  */
  void append_str_and_len(const char *str, size_t len) {
    /*
      The length is written as a four byte value at the end of the
      buffer, provided that there is enough room and string to be stored is
      longer than buffer.
    */
    size_t space_for_len =
        (len <= remaining())
            ? 0
            : std::min(static_cast<size_t>(VARLEN_PREFIX), remaining());

    /*
      The string contents are written up to where the length is
      stored, and get truncated if the string is longer than that.
    */
    size_t space_for_str = remaining() - space_for_len;
    size_t copy_len = std::min(len, space_for_str);
    memcpy(m_buffer + m_pos, str, copy_len);
    m_pos += copy_len;

    /*
      Write the length in a format that memcmp() knows how to sort.
      First we store it in little-endian format in a four-byte buffer,
      and then we use copy_integer to transform it into a format that
      works with memcmp().
    */
    if (space_for_str) {
      uchar length_buffer[VARLEN_PREFIX];
      int4store(length_buffer, static_cast<uint32>(len));
      copy_int(space_for_len, length_buffer, sizeof(length_buffer), true);
    }
  }
};

/// Helper class for building a hash key.
class Wrapper_hash_key {
 private:
  ulonglong m_crc;

 public:
  explicit Wrapper_hash_key(ulonglong hash_val) : m_crc(hash_val) {}

  /**
    Return the computed hash value.
  */
  ulonglong get_crc() const { return m_crc; }

  void add_character(uchar ch) { add_to_crc(ch); }

  void add_integer(longlong ll) {
    char tmp[8];
    int8store(tmp, ll);
    add_string(tmp, sizeof(tmp));
  }

  void add_double(double d) {
    // Make -0.0 and +0.0 have the same key.
    if (d == 0) {
      add_character(0);
      return;
    }

    char tmp[8];
    float8store(tmp, d);
    add_string(tmp, sizeof(tmp));
  }

  void add_string(const char *str, size_t len) {
    for (size_t idx = 0; idx < len; idx++) {
      add_to_crc(*str++);
    }
  }

 private:
  /**
    Add another character to the evolving crc.

    @param[in] ch The character to add
  */
  void add_to_crc(uchar ch) {
    // This logic was cribbed from sql_executor.cc/unique_hash
    m_crc = ((m_crc << 8) + (((uchar)ch))) +
            (m_crc >> (8 * sizeof(ha_checksum) - 8));
  }
};

/*
  Type identifiers used in the sort key generated by
  Json_wrapper::make_sort_key(). Types with lower identifiers sort
  before types with higher identifiers.
  See also note for Json_dom::enum_json_type.
*/
constexpr uchar JSON_KEY_NULL = '\x00';
constexpr uchar JSON_KEY_NUMBER_NEG = '\x01';
constexpr uchar JSON_KEY_NUMBER_ZERO = '\x02';
constexpr uchar JSON_KEY_NUMBER_POS = '\x03';
constexpr uchar JSON_KEY_STRING = '\x04';
constexpr uchar JSON_KEY_OBJECT = '\x05';
constexpr uchar JSON_KEY_ARRAY = '\x06';
constexpr uchar JSON_KEY_FALSE = '\x07';
constexpr uchar JSON_KEY_TRUE = '\x08';
constexpr uchar JSON_KEY_DATE = '\x09';
constexpr uchar JSON_KEY_TIME = '\x0A';
constexpr uchar JSON_KEY_DATETIME = '\x0B';
constexpr uchar JSON_KEY_OPAQUE = '\x0C';

}  // namespace

/*
  Max char position to pad numeric sort keys to. Includes max precision +
  sort key len.
*/
#define MAX_NUMBER_SORT_PAD \
  (std::max(DBL_DIG, DECIMAL_MAX_POSSIBLE_PRECISION) + VARLEN_PREFIX + 3)

/**
  Make a sort key for a JSON numeric value from its string representation. The
  input string could be either on scientific format (such as 1.234e2) or on
  plain format (such as 12.34).

  The sort key will have the following parts:

  1) One byte that is JSON_KEY_NUMBER_NEG, JSON_KEY_NUMBER_ZERO or
  JSON_KEY_NUMBER_POS if the number is positive, zero or negative,
  respectively.

  2) Two bytes that represent the decimal exponent of the number (log10 of the
  number, truncated to an integer).

  3) All the digits of the number, without leading zeros.

  4) Padding to ensure that equal numbers sort equal even if they have a
  different number of trailing zeros.

  If the number is zero, parts 2, 3 and 4 are skipped.

  For negative numbers, the values in parts 2, 3 and 4 need to be inverted so
  that bigger negative numbers sort before smaller negative numbers.

  @param[in]     from     the string representation of the number
  @param[in]     len      the length of the input string
  @param[in]     negative true if the number is negative, false otherwise
  @param[in,out] to       the target sort key
*/
static void make_json_numeric_sort_key(const char *from, size_t len,
                                       bool negative, Wrapper_sort_key *to) {
  const char *end = from + len;

  // Find the start of the exponent part, if there is one.
  const char *end_of_digits = std::find(from, end, 'e');

  /*
    Find the first significant digit. Skip past sign, leading zeros
    and the decimal point, until the first non-zero digit is found.
  */
  const auto is_non_zero_digit = [](char c) { return c >= '1' && c <= '9'; };
  const char *first_significant_digit =
      std::find_if(from, end_of_digits, is_non_zero_digit);

  if (first_significant_digit == end_of_digits) {
    // We didn't find any significant digits, so the number is zero.
    to->append(JSON_KEY_NUMBER_ZERO);
    return;
  }

  longlong exp;
  if (end_of_digits != end) {
    // Scientific format. Fetch the exponent part after the 'e'.
    char *endp = const_cast<char *>(end);
    exp = my_strtoll(end_of_digits + 1, &endp, 10);
  } else {
    /*
      Otherwise, find the exponent by calculating the distance between the
      first significant digit and the decimal point.
    */
    const char *dec_point = std::find(from, end_of_digits, '.');
    if (!dec_point) {
      // There is no decimal point. Just count the digits.
      exp = end_of_digits - first_significant_digit - 1;
    } else if (first_significant_digit < dec_point) {
      // Non-negative exponent.
      exp = dec_point - first_significant_digit - 1;
    } else {
      // Negative exponent.
      exp = dec_point - first_significant_digit;
    }
  }

  if (negative) {
    to->append(JSON_KEY_NUMBER_NEG);
    /*
      For negative numbers, we have to invert the exponents so that numbers
      with high exponents sort before numbers with low exponents.
    */
    exp = -exp;
  } else {
    to->append(JSON_KEY_NUMBER_POS);
  }

  /*
    Store the exponent part before the digits. Since the decimal exponent of a
    double can be in the range [-323, +308], we use two bytes for the
    exponent. (Decimals and bigints also fit in that range.)
  */
  uchar exp_buff[2];
  int2store(exp_buff, static_cast<int16>(exp));
  to->copy_int(sizeof(exp_buff), exp_buff, sizeof(exp_buff), false);

  /*
    Append all the significant digits of the number. Stop before the exponent
    part if there is one, otherwise go to the end of the string.
  */
  for (const char *ch = first_significant_digit; ch < end_of_digits; ++ch) {
    if (my_isdigit(&my_charset_numeric, *ch)) {
      /*
        If the number is negative, the digits must be inverted so that big
        negative numbers sort before small negative numbers.
      */
      if (negative)
        to->append('9' - *ch + '0');
      else
        to->append(*ch);
    }
  }

  /*
    Pad the number with zeros up to 30 bytes, so that the number of trailing
    zeros doesn't affect how the number is sorted. As above, we need to invert
    the digits for negative numbers.
  */
  to->pad_till(negative ? '9' : '0', MAX_NUMBER_SORT_PAD);
}

size_t Json_wrapper::make_sort_key(uchar *to, size_t to_length) const {
  Wrapper_sort_key key(to, to_length);

  const enum_json_type jtype = type();
  switch (jtype) {
    case enum_json_type::J_NULL:
      key.append(JSON_KEY_NULL);
      break;
    case enum_json_type::J_DECIMAL: {
      my_decimal dec;
      if (get_decimal_data(&dec)) break; /* purecov: inspected */
      StringBuffer<DECIMAL_MAX_STR_LENGTH + 1> str(&my_charset_numeric);
      if (my_decimal2string(E_DEC_FATAL_ERROR, &dec, &str))
        break; /* purecov: inspected */
      make_json_numeric_sort_key(str.ptr(), str.length(), dec.sign(), &key);
      break;
    }
    case enum_json_type::J_INT: {
      longlong i = get_int();
      char buff[MAX_BIGINT_WIDTH + 1];
      size_t len = longlong10_to_str(i, buff, -10) - buff;
      make_json_numeric_sort_key(buff, len, i < 0, &key);
      break;
    }
    case enum_json_type::J_UINT: {
      ulonglong ui = get_uint();
      char buff[MAX_BIGINT_WIDTH + 1];
      size_t len = longlong10_to_str(ui, buff, 10) - buff;
      make_json_numeric_sort_key(buff, len, false, &key);
      break;
    }
    case enum_json_type::J_DOUBLE: {
      double dbl = get_double();
      char buff[MY_GCVT_MAX_FIELD_WIDTH + 1];
      size_t len =
          my_gcvt(dbl, MY_GCVT_ARG_DOUBLE, sizeof(buff) - 1, buff, nullptr);
      make_json_numeric_sort_key(buff, len, (dbl < 0), &key);
      break;
    }
    case enum_json_type::J_STRING:
      key.append(JSON_KEY_STRING);
      key.append_str_and_len(get_data(), get_data_length());
      break;
    case enum_json_type::J_OBJECT:
    case enum_json_type::J_ARRAY:
      /*
        Internal ordering of objects and arrays only considers length
        for now.
      */
      {
        key.append(jtype == enum_json_type::J_OBJECT ? JSON_KEY_OBJECT
                                                     : JSON_KEY_ARRAY);
        uchar len[4];
        int4store(len, static_cast<uint32>(length()));
        key.copy_int(sizeof(len), len, sizeof(len), true);
        /*
          Raise a warning to give an indication that sorting of objects
          and arrays is not properly supported yet. The warning is
          raised for each object/array that is found during the sort,
          but Filesort_error_handler will make sure that only one
          warning is seen on the top level for every sort.
        */
        push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                            ER_NOT_SUPPORTED_YET,
                            ER_THD(current_thd, ER_NOT_SUPPORTED_YET),
                            "sorting of non-scalar JSON values");
        break;
      }
    case enum_json_type::J_BOOLEAN:
      key.append(get_boolean() ? JSON_KEY_TRUE : JSON_KEY_FALSE);
      break;
    case enum_json_type::J_DATE:
    case enum_json_type::J_TIME:
    case enum_json_type::J_DATETIME:
    case enum_json_type::J_TIMESTAMP: {
      if (jtype == enum_json_type::J_DATE)
        key.append(JSON_KEY_DATE);
      else if (jtype == enum_json_type::J_TIME)
        key.append(JSON_KEY_TIME);
      else
        key.append(JSON_KEY_DATETIME);

      /*
        Temporal values are stored in the packed format in the binary
        JSON format. The packed values are 64-bit signed little-endian
        integers.
      */
      const size_t packed_length = Json_datetime::PACKED_SIZE;
      char tmp[packed_length];
      const char *packed = get_datetime_packed(tmp);
      key.copy_int(packed_length, pointer_cast<const uchar *>(packed),
                   packed_length, false);
      break;
    }
    case enum_json_type::J_OPAQUE:
      key.append(JSON_KEY_OPAQUE);
      key.append(field_type());
      key.append_str_and_len(get_data(), get_data_length());
      break;
    case enum_json_type::J_ERROR:
      break;
  }

  return key.pos();
}

ulonglong Json_wrapper::make_hash_key(ulonglong hash_val) const {
  Wrapper_hash_key hash_key(hash_val);
  switch (type()) {
    case enum_json_type::J_NULL:
      hash_key.add_character(JSON_KEY_NULL);
      break;
    case enum_json_type::J_DECIMAL: {
      my_decimal dec;
      if (get_decimal_data(&dec)) break; /* purecov: inspected */
      double dbl;
      decimal2double(&dec, &dbl);
      hash_key.add_double(dbl);
      break;
    }
    case enum_json_type::J_INT:
      hash_key.add_double(static_cast<double>(get_int()));
      break;
    case enum_json_type::J_UINT:
      hash_key.add_double(ulonglong2double(get_uint()));
      break;
    case enum_json_type::J_DOUBLE:
      hash_key.add_double(get_double());
      break;
    case enum_json_type::J_STRING:
    case enum_json_type::J_OPAQUE:
      hash_key.add_string(get_data(), get_data_length());
      break;
    case enum_json_type::J_OBJECT: {
      hash_key.add_character(JSON_KEY_OBJECT);
      for (const auto &it : Json_object_wrapper(*this)) {
        const MYSQL_LEX_CSTRING &key = it.first;
        hash_key.add_string(key.str, key.length);
        hash_key.add_integer(it.second.make_hash_key(hash_key.get_crc()));
      }
      break;
    }
    case enum_json_type::J_ARRAY: {
      hash_key.add_character(JSON_KEY_ARRAY);
      size_t elts = length();
      for (uint i = 0; i < elts; i++) {
        hash_key.add_integer((*this)[i].make_hash_key(hash_key.get_crc()));
      }
      break;
    }
    case enum_json_type::J_BOOLEAN:
      hash_key.add_character(get_boolean() ? JSON_KEY_TRUE : JSON_KEY_FALSE);
      break;
    case enum_json_type::J_DATE:
    case enum_json_type::J_TIME:
    case enum_json_type::J_DATETIME:
    case enum_json_type::J_TIMESTAMP: {
      const size_t packed_length = Json_datetime::PACKED_SIZE;
      char tmp[packed_length];
      const char *packed = get_datetime_packed(tmp);
      hash_key.add_string(packed, packed_length);
      break;
    }
    case enum_json_type::J_ERROR:
      DBUG_ASSERT(false); /* purecov: inspected */
      break;              /* purecov: inspected */
  }

  ulonglong result = hash_key.get_crc();
  return result;
}

bool Json_wrapper::get_free_space(size_t *space) const {
  if (m_is_dom) {
    *space = 0;
    return false;
  }

  return m_value.get_free_space(current_thd, space);
}

bool Json_wrapper::attempt_binary_update(const Field_json *field,
                                         const Json_seekable_path &path,
                                         Json_wrapper *new_value, bool replace,
                                         String *result,
                                         bool *partially_updated,
                                         bool *replaced_path) {
  using namespace json_binary;

  // Can only do partial update if the input value is binary.
  DBUG_ASSERT(!is_dom());

  /*
    If we are replacing the top-level document, there's no need for
    partial update. The full document is rewritten anyway.
  */
  if (path.leg_count() == 0) {
    *partially_updated = false;
    *replaced_path = false;
    return false;
  }

  // Find the parent of the value we want to modify.
  Json_wrapper_vector hits(key_memory_JSON);
  if (seek_no_dup_elimination(
          m_value, path.begin(),
          Json_seek_params(path.end() - 1, &hits, false, true)))
    return true; /* purecov: inspected */

  if (hits.empty()) {
    /*
      No parent array/object was found, so both JSON_SET and
      JSON_REPLACE will be no-ops. Return success.
    */
    *partially_updated = true;
    *replaced_path = false;
    return false;
  }

  DBUG_ASSERT(hits.size() == 1);
  DBUG_ASSERT(!hits[0].is_dom());

  auto &parent = hits[0].m_value;
  const Json_path_leg *last_leg = path.last_leg();
  size_t element_pos;
  switch (parent.type()) {
    case Value::OBJECT:
      if (last_leg->get_type() != jpl_member) {
        /*
          Nothing to do for JSON_REPLACE, because we cannot replace an
          array cell in an object. JSON_SET will auto-wrap the object,
          so fall back to full update in that case
        */
        *partially_updated = replace;
        *replaced_path = false;
        return false;
      }
      element_pos = parent.lookup_index(last_leg->get_member_name());
      /*
        If the member is not found, JSON_REPLACE is done (it's a no-op),
        whereas JSON_SET will need to add a new element to the object.
      */
      if (element_pos == parent.element_count()) {
        *partially_updated = replace;
        *replaced_path = false;
        return false;
      }
      break;
    case Value::ARRAY: {
      if (last_leg->get_type() != jpl_array_cell) {
        // Nothing to do. Cannot replace an object member in an array.
        *partially_updated = true;
        *replaced_path = false;
        return false;
      }
      Json_array_index idx =
          last_leg->first_array_index(parent.element_count());
      /*
        If the element is not found, JSON_REPLACE is done (it's a no-op),
        whereas JSON_SET will need to add a new element to the array
      */
      if (!idx.within_bounds()) {
        *partially_updated = replace;
        *replaced_path = false;
        return false;
      }
      element_pos = idx.position();
    } break;
    default:
      /*
        There's no element to replace inside a scalar, so we're done if
        we have replace semantics. JSON_SET may want to auto-wrap the
        scalar if it is accessed as an array, and in that case we need
        to fall back to full update.
      */
      *partially_updated = replace || (last_leg->get_type() != jpl_array_cell);
      *replaced_path = false;
      return false;
  }

  DBUG_ASSERT(element_pos < parent.element_count());

  // Find out how much space we need to store new_value.
  size_t needed;
  const THD *thd = field->table->in_use;
  if (space_needed(thd, new_value, parent.large_format(), &needed)) return true;

  // Do we have that space available?
  size_t data_offset = 0;
  if (needed > 0 && !parent.has_space(element_pos, needed, &data_offset)) {
    *partially_updated = false;
    *replaced_path = false;
    return false;
  }

  /*
    Get a pointer to the binary representation of the document. If the result
    buffer is not empty, it contains the binary representation of the document,
    including any other partial updates made to it previously in this
    operation. If it is empty, the document is unchanged and its binary
    representation can be retrieved from the Field.
  */
  const char *original;
  if (result->is_empty()) {
    if (m_value.raw_binary(thd, result)) return true; /* purecov: inspected */
    original = field->get_binary();
  } else {
    DBUG_ASSERT(is_binary_backed_by(result));
    original = result->ptr();
  }

  DBUG_ASSERT(result->length() >= data_offset + needed);

  char *destination = result->ptr();
  bool changed = false;
  if (parent.update_in_shadow(field, element_pos, new_value, data_offset,
                              needed, original, destination, &changed))
    return true; /* purecov: inspected */

  m_value = parse_binary(result->ptr(), result->length());
  *partially_updated = true;
  *replaced_path = changed;
  return false;
}

bool Json_wrapper::binary_remove(const Field_json *field,
                                 const Json_seekable_path &path, String *result,
                                 bool *found_path) {
  // Can only do partial update if the input value is binary.
  DBUG_ASSERT(!is_dom());

  // Empty paths are short-circuited higher up. (Should be a no-op.)
  DBUG_ASSERT(path.leg_count() > 0);

  *found_path = false;

  Json_wrapper_vector hits(key_memory_JSON);
  if (seek_no_dup_elimination(
          m_value, path.begin(),
          Json_seek_params(path.end() - 1, &hits, false, true)))
    return true; /* purecov: inspected */

  DBUG_ASSERT(hits.size() <= 1);

  if (hits.empty()) return false;

  auto &parent = hits[0].m_value;
  const Json_path_leg *last_leg = path.last_leg();
  size_t element_pos;
  switch (parent.type()) {
    case json_binary::Value::OBJECT:
      if (last_leg->get_type() != enum_json_path_leg_type::jpl_member)
        return false;  // no match, nothing to remove
      element_pos = parent.lookup_index(last_leg->get_member_name());
      break;
    case json_binary::Value::ARRAY: {
      if (last_leg->get_type() != enum_json_path_leg_type::jpl_array_cell)
        return false;  // no match, nothing to remove
      Json_array_index idx =
          last_leg->first_array_index(parent.element_count());
      if (!idx.within_bounds()) return false;  // no match, nothing to remove
      element_pos = idx.position();
      break;
    }
    default:
      // Can only remove elements from objects and arrays, so nothing to do.
      return false;
  }

  if (element_pos >= parent.element_count())
    return false;  // no match, nothing to remove

  /*
    Get a pointer to the binary representation of the document. If the result
    buffer is not empty, it contains the binary representation of the document,
    including any other partial updates made to it previously in this
    operation. If it is empty, the document is unchanged and its binary
    representation can be retrieved from the Field.
  */
  const char *original;
  if (result->is_empty()) {
    if (m_value.raw_binary(field->table->in_use, result))
      return true; /* purecov: inspected */
    original = field->get_binary();
  } else {
    DBUG_ASSERT(is_binary_backed_by(result));
    original = result->ptr();
  }

  char *destination = result->ptr();

  if (parent.remove_in_shadow(field, element_pos, original, destination))
    return true; /* purecov: inspected */

  m_value = json_binary::parse_binary(result->ptr(), result->length());
  *found_path = true;
  return false;
}

void Json_wrapper::sort(const CHARSET_INFO *cs) {
  DBUG_ASSERT(type() == enum_json_type::J_ARRAY && is_dom());
  down_cast<Json_array *>(m_dom_value)->sort(cs);
}

void Json_wrapper::remove_duplicates(const CHARSET_INFO *cs) {
  DBUG_ASSERT(type() == enum_json_type::J_ARRAY && is_dom());
  down_cast<Json_array *>(m_dom_value)->remove_duplicates(cs);
}
#endif  // ifdef MYSQL_SERVER
