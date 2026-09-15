(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type get_class_add_dep =
  Decl_env.env -> string -> Decl_defs.decl_class_type option

val parent_init_prop : string

val init_not_required_props : Nast.class_ -> S_set.t -> S_set.t

val parent :
  get_class_add_dep:get_class_add_dep ->
  Decl_env.env ->
  Nast.class_ ->
  S_set.t ->
  S_set.t

val own_props : Nast.class_ -> S_set.t -> S_set.t

val parent_props :
  get_class_add_dep:get_class_add_dep ->
  Decl_env.env ->
  Nast.class_ ->
  S_set.t ->
  S_set.t

val trait_props :
  get_class_add_dep:get_class_add_dep ->
  Decl_env.env ->
  Nast.class_ ->
  S_set.t ->
  S_set.t

val private_deferred_init_props : has_own_cstr:bool -> Nast.class_ -> S_set.t

val nonprivate_deferred_init_props :
  has_own_cstr:bool ->
  get_class_add_dep:get_class_add_dep ->
  Decl_env.env ->
  Shallow_decl_defs.shallow_class ->
  S_set.t

val parent_initialized_members :
  get_class_add_dep:get_class_add_dep -> Decl_env.env -> Nast.class_ -> S_set.t
