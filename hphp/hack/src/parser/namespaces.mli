(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type elaborate_kind =
  | ElaborateFun
  | ElaborateClass
  | ElaborateConst

val elaborate_id : ?autoimport:bool -> Namespace_env.env ->
                   elaborate_kind ->
                   Ast.id ->
                   Ast.id

val elaborate_id_impl : autoimport:bool -> Namespace_env.env ->
                   elaborate_kind ->
                   Ast.id ->
                   bool * Ast.id
(* This function processes only top-level declarations and does not dive
  into inline classes/functions - those are disallowed in Hack and doing it will
  incur a perf hit that everybody will have to pay. For codegen purposed
  namespaces are propagated to inline declarations
  during closure conversion process *)
val elaborate_toplevel_defs : autoimport:bool -> ParserOptions.t -> Ast.program -> Ast.program
val elaborate_map_toplevel_defs :
  autoimport:bool ->
  ParserOptions.t ->
  Ast.program ->
  (Namespace_env.env -> Ast.def -> Ast.def) ->
  Ast.program

val elaborate_def:
  Namespace_env.env ->
  Ast.def ->
  Namespace_env.env * Ast.def list
