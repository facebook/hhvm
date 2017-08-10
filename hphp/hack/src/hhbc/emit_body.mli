(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

val make_body:
  Instruction_sequence.t ->
  string list -> (* Actually local_id list *)
  bool ->
  Hhas_param.t list ->
  Hhas_type_info.t option ->
  string list ->
  string option ->
  Hhas_body.t

val emit_body:
  pos: Pos.t ->
  scope: Ast_scope.Scope.t ->
  (* True if this is the body of a closure method *)
  is_closure_body: bool ->
  (* True if this is the body of a <<__Memoize>> method *)
  is_memoize: bool ->
  is_async: bool ->
  skipawaitable:bool ->
  (* True if the return type is a ref *)
  is_return_by_ref: bool ->
  default_dropthrough: Instruction_sequence.t option ->
  return_value: Instruction_sequence.t ->
  namespace: Namespace_env.env ->
  doc_comment: string option ->
  Ast.fun_param list ->
  Ast.hint option ->
  Ast.program ->
  Hhas_body.t * bool(* is_generator *) * bool (* is_pair_generator *)

val tparams_to_strings : Ast.tparam list -> string list

val emit_method_prolog :
  pos: Pos.t ->
  params: Hhas_param.t list ->
  needs_local_this:bool ->
  Instruction_sequence.t

val emit_return_type_info :
  scope: Ast_scope.Scope.t ->
  skipawaitable: bool ->
  namespace: Namespace_env.env ->
  Ast.hint option ->
  Hhas_type_info.t
