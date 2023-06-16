(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = { custom_errors: Custom_error.t list } [@@deriving eq, show]

val empty : t

val initialize :
  [ `Absolute of string | `Relative of Relative_path.t ] ->
  (t * string list, string) result
