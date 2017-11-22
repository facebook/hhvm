(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Emit wrapper function for functions with inout arguments *)
val emit_wrapper_function :
  decl_vars: string list ->
  is_top: bool ->
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
  decl_vars: string list ->
  (* Original identifer for function, used for the wrapper *)
  original_id: Hhbc_id.Method.t ->
  (* Renamed identifier, used for the wrapped function *)
  renamed_id: Hhbc_id.Method.t ->
  (* Function definition in AST *)
  Ast.class_ ->
  Ast.method_ ->
  Hhas_method.t
