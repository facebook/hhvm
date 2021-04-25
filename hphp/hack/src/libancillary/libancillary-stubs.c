/**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/signals.h>
#include <caml/callback.h>
#undef CAML_NAME_SPACE

#include <errno.h>
#include <string.h>
#include <stdio.h>


#include "hphp/hack/src/third-party/libancillary/ancillary.h"

CAMLprim value stub_ancil_send_fd(value int_val_socket, value int_val_fd) {
  CAMLparam2(int_val_socket, int_val_fd);
  int socket = Int_val(int_val_socket);
  int fd = Int_val(int_val_fd);
  CAMLreturn(Val_int(ancil_send_fd(socket, fd)));
}

/** Returns -1 on failure, or non-negative file descriptor on success. */
CAMLprim value stub_ancil_recv_fd(value int_val_socket) {
  CAMLparam1(int_val_socket);
  CAMLlocal3(ret, result_val, error_val);
  int fd = 0;
  int result;
  int socket = Int_val(int_val_socket);
  int error_code;
  char errmsg[64];

  result = ancil_recv_fd(socket, &fd);
  error_code = errno; /* read errno here to make sure we don't loose it */

  if (result >= 0) {
    error_val = caml_copy_string("");
    result_val = Val_int(fd);
  } else {
    snprintf(errmsg, sizeof(errmsg), "(errno=%d) %s", error_code, strerror(error_code));
    error_val = caml_copy_string(errmsg);
    result_val = Val_int(result);
  }
  ret = caml_alloc_tuple(2);
  Store_field(ret, 0, result_val);
  Store_field(ret, 1, error_val);
  CAMLreturn(ret);
}
