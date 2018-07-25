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

module H = Hhbc_ast
module SU = Hhbc_string_utils

let is_last_param_variadic param_count params =
  let last_p = List.nth_exn params (param_count - 1) in
  Hhas_param.is_variadic last_p

let emit_body_instrs_inout params call_instrs =
  let param_count = List.length params in
  let param_instrs = gather @@
    List.map params ~f:(fun p ->
      if Hhas_param.is_inout p
      then gather [
        instr_cgetquietl (Local.Named (Hhas_param.name p));
        instr_null;
        instr_popl (Local.Named (Hhas_param.name p))
      ]
      else instr_pushl (Local.Named (Hhas_param.name p))
    ) in
  let inout_params = List.filter_map params ~f:(fun p ->
      if not @@ Hhas_param.is_inout p then None else
        Some (instr_setl @@ Local.Named (Hhas_param.name p))) in
  let local = Local.get_unnamed_local () in
  let has_variadic = is_last_param_variadic param_count params in
  let param_count = if has_variadic then param_count - 1 else param_count in
  let num_inout = List.length inout_params in
  gather [
    gather @@ List.init num_inout ~f:(fun _ -> instr_nulluninit);
    call_instrs;
    param_instrs;
    instr_fcall param_count has_variadic (num_inout + 1);
    Emit_inout_helpers.emit_list_set_for_inout_call local inout_params;
    instr_retc
  ]

let emit_body_instrs_ref params call_instrs =
  let param_count = List.length params in
  let param_instrs = gather @@
    List.map params ~f:(fun p ->
      let local = Local.Named (Hhas_param.name p) in
      if Hhas_param.is_reference p then
        instr_vgetl local
      else
        instr_pushl local
    ) in
  let param_get_instrs =
    List.filter_map params ~f:(fun p ->
        if Hhas_param.is_reference p
        then Some (instr_cgetl (Local.Named (Hhas_param.name p))) else None) in
  let has_variadic = is_last_param_variadic param_count params in
  let param_count = if has_variadic then param_count - 1 else param_count in
  gather [
    call_instrs;
    param_instrs;
    instr_fcall param_count has_variadic 1;
    instr_unboxr_nop;
    gather param_get_instrs;
    instr_retm (List.length param_get_instrs + 1)
  ]

let emit_body_instrs ~wrapper_type env pos params call_instrs =
  let body =
    match wrapper_type with
    | Emit_inout_helpers.InoutWrapper ->
      emit_body_instrs_inout params call_instrs
    | Emit_inout_helpers.RefWrapper ->
      emit_body_instrs_ref params call_instrs
  in
  let begin_label, default_value_setters =
    Emit_param.emit_param_default_value_setter env pos params in
  gather [
    begin_label;
    Emit_pos.emit_pos pos;
    body;
    default_value_setters;
  ]

(* Construct the wrapper function body *)
let make_wrapper_body env doc decl_vars return_type params instrs =
  let params = List.map params ~f:Hhas_param.without_type in
  let return_user_type = Hhas_type_info.user_type return_type in
  let return_tc = Hhas_type_constraint.make None [] in
  let return_type = Hhas_type_info.make return_user_type return_tc in
  Emit_body.make_body
    instrs
    decl_vars
    false (* is_memoize_wrapper *)
    false (* is_memoize_wrapper_lsb *)
    params
    (Some return_type)
    [] (* static_inits: this is intentionally empty *)
    doc
    (Some env)

let emit_wrapper_function
  ~decl_vars ~is_top ~wrapper_type ~original_id ~renamed_id ast_fun =
  (* Wrapper methods may not have iterators *)
  Iterator.reset_iterator ();
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let namespace = ast_fun.Ast.f_namespace in
  let env = Emit_env.(
    empty |> with_namespace namespace |> with_scope scope
    ) in
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _, _) -> s) in
  let params = Emit_param.from_asts ~namespace ~tparams ~generate_defaults:true
    ~scope ast_fun.Ast.f_params in
  let function_attributes =
    Emit_attribute.from_asts namespace ast_fun.Ast.f_user_attributes
    |> Core_list.filter ~f:(fun attr -> not (Hhas_attribute.is_native attr)) in
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let return_type_info =
    Emit_body.emit_return_type_info
      ~scope ~skipawaitable:false ~namespace ast_fun.Ast.f_ret in
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
  Local.reset_local @@ List.length decl_vars + List.length params;
  let body_instrs =
    emit_body_instrs ~wrapper_type env ast_fun.Ast.f_span params call_instrs in
  let fault_instrs = extract_fault_funclets body_instrs in
  let body_instrs = gather [body_instrs; fault_instrs] in
  let doc = ast_fun.A.f_doc_comment in
  let body =
    make_wrapper_body env doc decl_vars return_type_info modified_params body_instrs
  in
  let return_by_ref = ast_fun.Ast.f_ret_by_ref in
  let is_interceptable =
    Interceptable.is_function_interceptable namespace ast_fun function_attributes in
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
    is_interceptable
    false (* is_memoize_impl *)

let emit_wrapper_method
  ~is_closure ~decl_vars ~original_id ~renamed_id ast_class ast_method =
  (* Wrapper methods may not have iterators *)
  Iterator.reset_iterator ();
  let decl_vars =
    if is_closure then List.filter decl_vars ((<>) "$0Closure") else decl_vars in
  let scope =
    [Ast_scope.ScopeItem.Method ast_method;
     Ast_scope.ScopeItem.Class ast_class] in
  let namespace = ast_class.Ast.c_namespace in
  let env = Emit_env.(
    empty |> with_namespace namespace |> with_scope scope
    ) in
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _, _) -> s) in
  let params = Emit_param.from_asts ~namespace ~tparams ~generate_defaults:true
    ~scope ast_method.Ast.m_params in
  let has_ref_params = List.exists params ~f:Hhas_param.is_reference in
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
  let method_is_return_by_ref = ast_method.Ast.m_ret_by_ref in
  let param_count = List.length params in
  let class_name =
    Hhbc_id.Class.from_ast_name @@ SU.Xhp.mangle @@ snd ast_class.A.c_name in
  let wrapper_type, original_id, renamed_id, params =
    if is_closure || has_ref_params then
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
  Local.reset_local @@ List.length decl_vars + List.length params;
  let body_instrs =
    emit_body_instrs ~wrapper_type env ast_method.Ast.m_span params call_instrs in
  let fault_instrs = extract_fault_funclets body_instrs in
  let body_instrs = gather [body_instrs; fault_instrs] in
  let params =
    if wrapper_type = Emit_inout_helpers.InoutWrapper then
      List.map ~f:Hhas_param.switch_inout_to_reference params
    else
      List.map ~f:Hhas_param.switch_reference_to_inout params
  in
  let doc = ast_method.A.m_doc_comment in
  let body =
    make_wrapper_body env doc decl_vars return_type_info params body_instrs
  in
  let method_is_interceptable =
    Interceptable.is_method_interceptable namespace ast_class original_id method_attributes in
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
    method_is_interceptable
    false (*method_is_memoize_impl*)
