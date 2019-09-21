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
  TypecheckerOptions.t ->
  Nast.fun_ ->
  (Tast.fun_ * Typing_env_types.global_tvenv) option

val class_def :
  TypecheckerOptions.t ->
  Nast.class_ ->
  (Tast.class_ * Typing_env_types.global_tvenv list) option

val record_def_def : TypecheckerOptions.t -> Nast.record_def -> Tast.record_def

val typedef_def : TypecheckerOptions.t -> Nast.typedef -> Tast.typedef

val gconst_def : TypecheckerOptions.t -> Nast.gconst -> Tast.gconst

val nast_to_tast : TypecheckerOptions.t -> Nast.program -> Tast.program
