(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val debug_print_last_pos : 'a -> unit

val typedef_def : Provider_context.t -> Nast.typedef -> Tast.typedef

val expr :
  ?expected:Typing_helpers.ExpectedTy.t ->
  Typing_env_types.env ->
  Nast.expr ->
  Typing_env_types.env * Tast.expr * Typing_defs.locl_ty

val expr_with_pure_coeffects :
  ?expected:Typing_helpers.ExpectedTy.t ->
  Typing_env_types.env ->
  Nast.expr ->
  Typing_env_types.env * Tast.expr * Typing_defs.locl_ty

val user_attribute :
  Typing_env_types.env ->
  Nast.user_attribute ->
  Typing_env_types.env * Tast.user_attribute

val stmt : Typing_env_types.env -> Nast.stmt -> Typing_env_types.env * Tast.stmt

val bind_param :
  Typing_env_types.env ->
  ?immutable:bool ->
  Typing_defs.locl_ty * Nast.fun_param ->
  Typing_env_types.env * Tast.fun_param

val fun_ :
  ?abstract:bool ->
  ?disable:bool ->
  Typing_env_types.env ->
  Typing_env_return_info.t ->
  Pos.t ->
  Nast.func_body ->
  Ast_defs.fun_kind ->
  Typing_env_types.env * Tast.stmt list

val attributes_check_def :
  Typing_env_types.env ->
  string ->
  Nast.user_attribute list ->
  Typing_env_types.env

val file_attributes :
  Typing_env_types.env ->
  Nast.file_attribute list ->
  Typing_env_types.env * Tast.file_attribute list

val type_param :
  Typing_env_types.env -> Nast.tparam -> Typing_env_types.env * Tast.tparam

val check_shape_keys_validity :
  Typing_env_types.env ->
  Pos.t ->
  Ast_defs.shape_field_name list ->
  Typing_env_types.env

val type_capability :
  Typing_env_types.env ->
  Aast.contexts option ->
  Aast.contexts option ->
  Pos.t ->
  Typing_env_types.env * Typing_defs.locl_ty * Typing_defs.locl_ty

val call :
  expected:Typing_helpers.ExpectedTy.t option ->
  ?nullsafe:Pos.t option ->
  ?in_await:Typing_reason.t ->
  Pos.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Nast.expr list ->
  Nast.expr option ->
  Typing_env_types.env
  * (Tast.expr list * Tast.expr option * Typing_defs.locl_ty * bool)

val with_special_coeffects :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  (Typing_env_types.env -> Typing_env_types.env * 'a) ->
  Typing_env_types.env * 'a

val triple_to_pair :
  Typing_env_types.env * 'a * 'b -> Typing_env_types.env * ('a * 'b)
