(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Instruction_sequence
open Hh_core

module H = Hhbc_ast

let emit_body_instrs_inout ~verify_ret params call_instrs =
  let param_count = List.length params in
  let param_instrs = gather @@
    List.mapi params ~f:(fun i p ->
        gather [
          instr_cgetquietl (Local.Named (Hhas_param.name p));
          instr_fpass H.PassByRefKind.AllowCell i H.Cell
        ]) in
  let inout_params = List.filter_map params ~f:(fun p ->
      if not @@ Hhas_param.is_inout p then None else
        Some (instr_setl @@ Local.Named (Hhas_param.name p))) in
  let local = Local.get_unnamed_local () in
  let verify_ret_instr = if verify_ret then instr_verifyRetTypeC else empty in
  gather [
    call_instrs;
    param_instrs;
    instr_fcall param_count;
    instr_unboxr_nop;
    Emit_inout_helpers.emit_list_set_for_inout_call local inout_params;
    verify_ret_instr;
    instr_retc
  ]

let emit_body_instrs_ref params call_instrs =
  let param_count = List.length params in
  let param_instrs = gather @@
    List.mapi params ~f:(fun i p ->
      let local = Local.Named (Hhas_param.name p) in
      if Hhas_param.is_reference p then
        gather [
          instr_vgetl local;
          instr_fpassvnop i H.Ref
        ]
      else
        gather [
          instr_cgetquietl local;
          instr_fpassc i H.Cell
        ]) in
  let param_get_instrs =
    List.filter_map params ~f:(fun p ->
        if Hhas_param.is_reference p
        then Some (instr_cgetl (Local.Named (Hhas_param.name p))) else None)
  in
  gather [
    call_instrs;
    param_instrs;
    instr_fcall param_count;
    instr_unboxr_nop;
    gather param_get_instrs;
    instr_new_vec_array (List.length param_get_instrs + 1);
    instr_retc;
  ]

let emit_body_instrs ~wrapper_type ~verify_ret env params call_instrs =
  let body =
    match wrapper_type with
    | Emit_inout_helpers.InoutWrapper ->
      emit_body_instrs_inout ~verify_ret params call_instrs
    | Emit_inout_helpers.RefWrapper ->
      emit_body_instrs_ref params call_instrs
  in
  let begin_label, default_value_setters =
    Emit_param.emit_param_default_value_setter env params in
  gather [
    begin_label;
    body;
    default_value_setters;
  ]

(* Construct the wrapper function body *)
let make_wrapper_body decl_vars return_type params instrs =
  Emit_body.make_body
    instrs
    decl_vars
    false (* is_memoize_wrapper *)
    params
    (Some return_type)
    [] (* static_inits: this is intentionally empty *)
    None (* doc *)

let emit_wrapper_function
  ~decl_vars ~is_top ~wrapper_type ~original_id ~renamed_id ast_fun =
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let namespace = ast_fun.Ast.f_namespace in
  let env = Emit_env.(
    empty |> with_namespace namespace |> with_scope scope
    ) in
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _) -> s) in
  let params = Emit_param.from_asts ~namespace ~tparams ~generate_defaults:true
    ~scope ast_fun.Ast.f_params in
  let function_attributes =
    Emit_attribute.from_asts namespace ast_fun.Ast.f_user_attributes in
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let return_type_info =
    Emit_body.emit_return_type_info
      ~scope ~skipawaitable:false ~namespace ast_fun.Ast.f_ret in
  let verify_ret =
    return_type_info.Hhas_type_info.type_info_user_type <> Some "" &&
    Hhas_type_info.has_type_constraint return_type_info in
  let param_count = List.length params in
  let modified_params, name, call_instrs =
    if wrapper_type = Emit_inout_helpers.InoutWrapper then
      List.map ~f:Hhas_param.switch_inout_to_reference params,
      original_id,
      instr_fpushfuncd param_count renamed_id
    else
      List.map ~f:Hhas_param.switch_reference_to_inout params,
      renamed_id,
      instr_fpushfuncd param_count original_id
  in
  let special_params = List.filter params
    ~f:(fun p -> Hhas_param.is_reference p || Hhas_param.is_inout p) in
  Local.reset_local @@ List.length decl_vars + List.length special_params + 1;
  let body_instrs =
    emit_body_instrs ~wrapper_type ~verify_ret env params call_instrs in
  let fault_instrs = extract_fault_instructions body_instrs in
  let body_instrs = gather [body_instrs; fault_instrs] in
  let body =
    make_wrapper_body decl_vars return_type_info modified_params body_instrs in
  let return_by_ref = ast_fun.Ast.f_ret_by_ref in
  Hhas_function.make
    function_attributes
    name
    body
    (Hhas_pos.pos_to_span ast_fun.Ast.f_span)
    false (* is_async *)
    false (* is_generator *)
    false (* is_pair_generator *)
    is_top
    true (* no_injection *)
    true (* inout_wrapper *)
    return_by_ref

