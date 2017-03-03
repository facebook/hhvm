(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

exception InvalidSyntax
exception UnsupportedSyntax of string

let get_exception_exit_value = function
  | Failure _ -> 1
  | InvalidSyntax -> 2
  | UnsupportedSyntax _ -> 3
  | _ -> 4

let get_error_string_from_exit_value = function
  | 1 -> "Internal Error"
  | 2 -> "File failed to parse without errors"
  | 3 -> "File contains unsupported syntax"
  | _ -> "Internal Error"
