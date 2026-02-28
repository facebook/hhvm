(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception File_provider_stale

val get_contents : ?force_read_disk:bool -> Relative_path.t -> string option

val provide_file_for_tests : Relative_path.t -> string -> unit

val remove_batch : Relative_path.Set.t -> unit

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit
