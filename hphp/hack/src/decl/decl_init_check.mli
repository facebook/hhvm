(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type get_class_add_dep =
  Decl_env.env -> string -> Decl_defs.decl_class_type option

val init_not_required_props : Nast.class_ -> SSet.t -> SSet.t

val parent :
  get_class_add_dep:get_class_add_dep ->
  Decl_env.env ->
  Nast.class_ ->
  SSet.t ->
  SSet.t

val own_props : Nast.class_ -> SSet.t -> SSet.t

val parent_props :
  get_class_add_dep:get_class_add_dep ->
  Decl_env.env ->
  Nast.class_ ->
  SSet.t ->
  SSet.t

val trait_props :
  get_class_add_dep:get_class_add_dep ->
  Decl_env.env ->
  Nast.class_ ->
  SSet.t ->
  SSet.t

val private_deferred_init_props : has_own_cstr:bool -> Nast.class_ -> SSet.t

val nonprivate_deferred_init_props :
  has_own_cstr:bool ->
  get_class_add_dep:get_class_add_dep ->
  Decl_env.env ->
  Shallow_decl_defs.shallow_class ->
  SSet.t

val parent_initialized_members :
  get_class_add_dep:get_class_add_dep -> Decl_env.env -> Nast.class_ -> SSet.t
