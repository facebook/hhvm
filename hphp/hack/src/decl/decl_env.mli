(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  mode: FileInfo.mode;
  droot: Typing_deps.Dep.dependent Typing_deps.Dep.variant option;
  droot_member: Typing_pessimisation_deps.dependent_member option;
      (** The child/member of [droot] currently under consideration.
        * Used for fine-grained dependency tracking. *)
  ctx: Provider_context.t;
}

val tcopt : env -> TypecheckerOptions.t

type class_cache = Decl_store.class_entries SMap.t

(** Auxiliary constant fallback function that returns [None]. *)
val no_fallback : env -> string -> Decl_defs.decl_class_type option

(** Lookup the class from the cache.

    If [shmem_fallback] and the class is not in the cache, look for the
    class in the classes heap.

    If the class is not in the cache, and [shmem_fallback] is false or the
    class is not in the classes heap, call and return [fallback].

    If a class is returned, add a dependency to the class. *)
val get_class_and_add_dep :
  cache:class_cache ->
  shmem_fallback:bool ->
  fallback:(env -> string -> Decl_defs.decl_class_type option) ->
  env ->
  string ->
  Decl_defs.decl_class_type option

val make_decl_pos : env -> Pos.t -> Pos_or_decl.t

val make_decl_posed : env -> Pos.t * 'a -> Pos_or_decl.t * 'a
