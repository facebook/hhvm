/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jansson.h"
#include "jansson_private.h"
#include "utf.h"

#define MAX_INTEGER_STR_LENGTH 100
#define MAX_REAL_STR_LENGTH 100

struct object_key {
  size_t serial;
  const char* key;
};

static int dump_to_string(const char* buffer, size_t size, void* data) {
  auto str = (std::string*)data;
  str->append(buffer, size);
  return 0;
}

static int dump_to_file(const char* buffer, size_t size, void* data) {
  FILE* dest = (FILE*)data;
  if (fwrite(buffer, size, 1, dest) != 1) {
    return -1;
  }
  return 0;
}

/* 32 spaces (the maximum indentation size) */
static char whitespace[] = "                                ";

static int dump_indent(
    size_t flags,
    int depth,
    int space,
    json_dump_callback_t dump,
    void* data) {
  if (JSON_INDENT(flags) > 0) {
    int i, ws_count = JSON_INDENT(flags);

    if (dump("\n", 1, data))
      return -1;

    for (i = 0; i < depth; i++) {
      if (dump(whitespace, ws_count, data))
        return -1;
    }
  } else if (space && !(flags & JSON_COMPACT)) {
    return dump(" ", 1, data);
  }
  return 0;
}

static int dump_string(
    const char* str,
    json_dump_callback_t dump,
    void* data,
    size_t flags) {
  const char *pos, *end;
  int32_t codepoint;

  if (dump("\"", 1, data))
    return -1;

  end = pos = str;
  while (1) {
    const char* text;
    char seq[13];
    int length;

    while (*end) {
      end = utf8_iterate(pos, &codepoint);
      if (!end) {
        return -1;
      }

      /* mandatory escape or control char */
      if (codepoint == '\\' || codepoint == '"' || codepoint < 0x20)
        break;

      /* slash */
      if ((flags & JSON_ESCAPE_SLASH) && codepoint == '/')
        break;

      /* non-ASCII */
      if ((flags & JSON_ENSURE_ASCII) && codepoint > 0x7F)
        break;

      pos = end;
    }

    if (pos != str) {
      if (dump(str, pos - str, data))
        return -1;
    }

    if (end == pos)
      break;

    /* handle \, /, ", and control codes */
    length = 2;
    switch (codepoint) {
      case '\\':
        text = "\\\\";
        break;
      case '\"':
        text = "\\\"";
        break;
      case '\b':
        text = "\\b";
        break;
      case '\f':
        text = "\\f";
        break;
      case '\n':
        text = "\\n";
        break;
      case '\r':
        text = "\\r";
        break;
      case '\t':
        text = "\\t";
        break;
      case '/':
        text = "\\/";
        break;
      default: {
        /* codepoint is in BMP */
        if (codepoint < 0x10000) {
          sprintf(seq, "\\u%04x", codepoint);
          length = 6;
        }

        /* not in BMP -> construct a UTF-16 surrogate pair */
        else {
          int32_t first, last;

          codepoint -= 0x10000;
          first = 0xD800 | ((codepoint & 0xffc00) >> 10);
          last = 0xDC00 | (codepoint & 0x003ff);

          sprintf(seq, "\\u%04x\\u%04x", first, last);
          length = 12;
        }

        text = seq;
        break;
      }
    }

    if (dump(text, length, data))
      return -1;

    str = pos = end;
  }

  return dump("\"", 1, data);
}

