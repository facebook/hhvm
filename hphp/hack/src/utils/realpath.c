/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <limits.h>
#include <stdlib.h>

#define Val_none Val_int(0)

#ifdef __cplusplus
extern "C" {
#endif

static value
Val_some( value v )
{
  CAMLparam1( v );
  CAMLlocal1( some );
  some = caml_alloc(1, 0);
  Store_field( some, 0, v );
  CAMLreturn( some );
}

CAMLprim value
hh_realpath(value v) {
  char *input;
  char *result;

  CAMLparam1(v);

  input = String_val(v);
#ifdef _MSC_VER
  char output[_MAX_PATH];
  result = _fullpath(output, input, _MAX_PATH);
#else
  char output[PATH_MAX];
  result = realpath(input, output);
#endif
  if (result == NULL) {
    CAMLreturn(Val_none);
  } else {
    CAMLreturn(Val_some(caml_copy_string(output)));
  }
}

#ifdef __cplusplus
}
#endif