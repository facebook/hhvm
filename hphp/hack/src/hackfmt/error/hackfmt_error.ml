(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception InvalidSyntax

exception InvalidCliArg of string

exception InvalidDiff of string

let get_exception_exit_value = function
  | InvalidSyntax -> 2
  | InvalidCliArg _ -> 3
  | InvalidDiff _ -> 4
  | _ -> 255

let get_error_string_from_exit_value = function
  | 2 -> "File failed to parse without errors"
  | 3 -> "Invalid argument"
  | 4 -> "Invalid diff"
  | _ -> "Internal Error"

let get_error_string_from_exn exn =
  get_exception_exit_value exn |> get_error_string_from_exit_value
