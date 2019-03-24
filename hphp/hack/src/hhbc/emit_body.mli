(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

val make_body:
  Instruction_sequence.t ->
  string list -> (* Actually local_id list *)
  bool ->
  bool ->
  Hhas_param.t list ->
  Hhas_type_info.t option ->
  string option ->
  Emit_env.t option ->
  Hhas_body.t

val emit_body:
  pos: Pos.t ->
  scope: Ast_scope.Scope.t ->
  (* True if this is the body of a closure method *)
  is_closure_body: bool ->
  (* True if this is the body of a <<__Memoize>> method *)
  is_memoize: bool ->
  is_native: bool ->
  is_async: bool ->
  is_rx_body: bool ->
  debugger_modify_program: bool ->
  deprecation_info: (Typed_value.t list) option ->
  skipawaitable:bool ->
  default_dropthrough: Instruction_sequence.t option ->
  return_value: Instruction_sequence.t ->
  namespace: Namespace_env.env ->
  doc_comment: string option ->
  Ast.tparam list ->
  Ast.fun_param list ->
  Ast.hint option ->
  Ast.program ->
  Hhas_body.t * bool(* is_generator *) * bool (* is_pair_generator *)

val tparams_to_strings : Ast.tparam list -> string list

val emit_method_prolog :
  env: Emit_env.t ->
  pos: Pos.t ->
  params: Hhas_param.t list ->
  ast_params: Ast.fun_param list ->
  should_emit_init_this:bool ->
  Instruction_sequence.t

val emit_return_type_info :
  scope: Ast_scope.Scope.t ->
  skipawaitable: bool ->
  namespace: Namespace_env.env ->
  Ast.hint option ->
  Hhas_type_info.t

val emit_deprecation_warning :
  Ast_scope.Scope.t -> (* scope *)
  (Typed_value.t list) option -> (* deprecation_info *)
  Instruction_sequence.t
