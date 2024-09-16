(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module FixmeMap : sig
  type t = Pos.t IMap.t IMap.t
end

val get_hh_fixmes : Relative_path.t -> FixmeMap.t option

val get_decl_hh_fixmes : Relative_path.t -> FixmeMap.t option

val get_fixme_codes_for_pos : Pos.t -> ISet.t

val provide_hh_fixmes : Relative_path.t -> FixmeMap.t -> unit

val provide_decl_hh_fixmes : Relative_path.t -> FixmeMap.t -> unit

val provide_disallowed_fixmes : Relative_path.t -> FixmeMap.t -> unit

val provide_ignores : Relative_path.t -> FixmeMap.t -> unit

val remove_batch : Relative_path.Set.t -> unit

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit

module UnusedFixmes : sig
  val get :
    codes:int list ->
    applied_fixmes:(Relative_path.t Pos.pos * int) list ->
    fold:
      ('files ->
      init:Pos.t list ->
      f:(Relative_path.t -> 'unused -> Pos.t list -> Pos.t list) ->
      Pos.t list) ->
    files:'files ->
    Pos.t list
end
