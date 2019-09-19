(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Emit wrapper function for functions with inout arguments *)
val emit_wrapper_function :
  decl_vars:string list ->
  hoisted:Closure_convert.hoist_kind ->
  wrapper_type:
    (* Whether this is wrapper for inout or reference *)
    Emit_inout_helpers.wrapper_type ->
  original_id:
    (* Original identifer for function, used for the wrapper *)
    Hhbc_id.Function.t ->
  renamed_id:
    (* Renamed identifier, used for the wrapped function *)
    Hhbc_id.Function.t ->
  (* Function definition in AST *)
  Tast.fun_ ->
  Hhas_function.t

(* Emit wrapper function for functions with inout arguments *)
val emit_wrapper_method :
  is_closure:bool ->
  decl_vars:string list ->
  original_id:
    (* Original identifer for function, used for the wrapper *)
    Hhbc_id.Method.t ->
  renamed_id:
    (* Renamed identifier, used for the wrapped function *)
    Hhbc_id.Method.t ->
  (* Function definition in AST *)
  Tast.class_ ->
  Tast.method_ ->
  Hhas_method.t
