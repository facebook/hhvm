(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val debug_print_last_pos : 'a -> unit

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

val stmt : Typing_env_types.env -> Nast.stmt -> Typing_env_types.env * Tast.stmt

val bind_params :
  Typing_env_types.env ->
  ?can_read_globals:bool ->
  no_auto_likes:bool ->
  Aast_defs.contexts option ->
  Typing_defs.locl_ty option list ->
  Nast.fun_param list ->
  Typing_env_types.env * Tast.fun_param list

val fun_ :
  ?abstract:bool ->
  ?native:bool ->
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
  Typing_env_types.env * Tast.user_attribute list

val file_attributes :
  Typing_env_types.env ->
  Nast.file_attribute list ->
  Typing_env_types.env * Tast.file_attribute list

val type_param :
  Typing_env_types.env -> Nast.tparam -> Typing_env_types.env * Tast.tparam

type dyn_func_kind

val call :
  expected:Typing_helpers.ExpectedTy.t option ->
  ?nullsafe:Pos.t option ->
  ?in_await:Typing_reason.t ->
  ?dynamic_func:dyn_func_kind ->
  expr_pos:Pos.t ->
  recv_pos:Pos.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  (Ast_defs.param_kind * Nast.expr) list ->
  Nast.expr option ->
  Typing_env_types.env
  * ((Ast_defs.param_kind * Tast.expr) list
    * Tast.expr option
    * Typing_defs.locl_ty
    * bool)

val with_special_coeffects :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  (Typing_env_types.env -> Typing_env_types.env * 'a) ->
  Typing_env_types.env * 'a

val triple_to_pair :
  Typing_env_types.env * 'a * 'b -> Typing_env_types.env * ('a * 'b)

val check_function_dynamically_callable :
  this_class:Decl_provider.Class.t option ->
  Typing_env_types.env ->
  Aast_defs.sid option ->
  Nast.fun_ ->
  Typing_defs.decl_ty option list ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Tast.fun_param list * Tast.stmt list * Tast.ty
