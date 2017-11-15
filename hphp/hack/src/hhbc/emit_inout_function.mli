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
  (* Original identifer for function, used for the wrapper *)
  original_id: Hhbc_id.Function.t ->
  (* Renamed identifier, used for the wrapped function *)
  renamed_id: Hhbc_id.Function.t ->
  (* Whether to emit verify return instruction  *)
  verify_ret: bool ->
  (* Function definition in AST *)
  Ast.fun_ ->
  Hhas_function.t
