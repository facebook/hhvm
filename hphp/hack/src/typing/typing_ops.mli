(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Add constraint or check that ty_sub is subtype of ty_super in envs *)
val sub_type_i :
  ?is_coeffect:bool ->
  Pos.t ->
  Typing_reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  Typing_error.Callback.t ->
  Typing_env_types.env * Typing_error.t option

val sub_type :
  Pos.t ->
  Typing_reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_error.Callback.t ->
  Typing_env_types.env * Typing_error.t option

(** Assert a type is a sub-type of another.
    If assertion fails, add error prefix to sub-typing error. *)
val sub_type_w_err_prefix :
  ?is_coeffect:bool ->
  Pos.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_error.Primary.t ->
  Typing_env_types.env * Typing_error.t option

val sub_type_decl :
  ?is_coeffect:bool ->
  on_error:Typing_error.Reasons_callback.t ->
  Pos_or_decl.t ->
  Typing_reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty ->
  Typing_env_types.env * Typing_error.t option

val unify_decl :
  Pos_or_decl.t ->
  Typing_reason.ureason ->
  Typing_env_types.env ->
  Typing_error.Reasons_callback.t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty ->
  Typing_env_types.env * Typing_error.t option
