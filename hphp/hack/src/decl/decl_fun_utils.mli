(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val has_return_disposable_attribute : Nast.user_attribute list -> bool

val has_memoize_attribute : Nast.user_attribute list -> bool

val hint_to_type_opt :
  Decl_env.env -> Nast.xhp_attr_hint option -> Typing_defs.decl_ty option

val make_param_ty :
  Decl_env.env ->
  Nast.fun_param ->
  Typing_defs.decl_ty Typing_defs_core.fun_param

val ret_from_fun_kind :
  ?is_constructor:bool ->
  Decl_env.env ->
  Pos.t ->
  Ast_defs.fun_kind ->
  Nast.xhp_attr_hint option ->
  Typing_defs.decl_ty

val type_param :
  Decl_env.env -> Nast.tparam -> Typing_defs.decl_ty Typing_defs_core.tparam

val where_constraint :
  Decl_env.env ->
  Nast.xhp_attr_hint * 'a * Nast.xhp_attr_hint ->
  Typing_defs.decl_ty * 'a * Typing_defs.decl_ty

val check_params : Nast.fun_param list -> Typing_error.t option

val make_params :
  Decl_env.env ->
  Nast.fun_param list ->
  Typing_defs.decl_ty Typing_defs_core.fun_param list
