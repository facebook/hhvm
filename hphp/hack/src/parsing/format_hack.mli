(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type return =
  | Php_or_decl
  | Parsing_error of Errors.error list
  | Internal_error
  | Success of string


val region: start:int -> end_:int -> string -> return
val program: string -> return
