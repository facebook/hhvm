(*
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
  | ElaborateRecord
  | ElaborateConst

val elaborate_id :
  Namespace_env.env -> elaborate_kind -> Ast_defs.id -> Ast_defs.id

val elaborate_id_impl :
  Namespace_env.env -> elaborate_kind -> string -> bool * string

(* This function processes only top-level declarations and does not dive
  into inline classes/functions - those are disallowed in Hack and doing it will
  incur a perf hit that everybody will have to pay. For codegen purposed
  namespaces are propagated to inline declarations
  during closure conversion process *)
val elaborate_toplevel_defs : ParserOptions.t -> Ast.program -> Ast.program
