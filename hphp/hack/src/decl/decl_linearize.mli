(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get_linearization :
  ?kind:Decl_defs.linearization_kind ->
  Provider_context.t ->
  string ->
  Decl_defs.linearization

val push_local_changes : unit -> unit

val pop_local_changes : unit -> unit

val remove_batch : SSet.t -> unit
