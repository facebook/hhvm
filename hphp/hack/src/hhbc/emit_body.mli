(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val make_body :
  Instruction_sequence.t ->
  string list ->
  (* Actually local_id list *)
  bool ->
  bool ->
  (string * Hhas_type_info.t list) list ->
  string list ->
  Hhas_param.t list ->
  Hhas_type_info.t option ->
  string option ->
  Emit_env.t option ->
  Hhas_body.t

val emit_body :
  pos:Pos.t ->
  scope:Ast_scope.Scope.t ->
  is_closure_body:(* True if this is the body of a closure method *)
                  bool ->
  is_memoize:(* True if this is the body of a <<__Memoize>> method *)
             bool ->
  is_native:bool ->
  is_async:bool ->
  is_rx_body:bool ->
  debugger_modify_program:bool ->
  deprecation_info:Typed_value.t list option ->
  skipawaitable:bool ->
  default_dropthrough:Instruction_sequence.t option ->
  return_value:Instruction_sequence.t ->
  namespace:Namespace_env.env ->
  doc_comment:string option ->
  immediate_tparams:Tast.tparam list ->
  class_tparam_names:string list ->
  ?call_context:string ->
  Tast.fun_param list ->
  Aast.hint option ->
  Tast.program ->
  (* is_generator * is_pair_generator *)
  Hhas_body.t * bool * bool

val tparams_to_strings : Tast.tparam list -> string list

val emit_method_prolog :
  env:Emit_env.t ->
  pos:Pos.t ->
  params:Hhas_param.t list ->
  ast_params:Tast.fun_param list ->
  tparams:string list ->
  should_emit_init_this:bool ->
  Instruction_sequence.t

val emit_return_type_info :
  scope:Ast_scope.Scope.t ->
  skipawaitable:bool ->
  Aast.hint option ->
  Hhas_type_info.t

val emit_deprecation_warning :
  Ast_scope.Scope.t ->
  (* scope *)
  Typed_value.t list option ->
  (* deprecation_info *)
  Instruction_sequence.t

val emit_generics_upper_bounds :
  Tast.tparam list ->
  string list ->
  skipawaitable:bool ->
  (string * Hhas_type_info.t list) list