static int do_dump(
    const json_ref& json,
    size_t flags,
    int depth,
    json_dump_callback_t dump,
    void* data) {
  switch (json.type()) {
    case JSON_NULL:
      return dump("null", 4, data);

    case JSON_TRUE:
      return dump("true", 4, data);

    case JSON_FALSE:
      return dump("false", 5, data);

    case JSON_INTEGER: {
      char buffer[MAX_INTEGER_STR_LENGTH];
      int size;

      size = snprintf(
          buffer,
          MAX_INTEGER_STR_LENGTH,
          "%" JSON_INTEGER_FORMAT,
          json_integer_value(json));
      if (size < 0 || size >= MAX_INTEGER_STR_LENGTH) {
        return -1;
      }

      return dump(buffer, size, data);
    }

    case JSON_REAL: {
      char buffer[MAX_REAL_STR_LENGTH];
      int size;
      double value = json_real_value(json);

      size = jsonp_dtostr(buffer, MAX_REAL_STR_LENGTH, value);
      if (size < 0) {
        return -1;
      }

      return dump(buffer, size, data);
    }

    case JSON_STRING:
      return dump_string(json_string_value(json), dump, data, flags);

    case JSON_ARRAY: {
      auto& arr = json.array();

      if (dump("[", 1, data)) {
        return -1;
      }
      if (arr.size() == 0) {
        return dump("]", 1, data);
      }
      if (dump_indent(flags, depth + 1, 0, dump, data))
        return -1;

      for (size_t i = 0; i < arr.size(); ++i) {
        if (do_dump(arr[i], flags, depth + 1, dump, data)) {
          return -1;
        }

        if (i < arr.size() - 1) {
          if (dump(",", 1, data) ||
              dump_indent(flags, depth + 1, 1, dump, data)) {
            return -1;
          }
        } else {
          if (dump_indent(flags, depth, 0, dump, data)) {
            return -1;
          }
        }
      }

      return dump("]", 1, data);
    }

    case JSON_OBJECT: {
      json_object_t* object;
      const char* separator;
      int separator_length;

      if (flags & JSON_COMPACT) {
        separator = ":";
        separator_length = 1;
      } else {
        separator = ": ";
        separator_length = 2;
      }

      object = json_to_object(json.get());
      auto it = object->map.begin();

      if (dump("{", 1, data)) {
        return -1;
      }
      if (object->map.empty()) {
        return dump("}", 1, data);
      }

      if (dump_indent(flags, depth + 1, 0, dump, data)) {
        return -1;
      }

      if (flags & JSON_SORT_KEYS) {
        using Pair = std::pair<const w_string, json_ref>;

        std::vector<Pair*> items;
        items.reserve(object->map.size());
        for (auto& item : object->map) {
          items.push_back(&item);
        }

        std::sort(items.begin(), items.end(), [](const Pair* a, const Pair* b) {
          return a->first < b->first;
        });

        auto sorted_it = items.begin();
        while (sorted_it != items.end()) {
          auto next = std::next(sorted_it);

          dump_string((*sorted_it)->first.c_str(), dump, data, flags);
          if (dump(separator, separator_length, data) ||
              do_dump((*sorted_it)->second, flags, depth + 1, dump, data)) {
            return -1;
          }

          if (next != items.end()) {
            if (dump(",", 1, data) ||
                dump_indent(flags, depth + 1, 1, dump, data)) {
              return -1;
            }
          } else {
            if (dump_indent(flags, depth, 0, dump, data)) {
              return -1;
            }
          }

          sorted_it = next;
        }
      } else {
        while (it != object->map.end()) {
          auto next = std::next(it);

          dump_string(it->first.c_str(), dump, data, flags);
          if (dump(separator, separator_length, data) ||
              do_dump(it->second, flags, depth + 1, dump, data)) {
            return -1;
          }

          if (next != object->map.end()) {
            if (dump(",", 1, data) ||
                dump_indent(flags, depth + 1, 1, dump, data)) {
              return -1;
            }
          } else {
            if (dump_indent(flags, depth, 0, dump, data)) {
              return -1;
            }
          }

          it = next;
        }
      }
      return dump("}", 1, data);
    }

    default:
      /* not reached */
      return -1;
  }
}

std::string json_dumps(const json_ref& json, size_t flags) {
  std::string strbuff;

  if (json_dump_callback(json, dump_to_string, (void*)&strbuff, flags)) {
    throw std::runtime_error("json_dumps failed");
  }
  return strbuff;
}

int json_dumpf(const json_ref& json, FILE* output, size_t flags) {
  return json_dump_callback(json, dump_to_file, (void*)output, flags);
}

int json_dump_file(const json_ref& json, const char* path, size_t flags) {
  std::unique_ptr<FILE, int (*)(FILE*)> output{fopen(path, "w"), &fclose};
  if (!output)
    return -1;

  return json_dumpf(json, output.get(), flags);
}

int json_dump_callback(
    const json_ref& json,
    json_dump_callback_t callback,
    void* data,
    size_t flags) {
  if (!(flags & JSON_ENCODE_ANY)) {
    if (!json.isArray() && !json.isObject())
      return -1;
  }

  return do_dump(json, flags, 0, callback, data);
}
