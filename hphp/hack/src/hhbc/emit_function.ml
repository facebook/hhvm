(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Instruction_sequence
module A = Ast

(* Given a function definition, emit code, and in the case of <<__Memoize>>,
 * a wrapper function
 *)
let emit_function : A.fun_ * Closure_convert.hoist_kind -> Hhas_function.t list =
  fun (ast_fun, hoisted) ->
  let namespace = ast_fun.A.f_namespace in
  let original_id =
    Hhbc_id.Function.elaborate_id namespace ast_fun.A.f_name in
  let function_is_async =
    ast_fun.Ast.f_fun_kind = Ast_defs.FAsync
    || ast_fun.Ast.f_fun_kind = Ast_defs.FAsyncGenerator in
  let function_attributes =
    Emit_attribute.from_asts namespace ast_fun.Ast.f_user_attributes in
  let function_attributes = Emit_attribute.add_reified_attribute
    function_attributes ast_fun.Ast.f_tparams in
  let is_memoize = Hhas_attribute.has_memoized function_attributes in
  let is_native = Hhas_attribute.has_native function_attributes in
  let deprecation_info = Hhas_attribute.deprecation_info function_attributes in
  let is_no_injection = Hhas_attribute.is_no_injection function_attributes in
  let wrapper_type_opt, inout_param_locations =
    Emit_inout_helpers.extract_function_inout_or_ref_param_locations ast_fun in
  let has_inout_args = Option.is_some wrapper_type_opt in
  let renamed_id =
    if is_memoize
    then Hhbc_id.Function.add_suffix
      original_id Emit_memoize_helpers.memoize_suffix
    else if has_inout_args
    then Hhbc_id.Function.add_suffix
      original_id (Emit_inout_helpers.inout_suffix inout_param_locations)
    else original_id in
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let function_rx_level = Rx.rx_level_from_ast ast_fun.Ast.f_user_attributes
    |> Option.value ~default:Rx.NonRx in
  let ast_body, is_rx_body, function_rx_disabled =
    if function_rx_level <> Rx.NonRx then
      match Rx.halves_of_is_enabled_body namespace ast_fun.Ast.f_body with
      | Some (enabled_body, disabled_body) ->
        if Hhbc_options.rx_is_enabled !Hhbc_options.compiler_options
        then enabled_body, true, false
        else disabled_body, false, true
      | None -> ast_fun.Ast.f_body, true, false
    else ast_fun.Ast.f_body, false, false in
  let function_body, function_is_generator, function_is_pair_generator =
    Emit_body.emit_body
      ~pos: ast_fun.A.f_span
      ~scope
      ~is_closure_body:false
      ~is_memoize
      ~is_native
      ~is_async:function_is_async
      ~is_rx_body
      ~debugger_modify_program:false
      ~deprecation_info:(if is_memoize then None else deprecation_info)
      ~skipawaitable:(ast_fun.Ast.f_fun_kind = Ast_defs.FAsync)
      ~default_dropthrough:None
      ~return_value:instr_null
      ~namespace
      ~doc_comment:ast_fun.Ast.f_doc_comment
      ast_fun.Ast.f_tparams
      ast_fun.Ast.f_params
      ast_fun.Ast.f_ret
      [Ast.Stmt (Pos.none, Ast.Block ast_body)] in
  let normal_function_name =
    if wrapper_type_opt = Some Emit_inout_helpers.RefWrapper
    then original_id else renamed_id in
  let is_interceptable =
    Interceptable.is_function_interceptable namespace ast_fun in
  let normal_function =
    Hhas_function.make
      function_attributes
      normal_function_name
      function_body
      (Hhas_pos.pos_to_span ast_fun.Ast.f_span)
      function_is_async
      function_is_generator
      function_is_pair_generator
      hoisted
      is_no_injection
      false (*inout_wrapper*)
      is_interceptable
      is_memoize (*is_memoize_impl*)
      function_rx_level
      function_rx_disabled
  in
  let decl_vars = Hhas_body.decl_vars @@ Hhas_function.body normal_function in
  if is_memoize
  then [normal_function;
    Emit_memoize_function.emit_wrapper_function
      ~original_id
      ~renamed_id
      ~is_method:false
      ~deprecation_info
      ast_fun]
  else match wrapper_type_opt with
  | Some wrapper_type ->
    let wrapper_fn =
      Emit_inout_function.emit_wrapper_function
              ~decl_vars
              ~hoisted
              ~wrapper_type
              ~original_id
              ~renamed_id
              ast_fun in
      if wrapper_type = Emit_inout_helpers.InoutWrapper
      then [wrapper_fn; normal_function]
      else [normal_function; wrapper_fn]
  | None -> [normal_function]

let emit_functions_from_program ast =
  List.concat_map ast
  (fun (is_top, d) ->
    match d with Ast.Fun fd -> emit_function (fd, is_top) | _ -> [])
