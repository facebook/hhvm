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
#include <caml/unixsupport.h>
#include <caml/intext.h>
#include <caml/custom.h>
#include <stdio.h>

#define Handle_val(fd) (Long_val(fd))
#define Val_handle(fd) (Val_long(fd))

value caml_hh_worker_get_handle(value x) {
    return Val_long(Handle_val(x));
}

value caml_hh_worker_create_handle(value x) {
  return Val_handle(Long_val(x));
}

value win_setup_handle_serialization(value unit) {
  (void)unit; // Dear compiler, please ignore this param
  return Val_unit;
}
