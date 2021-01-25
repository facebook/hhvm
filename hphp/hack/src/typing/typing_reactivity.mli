(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type method_call_info

val make_call_info :
  receiver_is_self:bool ->
  is_static:bool ->
  Typing_defs.locl_ty ->
  string ->
  method_call_info

val condition_type_from_reactivity :
  Typing_defs.reactivity -> Typing_defs.decl_ty option

val strip_conditional_reactivity :
  Typing_defs.reactivity -> Typing_defs.reactivity

val condition_type_matches :
  is_self:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  bool

val check_call :
  Typing_env_types.env ->
  method_call_info option ->
  Pos.t ->
  Typing_reason.t ->
  Typing_defs.locl_ty Typing_defs.fun_type ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env

val disallow_atmost_rx_as_rxfunc_on_non_functions :
  Typing_env_types.env -> Nast.fun_param -> Typing_defs.locl_ty -> unit

val try_substitute_type_with_condition :
  Typing_env_types.env ->
  Typing_defs.decl_ty ->
  Typing_defs.locl_ty ->
  (Typing_env_types.env * 'a Typing_defs.ty) option

val strip_condition_type_in_return :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty

val get_adjusted_return_type :
  Typing_env_types.env ->
  method_call_info option ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val check_awaitable_immediately_awaited :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Pos.t ->
  allow_awaitable:bool ->
  Typing_env_types.env

val check_assignment :
  Typing_env_types.env -> (Pos.t * 'a) * Tast.expr_ -> Typing_env_types.env

val check_unset_target :
  Typing_env_types.env ->
  ((Pos.t * 'a) * Tast.expr_) list ->
  Typing_env_types.env
