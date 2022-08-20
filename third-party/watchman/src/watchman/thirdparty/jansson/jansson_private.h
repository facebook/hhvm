/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JANSSON_PRIVATE_H
#define JANSSON_PRIVATE_H

#include <stddef.h>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include "jansson.h"

struct json_object_t : json_t {
  std::unordered_map<w_string, json_ref> map;

  explicit json_object_t(std::unordered_map<w_string, json_ref> values);

  typename std::unordered_map<w_string, json_ref>::iterator findCString(
      const char* key);
};

struct json_array_t : json_t {
  std::vector<json_ref> table;
  std::optional<json_ref> templ;

  json_array_t(std::vector<json_ref> values);
  json_array_t(std::initializer_list<json_ref> values);
};

struct json_string_t : json_t {
  w_string value;

  json_string_t(w_string str);
};

struct json_real_t : json_t {
  double value;

  json_real_t(double value);
};

struct json_integer_t : json_t {
  json_int_t value;

  json_integer_t(json_int_t value);
};

inline json_object_t* json_to_object(const json_t* json) {
  return static_cast<json_object_t*>(const_cast<json_t*>(json));
}

inline json_array_t* json_to_array(const json_t* json) {
  return static_cast<json_array_t*>(const_cast<json_t*>(json));
}

inline json_string_t* json_to_string(const json_t* json) {
  return static_cast<json_string_t*>(const_cast<json_t*>(json));
}

inline json_real_t* json_to_real(const json_t* json) {
  return static_cast<json_real_t*>(const_cast<json_t*>(json));
}

inline json_integer_t* json_to_integer(const json_t* json) {
  return static_cast<json_integer_t*>(const_cast<json_t*>(json));
}

void jsonp_error_init(json_error_t* error, const char* source);
void jsonp_error_set_source(json_error_t* error, const char* source);
void jsonp_error_set(
    json_error_t* error,
    int line,
    int column,
    size_t position,
    const char* msg,
    ...);
void jsonp_error_vset(
    json_error_t* error,
    int line,
    int column,
    size_t position,
    const char* msg,
    va_list ap);

/* Locale independent string<->double conversions */
int jsonp_strtod(std::string& strbuffer, double* out);
int jsonp_dtostr(char* buffer, size_t size, double value);

/* Windows compatibility */
#ifdef _WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

#endif
