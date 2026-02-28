/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <assert.h>
#include <errno.h>
#include <fmt/compile.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include "jansson_private.h"

#ifdef __APPLE__
#include <xlocale.h>
#endif

namespace {
#ifdef _WIN32
_locale_t getCLocale() {
  static _locale_t c = _create_locale(LC_ALL, "C");
  return c;
}
#else
locale_t getCLocale() {
  static locale_t c = newlocale(LC_ALL_MASK, "C", (locale_t)0);
  return c;
}
#endif
} // namespace

int jsonp_strtod(std::string& strbuffer, double* out) {
#ifdef _WIN32
  char* end;
  double result = _strtod_l(strbuffer.c_str(), &end, getCLocale());
  if (strbuffer.c_str() == end) {
    return -1;
  }
  *out = result;
  return 0;
#else
  char* end;
  double result = strtod_l(strbuffer.c_str(), &end, getCLocale());
  if (strbuffer.c_str() == end) {
    return -1;
  }

  *out = result;
  return 0;
#endif
}

int jsonp_dtostr(char* buffer, size_t size, double value) {
  auto result = fmt::format_to_n(buffer, size, FMT_COMPILE("{:.17g}"), value);
  if (result.size >= size) {
    return -1;
  }
  // If `value` is integral, buffer may not contain a . or e, so add one. This
  // avoids parsing a double as an integer, even though the JSON spec does not
  // differentiate between types of numbers.
  if (nullptr == memchr(buffer, '.', result.size) &&
      nullptr == memchr(buffer, 'e', result.size)) {
    if (result.size + 2 >= size) {
      return -1;
    }
    buffer[result.size] = '.';
    buffer[result.size + 1] = '0';
    return result.size + 2;
  }
  return result.size;
}
