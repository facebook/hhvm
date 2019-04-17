(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Decl_defs

type linearization_kind =
  | Member_resolution
  | Ancestor_types
  [@@deriving show]

val get_linearization : ?kind:linearization_kind -> string -> linearization

val push_local_changes : unit -> unit
val pop_local_changes : unit -> unit

val remove_batch : SSet.t -> unit
