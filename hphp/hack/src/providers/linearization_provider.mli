(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type key = string * Decl_defs.linearization_kind

val get : Provider_context.t -> key -> Decl_defs.linearization option

val add : Provider_context.t -> key -> Decl_defs.linearization -> unit

val complete : Provider_context.t -> key -> Decl_defs.mro_element list -> unit

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit

val remove_batch : Provider_context.t -> SSet.t -> unit
