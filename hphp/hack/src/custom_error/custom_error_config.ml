(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  valid: Custom_error.t list;
  invalid: Custom_error.t list;
}
[@@deriving eq, show] [@@boxed]

let empty = { valid = []; invalid = [] }

let is_valid = function
  | { invalid = []; _ } -> true
  | _ -> false

external initialize_custom_error_config : string -> (t, string) Result.t
  = "initialize_custom_error_config"

let initialize path =
  let abs_path =
    match path with
    | `Absolute path -> path
    | `Relative path -> Relative_path.to_absolute path
  in

  initialize_custom_error_config abs_path
