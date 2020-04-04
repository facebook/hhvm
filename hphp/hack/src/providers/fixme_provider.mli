(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type fixme_map = Pos.t IMap.t IMap.t

val get_fixmes : Relative_path.t -> fixme_map option

val get_hh_fixmes : Relative_path.t -> fixme_map option

val get_decl_hh_fixmes : Relative_path.t -> fixme_map option

val get_disallowed_fixmes : Relative_path.t -> fixme_map option

val get_fixme_codes_for_pos : Pos.t -> ISet.t

val get_unused_fixmes :
  codes:int list ->
  applied_fixmes:(Relative_path.t Pos.pos * int) list ->
  fold:
    ('a ->
    init:'b list ->
    f:(Relative_path.t -> 'c -> Pos.t list -> Pos.t list) ->
    'd) ->
  files_info:'a ->
  'd

val provide_hh_fixmes : Relative_path.t -> fixme_map -> unit

val provide_decl_hh_fixmes : Relative_path.t -> fixme_map -> unit

val provide_disallowed_fixmes : Relative_path.t -> fixme_map -> unit

val remove_batch : Relative_path.Set.t -> unit

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit

val local_changes_commit_batch : Relative_path.Set.t -> unit

val local_changes_revert_batch : Relative_path.Set.t -> unit
