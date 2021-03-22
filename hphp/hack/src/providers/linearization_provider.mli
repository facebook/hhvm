(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get : Provider_context.t -> string -> Decl_defs.lin option

val add : Provider_context.t -> string -> Decl_defs.lin -> unit

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit

val remove_batch : Provider_context.t -> SSet.t -> unit
