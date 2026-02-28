/* Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "jansson.h"
#include "jansson_private.h"

#include <algorithm>
#include <cmath>
#include <fmt/core.h>
#include <string>
#include <sstream>

#include "utf.h"
#include "watchman/watchman_string.h"

namespace {

const char* getTypeName(json_type t) {
  switch (t) {
    case JSON_OBJECT:
      return "object";
    case JSON_ARRAY:
      return "array";
    case JSON_STRING:
      return "string";
    case JSON_INTEGER:
      return "integer";
    case JSON_REAL:
      return "real";
    case JSON_TRUE:
      return "true";
    case JSON_FALSE:
      return "false";
    case JSON_NULL:
      return "null";
  }
  return "<unknown>";
}

static json_t the_null{JSON_NULL, json_t::SingletonHack()};
static json_t the_false{JSON_FALSE, json_t::SingletonHack()};
static json_t the_true{JSON_TRUE, json_t::SingletonHack()};

} // namespace

json_ref::~json_ref() {
  decref();
}

void json_ref::reset() {
  decref();

  ref_ = &the_null;

  // No-op, but leave in for clarity. Shouldn't matter.
  incref();
}

json_ref::json_ref(const json_ref& other) : ref_(other.ref_) {
  incref();
}

json_ref& json_ref::operator=(const json_ref& other) {
  if (ref_ != other.ref_) {
    decref();
    ref_ = other.ref_;
    incref();
  }
  return *this;
}

json_ref::json_ref(json_ref&& other) noexcept : ref_(other.ref_) {
  other.ref_ = &the_null;
}

json_ref& json_ref::operator=(json_ref&& other) noexcept {
  if (ref_ != other.ref_) {
    decref();
    ref_ = std::exchange(other.ref_, &the_null);
  }
  return *this;
}

json_t::json_t(json_type type) : type(type), refcount(1) {}

json_t::json_t(json_type type, json_t::SingletonHack&&)
    : type(type), refcount(-1) {}

const w_string& json_ref::asString() const {
  if (!isString()) {
    throw std::domain_error(
        fmt::format("json_ref expected string, got {}", getTypeName(type())));
  }
  return json_to_string(ref_)->value;
}

std::optional<w_string> json_ref::asOptionalString() const {
  if (!isString()) {
    return std::nullopt;
  }
  return json_to_string(ref_)->value;
}

std::string json_ref::toString() const {
  switch (this->type()) {
    case JSON_OBJECT:
    {
      std::stringstream ss;
      const auto& obj = this->object();
      ss << "{";
      for (auto itr = obj.begin(); itr != obj.end(); itr++) {
        ss << itr->first.c_str() << ":" << itr->second.toString() << ",";
      }
      if (obj.size() > 0) {
        // remove last comma
        ss.seekp(-1, std::ios_base::end);
      }
      ss << "}";
      return ss.str();
    }
    case JSON_ARRAY:
    {
      std::stringstream ss;
      const auto& arr = this->array();
      ss << "[";
      for (const auto& elem: arr) {
        ss << elem.toString() << ",";
      }
      if (arr.size() > 0) {
        // remove last comma
        ss.seekp(-1, std::ios_base::end);
      }
      ss << "]";
      return ss.str();
    }
    case JSON_STRING:
      return fmt::format("\"{}\"", json_string_value(*this));
    case JSON_INTEGER:
    case JSON_REAL:
      return std::to_string(json_number_value(*this));
    case JSON_TRUE:
      return "true";
    case JSON_FALSE:
      return "false";
    case JSON_NULL:
      return "null";
  }
  return std::string();
}

const char* json_ref::asCString() const {
  return asString().c_str();
}

bool json_ref::asBool() const {
  switch (type()) {
    case JSON_TRUE:
      return true;
    case JSON_FALSE:
      return false;
    default:
      throw std::domain_error(
          fmt::format("asBool called on non-boolean: {}", getTypeName(type())));
  }
}

/*** object ***/

const std::unordered_map<w_string, json_ref>& json_ref::object() const {
  if (type() != JSON_OBJECT) {
    throw std::domain_error("json_ref::object() called for non-object");
  }
  return json_to_object(ref_)->map;
}

json_object_t::json_object_t(std::unordered_map<w_string, json_ref> values)
    : json_t{JSON_OBJECT}, map{std::move(values)} {}

json_ref json_object(std::unordered_map<w_string, json_ref> values) {
  return json_ref::takeOwnership(new json_object_t(std::move(values)));
}

json_ref json_object(
    std::initializer_list<std::pair<const char*, json_ref>> values) {
  std::unordered_map<w_string, json_ref> object;
  object.reserve(values.size());

  for (auto& it : values) {
    object.emplace(w_string{it.first, W_STRING_UNICODE}, it.second);
  }

  return json_object(std::move(object));
}

