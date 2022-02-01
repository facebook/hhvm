(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type class_cache = Decl_store.class_entries SMap.t

val init_not_required_props : Nast.class_ -> SSet.t -> SSet.t

val parent :
  class_cache:Decl_env.class_cache option ->
  Decl_env.env ->
  Nast.class_ ->
  SSet.t ->
  SSet.t

val own_props : Nast.class_ -> SSet.t -> SSet.t

val parent_props :
  class_cache:class_cache option ->
  Decl_env.env ->
  Nast.class_ ->
  SSet.t ->
  SSet.t

val trait_props : Decl_env.env -> Nast.class_ -> SSet.t -> SSet.t

val private_deferred_init_props : has_own_cstr:bool -> Nast.class_ -> SSet.t

val nonprivate_deferred_init_props :
  has_own_cstr:bool ->
  class_cache:class_cache option ->
  Decl_env.env ->
  Shallow_decl_defs.shallow_class ->
  SSet.t

val parent_initialized_members :
  class_cache:class_cache option -> Decl_env.env -> Nast.class_ -> SSet.t
