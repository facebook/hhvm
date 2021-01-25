(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Add constraint or check that ty_sub is subtype of ty_super in envs *)
val sub_type_i :
  Pos.t ->
  Typing_reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  Errors.typing_error_callback ->
  Typing_env_types.env

val sub_type :
  Pos.t ->
  Typing_reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.typing_error_callback ->
  Typing_env_types.env

val sub_type_decl :
  Pos.t ->
  Typing_reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty ->
  Typing_env_types.env

val sub_type_decl_on_error :
  Pos.t ->
  Typing_reason.ureason ->
  Typing_env_types.env ->
  Errors.typing_error_callback ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty ->
  Typing_env_types.env

val unify_decl :
  Pos.t ->
  Typing_reason.ureason ->
  Typing_env_types.env ->
  Errors.typing_error_callback ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty ->
  Typing_env_types.env
