(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 * *)

open Typing_defs
open Typing_env_types

val check_class_access :
  is_method:bool ->
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  env ->
  ce_visibility * bool ->
  Nast.class_id_ ->
  Decl_provider.class_decl ->
  Typing_error.t option

val check_internal_access :
  in_signature:bool ->
  Typing_env_types.env ->
  string option ->
  Pos.t ->
  Pos_or_decl.t ->
  Typing_error.t option

val check_obj_access :
  is_method:bool ->
  is_receiver_interface:bool ->
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  env ->
  ce_visibility ->
  Typing_error.t option

(** Checks whether access to a symbol is permitted according to package visibility rules. *)
val check_package_access :
  should_check_package_boundary:Typing_packages.check_reason ->
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  env ->
  Aast_defs.package_membership option ->
  string ->
  Typing_error.t option

val check_top_level_access :
  should_check_package_boundary:Typing_packages.check_reason ->
  in_signature:bool ->
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  env ->
  bool ->
  string option ->
  Aast_defs.package_membership option ->
  string ->
  Typing_error.t list

val check_meth_caller_access :
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  ce_visibility ->
  Typing_error.t option

val check_deprecated :
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  env ->
  string option ->
  Typing_error.t option

(* Expression Trees can only access publicly accessible properties *)
val check_expression_tree_vis :
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  env ->
  ce_visibility ->
  Typing_error.t option

val is_visible :
  is_method:bool ->
  env ->
  ce_visibility * bool ->
  Nast.class_id_ option ->
  Decl_provider.class_decl ->
  bool

(** Checks whether a function call satisfies the callee's package requirement annotation
  * (RequirePackage or SoftRequirePackage).

  * This function validates that the caller's per-continuation package environment
  * includes the required package. For soft requirements, it also considers whether
  * the caller has a matching [__SoftRequirePackage] annotation to allow soft-to-soft
  * calls during migration.
  *)
val check_cross_package :
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  env ->
  package_requirement option ->
  Typing_error.t option