let emit_wrapper_method
  ~is_closure ~decl_vars ~original_id ~renamed_id ast_class ast_method =
  let decl_vars = if is_closure then [] else decl_vars in
  let scope =
    [Ast_scope.ScopeItem.Method ast_method;
     Ast_scope.ScopeItem.Class ast_class] in
  let namespace = ast_class.Ast.c_namespace in
  let env = Emit_env.(
    empty |> with_namespace namespace |> with_scope scope
    ) in
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _) -> s) in
  let params = Emit_param.from_asts ~namespace ~tparams ~generate_defaults:true
    ~scope ast_method.Ast.m_params in
  let method_is_abstract =
    List.mem ast_method.Ast.m_kind Ast.Abstract ||
    ast_class.Ast.c_kind = Ast.Cinterface in
  let method_is_final = List.mem ast_method.Ast.m_kind Ast.Final in
  let method_is_static = not is_closure &&
    List.mem ast_method.Ast.m_kind Ast.Static in
  let method_attributes = Emit_attribute.from_asts
      (Emit_env.get_namespace env) ast_method.Ast.m_user_attributes in
  let method_is_private =
    List.mem ast_method.Ast.m_kind Ast.Private in
  let method_is_protected =
    List.mem ast_method.Ast.m_kind Ast.Protected in
  let method_is_public =
    List.mem ast_method.Ast.m_kind Ast.Public ||
    (not method_is_private && not method_is_protected) in
  let is_in_trait = ast_class.Ast.c_kind = Ast.Ctrait in
  let return_type_info =
    Emit_body.emit_return_type_info
      ~scope ~skipawaitable:false ~namespace ast_method.Ast.m_ret in
  let verify_ret =
    return_type_info.Hhas_type_info.type_info_user_type <> Some "" &&
    Hhas_type_info.has_type_constraint return_type_info in
  let method_is_return_by_ref = ast_method.Ast.m_ret_by_ref in
  let param_count = List.length params in
  let class_name = Hhbc_id.Class.from_ast_name @@ snd ast_class.A.c_name in
  let wrapper_type, original_id, renamed_id, params =
    if is_closure then
      Emit_inout_helpers.RefWrapper, renamed_id, original_id,
      List.map ~f:Hhas_param.switch_inout_to_reference params
    else
      Emit_inout_helpers.InoutWrapper, original_id, renamed_id, params
  in
  let call_instrs =
    if is_closure then
      gather [
        instr_this;
        instr_fpushfunc param_count []
      ]
    else if method_is_static then
      if is_in_trait then
        instr_fpushclsmethodsd param_count H.SpecialClsRef.Self renamed_id
      else
        instr_fpushclsmethodd param_count renamed_id class_name
    else
      gather [
        instr_this;
        instr_fpushobjmethodd param_count renamed_id Ast.OG_nullsafe
      ]
  in
  let special_params = List.filter params
    ~f:(fun p -> Hhas_param.is_reference p || Hhas_param.is_inout p) in
  Local.reset_local @@ List.length decl_vars + List.length special_params + 1;
  let body_instrs =
    emit_body_instrs ~wrapper_type ~verify_ret env params call_instrs in
  let fault_instrs = extract_fault_instructions body_instrs in
  let body_instrs = gather [body_instrs; fault_instrs] in
  let params =
    if wrapper_type = Emit_inout_helpers.InoutWrapper then
      List.map ~f:Hhas_param.switch_inout_to_reference params
    else
      List.map ~f:Hhas_param.switch_reference_to_inout params
  in
  let body = make_wrapper_body decl_vars return_type_info params body_instrs in
  Hhas_method.make
    method_attributes
    method_is_protected
    method_is_public
    method_is_private
    method_is_static
    method_is_final
    method_is_abstract
    true (*method_no_injection*)
    true (*method_inout_wrapper*)
    original_id
    body
    (Hhas_pos.pos_to_span ast_method.Ast.m_span)
    false (*method_is_async*)
    false (*method_is_generator*)
    false (*method_is_pair_generator*)
    false (*method_is_closure_body*)
    method_is_return_by_ref