json_ref json_object() {
  return json_ref::takeOwnership(new json_object_t{{}});
}

size_t json_object_size(const json_ref& json) {
  if (!json.isObject()) {
    return 0;
  }

  return json_to_object(json.get())->map.size();
}

typename std::unordered_map<w_string, json_ref>::iterator
json_object_t::findCString(const char* key) {
  w_string key_string(key, W_STRING_BYTE);
  return map.find(key_string);
}

json_ref json_ref::get_default(const char* key, json_ref defval) const {
  if (type() != JSON_OBJECT) {
    return defval;
  }
  auto object = json_to_object(ref_);
  auto it = object->findCString(key);
  if (it == object->map.end()) {
    return defval;
  }
  return it->second;
}

const json_ref& json_ref::get(const char* key) const {
  if (type() != JSON_OBJECT) {
    throw std::domain_error("json_ref::get called on a non object type");
  }
  auto object = json_to_object(ref_);
  auto it = object->findCString(key);
  if (it == object->map.end()) {
    throw std::range_error(
        std::string("key '") + key + "' is not present in this json object");
  }
  return it->second;
}

std::optional<json_ref> json_ref::get_optional(const char* key) const {
  if (type() != JSON_OBJECT) {
    return std::nullopt;
  }

  auto object = json_to_object(ref_);
  auto it = object->findCString(key);
  if (it == object->map.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::optional<json_ref> json_object_get(const json_ref& json, const char* key) {
  if (!json.isObject()) {
    return std::nullopt;
  }

  auto* object = json_to_object(json.get());
  auto it = object->findCString(key);
  if (it == object->map.end()) {
    return std::nullopt;
  }
  return it->second;
}

int json_object_set_new_nocheck(
    const json_ref& json,
    const char* key,
    json_ref&& value) {
  if (!json.isObject()) {
    return -1;
  }

  if (!key || json.get() == value.get()) {
    return -1;
  }
  auto* object = json_to_object(json.get());

  object->map.insert_or_assign(w_string{key}, std::move(value));
  return 0;
}

void json_ref::set(const w_string& key, json_ref&& val) {
  json_to_object(ref_)->map.insert_or_assign(key, std::move(val));
}

void json_ref::set(const char* key, json_ref&& val) {
#if 0 // circular build dep
  w_assert(key != nullptr, "json_ref::set called with NULL key");
  w_assert(ref_ != nullptr, "json_ref::set called NULL object");
  w_assert(val != ref_, "json_ref::set cannot create cycle");
  w_assert(json_is_object(ref_), "json_ref::set called for non object type");
#endif

  json_to_object(ref_)->map.insert_or_assign(w_string{key}, std::move(val));
}

int json_object_set_new(
    const json_ref& json,
    const char* key,
    json_ref&& value) {
  if (!key || !utf8_check_string(key, -1)) {
    return -1;
  }

  return json_object_set_new_nocheck(json, key, std::move(value));
}

static int json_object_equal(const json_ref& object1, const json_ref& object2) {
  if (json_object_size(object1) != json_object_size(object2))
    return 0;

  auto target_obj = json_to_object(object2.get());
  for (auto& it : json_to_object(object1.get())->map) {
    auto other_it = target_obj->map.find(it.first);

    if (other_it == target_obj->map.end()) {
      return 0;
    }

    if (!json_equal(it.second, other_it->second)) {
      return 0;
    }
  }

  return 1;
}

static json_ref json_object_deep_copy(const json_ref& object) {
  json_ref result = json_object();

  auto target_obj = json_to_object(result.get());
  for (auto& it : json_to_object(object.get())->map) {
    target_obj->map.insert_or_assign(it.first, json_deep_copy(it.second));
  }

  return result;
}

/*** array ***/

json_array_t::json_array_t(std::vector<json_ref> values)
    : json_t(JSON_ARRAY), table{std::move(values)} {}

json_array_t::json_array_t(std::initializer_list<json_ref> values)
    : json_t(JSON_ARRAY), table(values) {}

const std::vector<json_ref>& json_ref::array() const {
  if (!isArray()) {
    throw std::domain_error("json_ref::array() called for non-array");
  }
  return json_to_array(ref_)->table;
}

json_ref json_array(std::vector<json_ref> values) {
  return json_ref::takeOwnership(new json_array_t(std::move(values)));
}

json_ref json_array(std::initializer_list<json_ref> values) {
  return json_ref::takeOwnership(new json_array_t(std::move(values)));
}

int json_array_set_template(const json_ref& json, const json_ref& templ) {
  return json_array_set_template_new(json, json_ref(templ));
}

int json_array_set_template_new(const json_ref& json, json_ref&& templ) {
  if (!json.isArray()) {
    return 0;
  }
  json_to_array(json.get())->templ = std::move(templ);
  return 1;
}

std::optional<json_ref> json_array_get_template(const json_ref& array) {
  if (!array.isArray()) {
    return std::nullopt;
  }
  return json_to_array(array.get())->templ;
}

size_t json_array_size(const json_ref& json) {
  if (!json.isArray()) {
    return 0;
  }

  return json_to_array(json.get())->table.size();
}

static int json_array_equal(const json_ref& array1, const json_ref& array2) {
  auto& arr1 = array1.array();
  auto& arr2 = array2.array();

  if (arr1.size() != arr2.size()) {
    return 0;
  }

  for (size_t i = 0; i < arr1.size(); ++i) {
    if (!json_equal(arr1[i], arr2[i])) {
      return 0;
    }
  }

  return 1;
}

static json_ref json_array_deep_copy(const json_ref& array) {
  std::vector<json_ref> result;

  for (auto& elem : array.array())
    result.push_back(json_deep_copy(elem));

  return json_array(std::move(result));
}

/*** string ***/

json_string_t::json_string_t(w_string str)
    : json_t(JSON_STRING), value(std::move(str)) {}

json_ref w_string_to_json(w_string str) {
  return json_ref::takeOwnership(new json_string_t(str));
}

const char* json_string_value(const json_ref& json) {
  if (!json.isString()) {
    return nullptr;
  }

  return json_to_string(json.get())->value.c_str();
}

const w_string& json_to_w_string(const json_ref& json) {
  if (!json.isString()) {
    throw std::runtime_error("expected json string object");
  }

  return json_to_string(json.get())->value;
}

static int json_string_equal(const json_ref& string1, const json_ref& string2) {
  return json_to_string(string1.get())->value ==
      json_to_string(string2.get())->value;
}

/*** integer ***/

json_integer_t::json_integer_t(json_int_t value)
    : json_t(JSON_INTEGER), value(value) {}

json_ref json_integer(json_int_t value) {
  return json_ref::takeOwnership(new json_integer_t(value));
}

json_int_t json_integer_value(const json_ref& json) {
  if (!json.isInt()) {
    return 0;
  }
  return json_to_integer(json.get())->value;
}

json_int_t json_ref::asInt() const {
  return json_integer_value(*this);
}

static int json_integer_equal(
    const json_ref& integer1,
    const json_ref& integer2) {
  return json_integer_value(integer1) == json_integer_value(integer2);
}

/*** real ***/

json_real_t::json_real_t(double value) : json_t(JSON_REAL), value(value) {}

json_ref json_real(double value) {
  if (!std::isfinite(value)) {
    throw std::domain_error("Numeric JSON values must be finite");
  }
  return json_ref::takeOwnership(new json_real_t(value));
}

double json_real_value(const json_ref& json) {
  if (!json.isDouble()) {
    return 0;
  }

  return json_to_real(json.get())->value;
}

static int json_real_equal(const json_ref& real1, const json_ref& real2) {
  return json_real_value(real1) == json_real_value(real2);
}

/*** number ***/

double json_number_value(const json_ref& json) {
  if (json.isInt())
    return (double)json_integer_value(json);
  else if (json.isDouble())
    return json_real_value(json);
  else
    return 0.0;
}

/*** simple values ***/

json_ref json_true() {
  return json_ref::takeOwnership(&the_true);
}

json_ref json_false() {
  return json_ref::takeOwnership(&the_false);
}

json_ref json_null() {
  return json_ref::takeOwnership(&the_null);
}

/*** deletion ***/

void json_ref::json_delete(json_t* json) {
  switch (json->type) {
    case JSON_OBJECT:
      delete (json_object_t*)json;
      break;
    case JSON_ARRAY:
      delete (json_array_t*)json;
      break;
    case JSON_STRING:
      delete (json_string_t*)json;
      break;
    case JSON_INTEGER:
      delete (json_integer_t*)json;
      break;
    case JSON_REAL:
      delete (json_real_t*)json;
      break;
    case JSON_TRUE:
    case JSON_FALSE:
    case JSON_NULL:
      break;
  }
}

/*** equality ***/

int json_equal(const json_ref& json1, const json_ref& json2) {
  if (json1.type() != json2.type())
    return 0;

  /* this covers true, false and null as they are singletons */
  if (json1.get() == json2.get())
    return 1;

  if (json1.isObject())
    return json_object_equal(json1, json2);

  if (json1.isArray())
    return json_array_equal(json1, json2);

  if (json1.isString())
    return json_string_equal(json1, json2);

  if (json1.isInt())
    return json_integer_equal(json1, json2);

  if (json1.isDouble())
    return json_real_equal(json1, json2);

  return 0;
}

/*** copying ***/

json_ref json_deep_copy(const json_ref& json) {
  if (json.isObject())
    return json_object_deep_copy(json);

  if (json.isArray())
    return json_array_deep_copy(json);

  // For the rest of the types, the values are immutable, so just increment the
  // reference count.

  return json;
}
