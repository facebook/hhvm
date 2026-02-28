/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h> // @manual=fbsource//third-party/python:python

#ifdef __cplusplus
extern "C" {
#endif

#define BSER_ARRAY 0x00
#define BSER_OBJECT 0x01
#define BSER_BYTESTRING 0x02
#define BSER_INT8 0x03
#define BSER_INT16 0x04
#define BSER_INT32 0x05
#define BSER_INT64 0x06
#define BSER_REAL 0x07
#define BSER_TRUE 0x08
#define BSER_FALSE 0x09
#define BSER_NULL 0x0a
#define BSER_TEMPLATE 0x0b
#define BSER_SKIP 0x0c
#define BSER_UTF8STRING 0x0d

// An immutable object representation of BSER_OBJECT.
// Rather than build a hash table, key -> value are obtained
// by walking the list of keys to determine the offset into
// the values array.  The assumption is that the number of
// array elements will be typically small (~6 for the top
// level query result and typically 3-5 for the file entries)
// so that the time overhead for this is small compared to
// using a proper hash table.  Even with this simplistic
// approach, this is still faster for the mercurial use case
// as it helps to eliminate creating N other objects to
// represent the stat information in the hgwatchman extension
// clang-format off
typedef struct {
  PyObject_HEAD
  PyObject *keys;   // tuple of field names
  PyObject *values; // tuple of values
} bserObject;

extern PyTypeObject bserObjectType;

typedef struct loads_ctx {
  int is_mutable;
  const char* value_encoding;
  const char* value_errors;
  uint32_t bser_version;
  uint32_t bser_capabilities;
} unser_ctx_t;

int bunser_int(const char** ptr, const char* end, int64_t* val);

PyObject*
bser_loads_recursive(const char** ptr, const char* end, const unser_ctx_t* ctx);

#ifdef __cplusplus
}
#endif
