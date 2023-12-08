/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
#define CAML_NAME_SPACE
#include <caml/memory.h>
#include <caml/alloc.h>
#include <string.h>

extern const char* const build_id;

value hh_get_compiler_id(void) {
  CAMLparam0();
  CAMLlocal1(result);
  const char* const buf = build_id;
  const ssize_t len = strlen(buf);

  result = caml_alloc_initialized_string(len, buf);
  CAMLreturn(result);
}
