(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val class_def : Provider_context.t -> Nast.class_ -> Tast.class_ option

val fun_def : Provider_context.t -> Nast.fun_def -> Tast.fun_def list option

val gconst_def : Provider_context.t -> Nast.gconst -> Tast.gconst

(** Run typing on the given named AST (NAST) to produced a typed AST (TAST).

Set [do_tast_checks] to [false] to skip running TAST checks on the resulting
TAST. This means that the associated list of errors may be incomplete. This is
useful for performance in cases where we want the TAST, but don't need a correct
list of errors.

It is unfortunate that this routine exists alongside [Typing_check_job.calc_errors_and_tast]
which does almost exactly the same thing, except it also does [Naming], and
there are minor differences in treatment of some toplevel nodes. *)
val nast_to_tast :
  do_tast_checks:bool -> Provider_context.t -> Nast.program -> Tast.program

val module_def : Provider_context.t -> Nast.module_def -> Tast.module_def
