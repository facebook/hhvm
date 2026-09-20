/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 */

#define CAML_INTERNALS
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/signals.h>

extern value hh_rust_provider_backend_naming_types_get_pos(value, value);
extern value hh_rust_provider_backend_naming_funs_get_pos(value, value);
extern value hh_rust_provider_backend_naming_consts_get_pos(value, value);
extern value hh_rust_provider_backend_naming_modules_get_pos(value, value);

typedef value (*naming_lookup)(value, value);

CAMLprim value naming_lookup_with_pending_gc(
    value kind,
    value backend,
    value name) {
  CAMLparam3(kind, backend, name);
  CAMLlocal1(result);
  static naming_lookup const lookups[] = {
      hh_rust_provider_backend_naming_types_get_pos,
      hh_rust_provider_backend_naming_funs_get_pos,
      hh_rust_provider_backend_naming_consts_get_pos,
      hh_rust_provider_backend_naming_modules_get_pos,
  };
  const intnat index = Int_val(kind);
  if (index < 0 || index >= (intnat)(sizeof(lookups) / sizeof(*lookups))) {
    caml_invalid_argument("naming lookup kind");
  }
  const value before = name;
  /* caml_input_value_from_block processes pending actions in intern_end, even
   * for an immediate value, so this collects after Rust borrows name. */
  caml_request_minor_gc();
  result = lookups[index](backend, name);
  /* The FFI returns a bare [pos option]: a cached absence is immediate None. */
  CAMLreturn(Val_bool(name != before && result == Val_int(0)));
}
