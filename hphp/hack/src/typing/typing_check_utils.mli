(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [type_file ctx fn ~full_ast] works as follows:
1. uses [fn] for the error context
2. iterates over the defs in [full_ast]
3. For each one runs them through [Naming], does [Nast_check],
   typechecks and obtains tast with [Typing_toplevel], does [Tast_check]

It is unfortunate that this routine exists alongside [Typing_top_level.nast_to_tast]
which does almost exactly the same thing, except it doesn't do [Naming], and
there are minor differences in treatment of some toplevel nodes. *)
val type_file :
  Provider_context.t ->
  Relative_path.t ->
  full_ast:Nast.program ->
  Tast.def list * Errors.t
