(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val type_fun :
  Provider_context.t -> full_ast:Nast.fun_def -> Tast.def list option

val type_class : Provider_context.t -> full_ast:Nast.class_ -> Tast.def option

val check_typedef :
  Provider_context.t -> full_ast:Nast.typedef -> Tast.def option

val check_const : Provider_context.t -> full_ast:Nast.gconst -> Tast.def option

val check_module :
  Provider_context.t -> full_ast:Nast.module_def -> Tast.def option

(** [type_file ctx fn ~full_ast] works as follows:
1. uses [fn] for the error context
2. iterates over the defs in [full_ast]
3. For each one runs them through [Naming], does [Nast_check],
   typechecks and obtains tast with [Typing_toplevel], does [Tast_check]

It is unfortunate that this routine exists alongside [Typing_top_level.nast_to_tast]
which does almost exactly the same thing, except it doesn't do [Naming], and
there are minor differences in treatment of some toplevel nodes. *)
val calc_errors_and_tast :
  Provider_context.t ->
  ?drop_fixmed:bool ->
  Relative_path.t ->
  full_ast:Nast.program ->
  Errors.t * Tast.by_names

val calc_errors_and_tast_for :
  Provider_context.t ->
  ?drop_fixmed:bool ->
  Relative_path.t ->
  (Provider_context.t -> full_ast:'def -> 'res option) ->
  full_ast:'def ->
  FileInfo.id ->
  Errors.t * 'res SMap.t
