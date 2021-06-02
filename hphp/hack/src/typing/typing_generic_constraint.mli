(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val check_constraint :
  Typing_env_types.env ->
  Ast_defs.constraint_kind ->
  Typing_defs.locl_ty ->
  cstr_ty:Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  Typing_env_types.env

val check_tparams_constraint :
  Typing_env_types.env ->
  use_pos:Pos.t ->
  Ast_defs.constraint_kind ->
  cstr_ty:Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env

val check_where_constraint :
  in_class:bool ->
  Typing_env_types.env ->
  use_pos:Pos.t ->
  definition_pos:Pos_or_decl.t ->
  Ast_defs.constraint_kind ->
  cstr_ty:Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env
