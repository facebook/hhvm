(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shallow_decl_defs

(** Return the shallow declaration of the class with the given name if it is
present in the cache. Otherwise, compute it, store it in the cache, and
return it.

Raises [Failure] if [shallow_class_decl] is not enabled. *)
val get : Provider_context.t -> string -> shallow_class option

(** Return the member filter of the class with the given name if it is
present in the cache. Otherwise [None] is returned. This filter is a cheaper
way to test if a a shallow class might contain a particular member
(property, method, constructor, constant, or type constant). *)
val get_member_filter : Provider_context.t -> string -> BloomFilter.t option

(** DEPRECATED: Use the direct decl parser or [get] instead. *)
val decl_DEPRECATED : Provider_context.t -> Nast.class_ -> shallow_class

val get_batch : Provider_context.t -> SSet.t -> shallow_class option SMap.t

val get_old_batch :
  Provider_context.t ->
  SSet.t ->
  fetch_old_decls:(string list -> Shallow_decl_defs.shallow_class option SMap.t) ->
  shallow_class option SMap.t

val oldify_batch : Provider_context.t -> SSet.t -> unit

val remove_old_batch : Provider_context.t -> SSet.t -> unit

val remove_batch : Provider_context.t -> SSet.t -> unit

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit
