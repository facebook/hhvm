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

(** If a shallow declaration for the class with the given name is present in the
    cache, return it. Otherwise, convert the given class AST to a shallow class
    declaration, store it in the cache, and return it.

    Raises [Failure] if [shallow_class_decl] is not enabled. *)
val decl :
  ctx:Provider_context.t -> use_cache:bool -> Nast.class_ -> shallow_class

val get_batch : SSet.t -> shallow_class option SMap.t

val get_old_batch : SSet.t -> shallow_class option SMap.t

val oldify_batch : SSet.t -> unit

val remove_old_batch : SSet.t -> unit

val remove_batch : SSet.t -> unit

val push_local_changes : unit -> unit

val pop_local_changes : unit -> unit

val invalidate_class : Provider_context.t -> string -> unit
