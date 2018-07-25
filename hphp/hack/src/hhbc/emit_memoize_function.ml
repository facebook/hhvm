(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Instruction_sequence
open Hh_core
open Emit_memoize_helpers

let make_memoize_function_no_params_code
  ~deprecation_info env renamed_function_id =
  let label = Label.Regular 0 in
  let scope = Emit_env.get_scope env in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info in
  gather [
    deprecation_body;
    instr_memoget label None;
    instr_retc;
    instr_label label;
    instr_fpushfuncd 0 renamed_function_id;
    instr_fcall 0 false 1;
    instr_unboxr;
    instr_memoset None;
    instr_retc
  ]

let make_memoize_function_with_params_code
  ~pos ~deprecation_info env params renamed_method_id =
  let param_count = List.length params in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed param_count in
  let begin_label, default_value_setters =
    (* Default value setters belong in the wrapper function not in the original function *)
    Emit_param.emit_param_default_value_setter env pos params
  in
  let scope = Emit_env.get_scope env in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info in
  gather [
    begin_label;
    Emit_body.emit_method_prolog ~pos ~params:params ~should_emit_init_this:false;
    deprecation_body;
    param_code_sets params param_count;
    instr_memoget label (Some (first_local, param_count));
    instr_retc;
    instr_label label;
    instr_fpushfuncd param_count renamed_method_id;
    param_code_gets params;
    instr_fcall param_count false 1;
    instr_unboxr;
    instr_memoset (Some (first_local, param_count));
    instr_retc;
    default_value_setters
  ]

let make_memoize_function_code
  ~pos ~deprecation_info env params renamed_method_id =
  Emit_pos.emit_pos_then pos @@
  if List.is_empty params
  then make_memoize_function_no_params_code
         ~deprecation_info env renamed_method_id
  else make_memoize_function_with_params_code
        ~pos ~deprecation_info env params renamed_method_id

(* Construct the wrapper function body *)
let make_wrapper_body env return_type params instrs =
  Emit_body.make_body
    instrs
    [] (* decl_vars *)
    true (* is_memoize_wrapper *)
    false (* is_memoize_wrapper_lsb *)
    params
    (Some return_type)
    [] (* static_inits: this is intentionally empty *)
    None (* doc *)
    (Some env)

let emit_wrapper_function
  ~original_id ~renamed_id ~is_method ~deprecation_info ast_fun =
  let ret_by_ref = ast_fun.Ast.f_ret_by_ref in
  Emit_memoize_helpers.check_memoize_possible (fst ast_fun.Ast.f_name)
    ~ret_by_ref
    ~params: ast_fun.Ast.f_params
    ~is_method;
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let namespace = ast_fun.Ast.f_namespace in
  let pos = ast_fun.Ast.f_span in
  let env = Emit_env.(
    empty |> with_namespace namespace |> with_scope scope
    ) in
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _, _) -> s) in
  let params = Emit_param.from_asts ~namespace ~tparams ~generate_defaults:true
    ~scope ast_fun.Ast.f_params in
  let function_attributes =
    Emit_attribute.from_asts namespace ast_fun.Ast.f_user_attributes in
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let return_type_info =
    Emit_body.emit_return_type_info
      ~scope ~skipawaitable:false ~namespace ast_fun.Ast.f_ret in
  let body_instrs =
    make_memoize_function_code
      ~pos ~deprecation_info env params renamed_id
  in
  let memoized_body =
    make_wrapper_body env return_type_info params body_instrs in
  let is_interceptable =
    Interceptable.is_function_interceptable namespace ast_fun function_attributes in
  Hhas_function.make
    function_attributes
    original_id
    memoized_body
    (Hhas_pos.pos_to_span ast_fun.Ast.f_span)
    false (* is_async *)
    false (* is_generator *)
    false (* is_pair_generator *)
    true  (* is_top *)
    false (* no_injection *)
    false (* inout_wrapper *)
    ret_by_ref
    is_interceptable
    false (* is_memoize_impl *)
