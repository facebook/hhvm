(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

val emit_body:
  scope: Ast_scope.Scope.t ->
  is_closure_body: bool ->
  skipawaitable: bool ->
  default_dropthrough: Instruction_sequence.t option ->
  return_value: Instruction_sequence.t ->
  namespace: Namespace_env.env ->
  Ast.fun_param list ->
  Ast.hint option ->
  Ast.program ->
  Hhas_body.t * bool(* is_generator *) * bool (* is_pair_generator *)

val tparams_to_strings : Ast.tparam list -> string list

val emit_method_prolog :
  params:Hhas_param.t list ->
  needs_local_this:bool ->
  Instruction_sequence.t
