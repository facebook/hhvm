(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val class_def :
  Provider_context.t ->
  Nast.class_ ->
  (Tast.class_ * Typing_inference_env.t_global_with_pos list) option

val fun_def :
  Provider_context.t ->
  Nast.fun_def ->
  (Tast.fun_def * Typing_inference_env.t_global_with_pos) option

val gconst_def : Provider_context.t -> Nast.gconst -> Tast.gconst

val record_def_def : Provider_context.t -> Nast.record_def -> Tast.record_def

val nast_to_tast_gienv :
  do_tast_checks:bool ->
  Provider_context.t ->
  Nast.program ->
  Tast.program * Typing_inference_env.t_global_with_pos list

(** Run typing on the given named AST (NAST) to produced a typed AST (TAST).

Set [do_tast_checks] to [false] to skip running TAST checks on the resulting
TAST. This means that the associated list of errors may be incomplete. This is
useful for performance in cases where we want the TAST, but don't need a correct
list of errors. *)
val nast_to_tast :
  do_tast_checks:bool -> Provider_context.t -> Nast.program -> Tast.program
