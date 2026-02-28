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

val empty : t

val is_valid : t -> bool

val initialize :
  [ `Absolute of string | `Relative of Relative_path.t ] -> (t, string) result
