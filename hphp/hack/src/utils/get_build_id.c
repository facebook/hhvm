/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#define _XOPEN_SOURCE

#define CAML_NAME_SPACE
#include <caml/memory.h>
#include <caml/alloc.h>

#include <assert.h>
#include <string.h>

extern const char* const BuildInfo_kRevision;

/**
 * Export the constant provided by Facebook's build system to ocaml-land, since
 * their FFI only allows you to call functions, not reference variables. Doing
 * it this way makes sense for Facebook internally since our build system has
 * machinery for providing this constant automatically (and no machinery for
 * doing codegen in a consistent way to build an ocaml file with them) but is
 * very roundabout for external users who have to have CMake codegen this
 * constant anyways. Sorry about that.
 */
value hh_get_build_revision(void) {
  CAMLparam0();
  CAMLlocal1(result);

  size_t len = strlen(BuildInfo_kRevision);
  result = caml_alloc_string(len);

  memcpy(String_val(result), BuildInfo_kRevision, len);
  CAMLreturn(result);
}
