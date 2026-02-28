/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
#define CAML_NAME_SPACE
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#include "sha1c.h"

value sha1sum(value data) {
  CAMLparam1(data);
  CAMLlocal1(result);

  const char *msg = String_val(data);
  size_t msglen = caml_string_length(data);

  struct sha1_ctx ctx;

  sha1_init(&ctx);
  sha1_update(&ctx, (unsigned char *)msg, msglen);

  sha1_digest digest;
  sha1_finalize(&ctx, &digest);

  char hex[41];
  sha1_to_hex(&digest, hex);

  result = caml_copy_string(hex);
  CAMLreturn(result);
}
