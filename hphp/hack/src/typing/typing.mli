(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val with_expr_hook :
  (Nast.expr -> Typing_defs.locl_ty -> unit) -> (unit -> 'a) -> 'a

val debug_print_last_pos : 'a -> unit

val fun_def :
  Provider_context.t ->
  Nast.fun_ ->
  (Tast.fun_ * Typing_inference_env.t_global_with_pos) option

val class_def :
  Provider_context.t ->
  Nast.class_ ->
  (Tast.class_ * Typing_inference_env.t_global_with_pos list) option

val typedef_def : Provider_context.t -> Nast.typedef -> Tast.typedef

val expr :
  ?expected:Typing_helpers.ExpectedTy.t ->
  Typing_env_types.env ->
  Nast.expr ->
  Typing_env_types.env * Tast.expr * Typing_defs.locl_ty

val user_attribute :
  Typing_env_types.env ->
  Nast.user_attribute ->
  Typing_env_types.env * Tast.user_attribute

val stmt : Typing_env_types.env -> Nast.stmt -> Typing_env_types.env * Tast.stmt
