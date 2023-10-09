/* Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#pragma once

#include "watchman/watchman_string.h" // Needed for w_string_t

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <atomic>
#include <cstdlib> /* for size_t */
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "watchman/thirdparty/jansson/utf.h"

/* types */

enum json_type : char {
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_STRING,
  JSON_INTEGER,
  JSON_REAL,
  JSON_TRUE,
  JSON_FALSE,
  JSON_NULL
};

// Required for fmt 10
inline char format_as(json_type type) {
  return static_cast<char>(type);
}

struct json_t {
  json_type type;
  std::atomic<size_t> refcount;

  explicit json_t(json_type type);

  struct SingletonHack {};
  // true, false, null are never heap allocated, always
  // reference a global singleton value with a bogus refcount
  json_t(json_type type, SingletonHack&&);
};

#define JSON_INTEGER_FORMAT PRId64
using json_int_t = int64_t;

class json_ref {
  // ref_ is never a null pointer. The moved-from json_ref points to the JSON
  // null singleton.
  json_t* ref_;

  void incref() {
    if (ref_->refcount.load(std::memory_order_relaxed) != (size_t)-1) {
      ref_->refcount.fetch_add(1, std::memory_order_relaxed);
    }
  }

  void decref() {
    if (ref_->refcount.load(std::memory_order_relaxed) != (size_t)-1) {
      if (1 == ref_->refcount.fetch_sub(1, std::memory_order_acq_rel)) {
        json_delete(ref_);
      }
    }
  }

  static void json_delete(json_t* json);

  /**
   * DO NOT USE. Private constructor for internal use by json_* constructor
   * functions.
   */
  explicit json_ref(json_t* ref) : ref_{ref} {
    assert(ref_ != nullptr);
  }

 public:
  /**
   * DO NOT USE. Private constructor for internal use by json_* constructor
   * functions.
   */
  static json_ref takeOwnership(json_t* ref) {
    return json_ref{ref};
  }

  json_ref() = delete;
  ~json_ref();

  json_ref(const json_ref& other);
  json_ref& operator=(const json_ref& other);

  json_ref(json_ref&& other) noexcept;
  json_ref& operator=(json_ref&& other) noexcept;

  void reset();

  json_t* get() const {
    return ref_;
  }

  /**
   * Returns the value associated with key in a json object.
   * Returns defval if this json value is not an object or
   * if the key was not found.
   */
  json_ref get_default(const char* key, json_ref defval) const;

  /**
   * Returns the value associated with key in a json object.
   * Throws domain_error if this is not a json object or
   * a range_error if the key is not present.
   */
  const json_ref& get(const char* key) const;

  /**
   * Returns the value associated with a key in a JSON object.
   * Returns std::nullopt if this JSON value is not an object or if the key is
   * not found.
   */
  std::optional<json_ref> get_optional(const char* key) const;

  /** Set key = value */
  void set(const char* key, json_ref&& val);
  void set(const w_string& key, json_ref&& val);

  /** Set a list of key/value pairs */
  void set(std::initializer_list<std::pair<const char*, json_ref&&>> pairs) {
    for (auto& p : pairs) {
      set(p.first, std::move(p.second));
    }
  }

  /**
   * Returns a reference to the underlying array.
   * Throws domain_error if this is not an array.
   * This is useful both for iterating the array contents
   * and for returning the size of the array.
   */
  const std::vector<json_ref>& array() const;

  /**
   * Returns a reference to the underlying map object.
   * Throws domain_error if this is not an object.
   * This is useful for iterating over the object contents, etc.
   */
  const std::unordered_map<w_string, json_ref>& object() const;

  /** Returns a reference to the array value at the specified index.
   * Throws out_of_range or domain_error if the index is bad or if
   * this is not an array */
  const json_ref& at(std::size_t idx) const {
    return array().at(idx);
  }

  json_type type() const {
    assert(ref_ != nullptr);
    return ref_->type;
  }

  bool isObject() const {
    return type() == JSON_OBJECT;
  }
  bool isArray() const {
    return type() == JSON_ARRAY;
  }
  bool isString() const {
    return type() == JSON_STRING;
  }
  bool isBool() const {
    return (type() == JSON_TRUE || type() == JSON_FALSE);
  }
  bool isTrue() const {
    return type() == JSON_TRUE;
  }
  bool isFalse() const {
    return type() == JSON_FALSE;
  }
  bool isNull() const {
    return type() == JSON_NULL;
  }
  bool isNumber() const {
    return isInt() || isDouble();
  }
  bool isInt() const {
    return type() == JSON_INTEGER;
  }
  bool isDouble() const {
    return type() == JSON_REAL;
  }

