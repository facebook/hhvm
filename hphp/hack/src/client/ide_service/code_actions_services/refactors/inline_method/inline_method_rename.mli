(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

(**
* Enables creating fresh variables that resemble existing variables,
* for use in generated code in the refactor.
*
* Given `used_vars` containing "$a" and "$b",
* `rename_all ["$b"; "$c"]` is `["$b_"; "$c"]
*)

type t

val create : used_vars:String.Set.t -> t

(** idempotent, generates a fresh name not in `used_vars` *)
val rename : t -> string -> t * string

(** idempotent, preserves order *)
val rename_all : t -> string list -> t * string list
