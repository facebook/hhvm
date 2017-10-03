(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type elaborate_kind =
  | ElaborateFun
  | ElaborateClass
  | ElaborateConst

val elaborate_id : Namespace_env.env ->
                   elaborate_kind ->
                   Ast.id ->
                   Ast.id
(* This function processes only top-level declarations and does not dive
  into inline classes/functions - those are disallowed in Hack and doing it will
  incur a perf hit that everybody will have to pay. For codegen purposed
  namespaces are propagated to inline declarations
  during closure conversion process *)
val elaborate_toplevel_defs : ParserOptions.t -> Ast.program -> Ast.program

val elaborate_def:
  Namespace_env.env ->
  Ast.def ->
  Namespace_env.env * Ast.def list

val renamespace_if_aliased : ?reverse:bool ->
                             (string * string) list ->
                             string ->
                             string
