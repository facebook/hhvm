(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** If the input is the winning definition of its symbol,
returns [Some t] and as a side effect populates errors encountered
during typechecking. If not the winning definition, returns None
and emits a "duplicate name" error. (We're not able to check
a class that isn't the winning declaration, because we use both
the AST and the folded-decl in the work of checking, and the only
folded-decl available is that of the winner, so we can't proceed
if this definition isn't the winner.

If any method within the class took longer than `--config timeout=<secs>`
to typecheck (default infinite) then it's omitted from the resulting Tast. *)
val class_def :
  Provider_context.t -> Nast.class_ -> Tast.class_ Tast_with_dynamic.t option

val typedef_def : Provider_context.t -> Nast.typedef -> Tast.typedef

(** If it takes longer than `--config timeout=<secs>` then produces None.
Otherwise, it returns either 1 or 2 tasts, depending on whether sound-dynamic is enabled. *)
val fun_def :
  Provider_context.t -> Nast.fun_def -> Tast.fun_def Tast_with_dynamic.t option

val gconst_def : Provider_context.t -> Nast.gconst -> Tast.gconst

val module_def : Provider_context.t -> Nast.module_def -> Tast.module_def

val set_module_def : Provider_context.t -> Nast.sid -> Tast.sid

(** Run typing on the given named AST (NAST) to produced a typed AST (TAST).

Set [do_tast_checks] to [false] to skip running TAST checks on the resulting
TAST. This means that the associated list of errors may be incomplete. This is
useful for performance in cases where we want the TAST, but don't need a correct
list of errors.

It is unfortunate that this routine exists alongside [Typing_check_job.calc_errors_and_tast]
which does almost exactly the same thing, except it also does [Naming], and
there are minor differences in treatment of some toplevel nodes. *)
val nast_to_tast :
  do_tast_checks:bool ->
  Provider_context.t ->
  Nast.program ->
  Tast.program Tast_with_dynamic.t