  /**
   * Throws if not a string.
   */
  const w_string& asString() const;

  /**
   * If not a string, returns std::nullopt.
   *
   * A more efficient method would return a nullable pointer.
   */
  std::optional<w_string> asOptionalString() const;

  const char* asCString() const;
  bool asBool() const;
  json_int_t asInt() const;
};

/* construction, destruction, reference counting */

json_ref json_object();
json_ref json_object(std::unordered_map<w_string, json_ref> values);
json_ref json_object(
    std::initializer_list<std::pair<const char*, json_ref>> values);
json_ref json_array(std::vector<json_ref> values);
json_ref json_array(std::initializer_list<json_ref> values);
json_ref w_string_to_json(w_string str);

template <typename... Args>
json_ref typed_string_to_json(Args&&... args) {
  return w_string_to_json(w_string(std::forward<Args>(args)...));
}

const w_string& json_to_w_string(const json_ref& json);
json_ref json_integer(json_int_t value);
json_ref json_real(double value);
json_ref json_true();
json_ref json_false();
#define json_boolean(val) ((val) ? json_true() : json_false())
json_ref json_null();

/* error reporting */

#define JSON_ERROR_TEXT_LENGTH 160
#define JSON_ERROR_SOURCE_LENGTH 80

struct json_error_t {
  int line = 0;
  int column = 0;
  int position = 0;
  char source[JSON_ERROR_SOURCE_LENGTH];
  char text[JSON_ERROR_TEXT_LENGTH];

  json_error_t() {
    source[0] = 0;
    text[0] = 0;
  }

  explicit json_error_t(const char* t) {
    source[0] = 0;
    snprintf(text, sizeof(text), "%s", t);
  }
};

/* getters, setters, manipulation */

size_t json_object_size(const json_ref& object);
std::optional<json_ref> json_object_get(
    const json_ref& object,
    const char* key);
int json_object_set_new(
    const json_ref& object,
    const char* key,
    json_ref&& value);
int json_object_set_new_nocheck(
    const json_ref& object,
    const char* key,
    json_ref&& value);

inline int json_object_set(
    const json_ref& object,
    const char* key,
    const json_ref& value) {
  return json_object_set_new(object, key, json_ref(value));
}

inline int json_object_set_nocheck(
    const json_ref& object,
    const char* key,
    const json_ref& value) {
  return json_object_set_new_nocheck(object, key, json_ref(value));
}

size_t json_array_size(const json_ref& array);
int json_array_set_template_new(const json_ref& json, json_ref&& templ);
std::optional<json_ref> json_array_get_template(const json_ref& array);

const char* json_string_value(const json_ref& string);
json_int_t json_integer_value(const json_ref& integer);
double json_real_value(const json_ref& real);
double json_number_value(const json_ref& json);

#define JSON_VALIDATE_ONLY 0x1
#define JSON_STRICT 0x2

/* equality */

int json_equal(const json_ref& value1, const json_ref& value2);

/* copying */

json_ref json_deep_copy(const json_ref& value);

/* decoding */

#define JSON_REJECT_DUPLICATES 0x1
#define JSON_DISABLE_EOF_CHECK 0x2
#define JSON_DECODE_ANY 0x4

std::optional<json_ref>
json_loads(const char* input, size_t flags, json_error_t* error);
std::optional<json_ref> json_loadb(
    const char* buffer,
    size_t buflen,
    size_t flags,
    json_error_t* error);
std::optional<json_ref>
json_loadf(FILE* input, size_t flags, json_error_t* error);
json_ref json_load_file(const char* path, size_t flags);

/* encoding */

#define JSON_INDENT(n) (n & 0x1F)
#define JSON_COMPACT 0x20
#define JSON_ENSURE_ASCII 0x40
#define JSON_SORT_KEYS 0x80
#define JSON_ENCODE_ANY 0x200
#define JSON_ESCAPE_SLASH 0x400

typedef int (
    *json_dump_callback_t)(const char* buffer, size_t size, void* data);

std::string json_dumps(const json_ref& json, size_t flags);
int json_dumpf(const json_ref& json, FILE* output, size_t flags);
int json_dump_file(const json_ref& json, const char* path, size_t flags);
int json_dump_callback(
    const json_ref& json,
    json_dump_callback_t callback,
    void* data,
    size_t flags);
