(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val trait_reuse_with_final_method_error :
  Typing_env_types.env ->
  Typing_defs.class_elt ->
  class_name:string ->
  first_using_parent_or_trait:Decl_provider.class_decl ->
  second_using_trait:Pos.t * Decl_provider.class_decl ->
  Typing_error.t

val method_import_via_diamond_error :
  Typing_env_types.env ->
  Pos.t * string ->
  string * Typing_defs.class_elt ->
  first_using_trait:Decl_provider.class_decl ->
  second_using_trait:Decl_provider.class_decl ->
  Typing_error.t

val generic_property_import_via_diamond_error :
  Typing_env_types.env ->
  Pos.t * string ->
  string * Typing_defs.class_elt ->
  first_using_trait:Decl_provider.class_decl ->
  second_using_trait:Decl_provider.class_decl ->
  unit
