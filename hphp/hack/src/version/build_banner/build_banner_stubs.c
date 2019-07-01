/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

#define CAML_NAME_SPACE
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#include <string.h>

#ifdef HH_BUILD_BANNER
#include "hphp/runtime/version.h"
#endif

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY_VALUE(x) STRINGIFY_HELPER(x)

CAMLprim value hh_get_build_banner(void) {
  CAMLparam0();
  CAMLlocal2(result, option);

#ifdef HH_BUILD_BANNER
  const char* const buf =
    STRINGIFY_VALUE(HH_BUILD_BANNER) "-" HHVM_VERSION_C_STRING_LITERALS;
  const size_t len = strlen(buf);
  result = caml_alloc_string(len);
  memcpy(String_val(result), buf, len);
  option = caml_alloc(1, 0); // Some result
  Store_field(option, 0, result);
#else
  option = Val_int(0); // None
#endif

  CAMLreturn(option);
}
