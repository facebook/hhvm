(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type file_type =
  | Disk of string
  | Ide of string

exception File_provider_stale

val get : Relative_path.t -> file_type option

val get_unsafe : Relative_path.t -> file_type

val get_contents : Relative_path.t -> string option

val get_ide_contents_unsafe : Relative_path.t -> string

val provide_file : Relative_path.t -> file_type -> unit

val provide_file_hint : Relative_path.t -> file_type -> unit

val remove_batch : Relative_path.Set.t -> unit

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit

val local_changes_commit_batch : Relative_path.Set.t -> unit

val local_changes_revert_batch : Relative_path.Set.t -> unit
