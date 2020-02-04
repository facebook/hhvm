(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SU = Hhbc_string_utils
module SN = Naming_special_names
module T = Aast
open Core_kernel
open Instruction_sequence

let from_ast_wrapper privatize make_name ast_class ast_method =
  let (pos, original_name) = ast_method.T.m_name in
  let class_name = SU.Xhp.mangle @@ Utils.strip_ns @@ snd ast_class.T.c_name in
  (* TODO: use something that can't be faked in user code *)
  let method_is_closure_body =
    original_name = "__invoke" && String.is_prefix ~prefix:"Closure$" class_name
  in
  let namespace = ast_class.T.c_namespace in
  let method_attributes =
    Emit_attribute.from_asts namespace ast_method.T.m_user_attributes
  in
  let method_attributes =
    if method_is_closure_body then
      method_attributes
    else
      Emit_attribute.add_reified_attribute
        method_attributes
        ast_method.T.m_tparams
  in
  let is_native = Hhas_attribute.has_native method_attributes in
  let is_native_opcode_impl =
    Hhas_attribute.is_native_opcode_impl method_attributes
  in
  let method_is_final = ast_method.T.m_final in
  let method_is_abstract =
    ast_class.T.c_kind = Ast_defs.Cinterface || ast_method.T.m_abstract
  in
  let method_is_static = ast_method.T.m_static in
  let method_visibility =
    if is_native_opcode_impl then
      Aast.Public
    else if privatize then
      Aast.Private
    else
      ast_method.T.m_visibility
  in
  let is_memoize =
    Emit_attribute.ast_any_is_memoize ast_method.T.m_user_attributes
  in
  let deprecation_info =
    if is_memoize then
      None
    else
      Hhas_attribute.deprecation_info method_attributes
  in
  let is_no_injection = Hhas_attribute.is_no_injection method_attributes in
  let ret = T.hint_of_type_hint ast_method.T.m_ret in
  let method_id = make_name ast_method.T.m_name in
  if not (method_is_static || method_is_closure_body) then
    List.iter ast_method.T.m_params ~f:(fun p ->
        if p.T.param_name = SN.SpecialIdents.this then
          Emit_fatal.raise_fatal_parse p.T.param_pos "Cannot re-assign $this");
  let method_is_async =
    ast_method.T.m_fun_kind = Ast_defs.FAsync
    || ast_method.T.m_fun_kind = Ast_defs.FAsyncGenerator
  in
  if
    ast_class.T.c_kind = Ast_defs.Cinterface
    && not (List.is_empty ast_method.T.m_body.T.fb_ast)
  then
    Emit_fatal.raise_fatal_parse
      pos
      (Printf.sprintf
         "Interface method %s::%s cannot contain body"
         class_name
         original_name);
  if
    (not method_is_static)
    && ast_class.T.c_final
    && ast_class.T.c_kind = Ast_defs.Cabstract
  then
    Emit_fatal.raise_fatal_parse
      pos
      ( "Class "
      ^ class_name
      ^ " contains non-static method "
      ^ original_name
      ^ " and therefore cannot be declared 'abstract final'" );
  let has_variadic_param =
    List.exists ast_method.T.m_params ~f:(fun p -> p.T.param_is_variadic)
  in
  if has_variadic_param && original_name = Naming_special_names.Members.__call
  then
    Emit_fatal.raise_fatal_parse pos
    @@ Printf.sprintf
         "Method %s::__call() cannot take a variadic argument"
         class_name;
  let default_dropthrough =
    if ast_method.T.m_abstract then
      Some
        (Emit_fatal.emit_fatal_runtimeomitframe
           pos
           ( "Cannot call abstract method "
           ^ class_name
           ^ "::"
           ^ original_name
           ^ "()" ))
    else
      None
  in
  let scope =
    [Ast_scope.ScopeItem.Method ast_method; Ast_scope.ScopeItem.Class ast_class]
  in
  let scope =
    if method_is_closure_body then
      Ast_scope.ScopeItem.Lambda (method_is_async, None) :: scope
    else
      scope
  in
  let closure_namespace =
    SMap.find_opt class_name (Emit_env.get_closure_namespaces ())
  in
  let namespace = Option.value closure_namespace ~default:namespace in
  let method_rx_level =
    match Rx.rx_level_from_ast ast_method.T.m_user_attributes with
    | Some l -> l
    | None ->
      if method_is_closure_body then
        Emit_env.get_lambda_rx_of_scope ast_class ast_method
      else
        Rx.NonRx
  in
  let (ast_body_block, is_rx_body, method_rx_disabled) =
    if method_rx_level <> Rx.NonRx then
      match Rx.halves_of_is_enabled_body ast_method.T.m_body with
      | Some (enabled_body, disabled_body) ->
        if Hhbc_options.rx_is_enabled !Hhbc_options.compiler_options then
          (enabled_body, true, false)
        else
          (disabled_body, false, true)
      | None -> (ast_method.T.m_body.T.fb_ast, true, false)
    else
      (ast_method.T.m_body.T.fb_ast, false, false)
  in
  let class_tparam_names =
    List.map ast_class.T.c_tparams.T.c_tparam_list (fun t -> snd t.T.tp_name)
  in
  let (method_body, method_is_generator, method_is_pair_generator) =
    if is_native_opcode_impl then
      ( Emit_native_opcode.emit_body
          scope
          namespace
          ast_class.T.c_user_attributes
          ast_method.T.m_name
          ast_method.T.m_params
          ret,
        false,
        false )
    else
      Emit_body.emit_body
        ~pos:ast_method.T.m_span
        ~scope
        ~is_closure_body:method_is_closure_body
        ~is_memoize
        ~is_native
        ~is_async:method_is_async
        ~is_rx_body
        ~debugger_modify_program:false
        ~deprecation_info
        ~skipawaitable:(ast_method.T.m_fun_kind = Ast_defs.FAsync)
        ~default_dropthrough
        ~return_value:instr_null
        ~namespace
        ~doc_comment:ast_method.T.m_doc_comment
        ~immediate_tparams:ast_method.T.m_tparams
        ~class_tparam_names
        ast_method.T.m_params
        ret
        [T.Stmt (Pos.none, T.Block ast_body_block)]
  in
  let method_is_interceptable =
    Interceptable.is_method_interceptable ast_class method_id
  in
  let method_span =
    if is_native_opcode_impl then
      (0, 0)
    else
      Hhas_pos.pos_to_span ast_method.T.m_span
  in
  let normal_function =
    Hhas_method.make
      method_attributes
      method_visibility
      method_is_static
      method_is_final
      method_is_abstract
      is_no_injection
      method_id
      method_body
      method_span
      method_is_async
      method_is_generator
      method_is_pair_generator
      method_is_closure_body
      method_is_interceptable
      is_memoize (*method_is_memoize_impl*)
      method_rx_level
      method_rx_disabled
  in
  [normal_function]

let from_ast class_ method_ =
  let is_memoize =
    Emit_attribute.ast_any_is_memoize method_.T.m_user_attributes
  in
  let make_name (_, name) =
    if is_memoize then
      Hhbc_id.Method.from_ast_name (name ^ Emit_memoize_helpers.memoize_suffix)
    else
      Hhbc_id.Method.from_ast_name name
  in
  from_ast_wrapper is_memoize make_name class_ method_

let from_asts class_ methods = List.concat_map methods ~f:(from_ast class_)
