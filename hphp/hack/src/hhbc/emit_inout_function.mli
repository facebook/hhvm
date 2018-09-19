(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* Emit wrapper function for functions with inout arguments *)
val emit_wrapper_function :
  decl_vars: string list ->
  hoisted: Closure_convert.hoist_kind ->
  (* Whether this is wrapper for inout or reference *)
  wrapper_type: Emit_inout_helpers.wrapper_type ->
  (* Original identifer for function, used for the wrapper *)
  original_id: Hhbc_id.Function.t ->
  (* Renamed identifier, used for the wrapped function *)
  renamed_id: Hhbc_id.Function.t ->
  (* Function definition in AST *)
  Ast.fun_ ->
  Hhas_function.t

(* Emit wrapper function for functions with inout arguments *)
val emit_wrapper_method :
  is_closure: bool ->
  decl_vars: string list ->
  (* Original identifer for function, used for the wrapper *)
  original_id: Hhbc_id.Method.t ->
  (* Renamed identifier, used for the wrapped function *)
  renamed_id: Hhbc_id.Method.t ->
  (* Function definition in AST *)
  Ast.class_ ->
  Ast.method_ ->
  Hhas_method.t
