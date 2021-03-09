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
  ctx: Provider_context.t;
}

val tcopt : env -> TypecheckerOptions.t

val add_wclass : env -> string -> unit

val add_extends_dependency : env -> string -> unit

type class_cache = Decl_store.class_entries SMap.t

(** Add a dependency to the class then get class decl.
    First look up in cache and if not found lookup in heap. *)
val get_class_add_dep :
  env -> ?cache:class_cache -> string -> Decl_defs.decl_class_type option

val get_construct :
  env ->
  Decl_defs.decl_class_type ->
  Decl_defs.element option * Typing_defs.consistent_kind

val add_constructor_dependency : env -> string -> unit

val make_decl_pos : env -> Pos.t -> Pos_or_decl.t

val make_decl_posed : env -> Pos.t * 'a -> Pos_or_decl.t * 'a
