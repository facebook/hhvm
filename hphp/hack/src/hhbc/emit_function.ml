(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Instruction_sequence
module A = Ast_defs
module T = Aast
module SU = Hhbc_string_utils

(* Given a function definition, emit code, and in the case of <<__Memoize>>,
 * a wrapper function
 *)
let emit_function (ast_fun, hoisted) : Hhas_function.t list =
  let namespace = ast_fun.T.f_namespace in
  let original_id = Hhbc_id.Function.from_ast_name (snd ast_fun.T.f_name) in
  let function_is_async =
    ast_fun.T.f_fun_kind = Ast_defs.FAsync
    || ast_fun.T.f_fun_kind = Ast_defs.FAsyncGenerator
  in
  let function_attributes =
    Emit_attribute.from_asts namespace ast_fun.T.f_user_attributes
  in
  let function_attributes =
    Emit_attribute.add_reified_attribute function_attributes ast_fun.T.f_tparams
  in
  let is_memoize = Hhas_attribute.has_memoized function_attributes in
  let is_native = Hhas_attribute.has_native function_attributes in
  let deprecation_info = Hhas_attribute.deprecation_info function_attributes in
  let is_no_injection = Hhas_attribute.is_no_injection function_attributes in
  let renamed_id =
    if is_memoize then
      Hhbc_id.Function.add_suffix
        original_id
        Emit_memoize_helpers.memoize_suffix
    else
      original_id
  in
  let fun_name = snd ast_fun.T.f_name in
  let fun_is_meth_caller = String.is_prefix ~prefix:"\\MethCaller$" fun_name in
  let call_context =
    if fun_is_meth_caller then
      match ast_fun.T.f_user_attributes with
      | [{ T.ua_name = (_, "__MethCaller"); T.ua_params = [(_, T.String ctx)] }]
        when ctx <> "" ->
        Some (ctx |> Hhbc_id.Class.from_ast_name |> Hhbc_id.Class.to_raw_string)
      | _ -> None
    else
      None
  in
  let is_debug_main =
    match ast_fun.T.f_user_attributes with
    | [{ T.ua_name = (_, "__DebuggerMain"); T.ua_params = [] }] -> true
    | _ -> false
  in
  let scope =
    if is_debug_main then
      Ast_scope.Scope.toplevel
    else
      [Ast_scope.ScopeItem.Function ast_fun]
  in
  let function_rx_level =
    Rx.rx_level_from_ast ast_fun.T.f_user_attributes
    |> Option.value ~default:Rx.NonRx
  in
  let (ast_body, is_rx_body, function_rx_disabled) =
    if function_rx_level <> Rx.NonRx then
      match Rx.halves_of_is_enabled_body ast_fun.T.f_body with
      | Some (enabled_body, disabled_body) ->
        if Hhbc_options.rx_is_enabled !Hhbc_options.compiler_options then
          (enabled_body, true, false)
        else
          (disabled_body, false, true)
      | None -> (ast_fun.T.f_body.T.fb_ast, true, false)
    else
      (ast_fun.T.f_body.T.fb_ast, false, false)
  in
  let (function_body, function_is_generator, function_is_pair_generator) =
    Emit_body.emit_body
      ~pos:ast_fun.T.f_span
      ~scope
      ~is_closure_body:false
      ~is_memoize
      ~is_native
      ~is_async:function_is_async
      ~is_rx_body
      ~debugger_modify_program:false
      ~deprecation_info:
        ( if is_memoize then
          None
        else
          deprecation_info )
      ~skipawaitable:(ast_fun.T.f_fun_kind = Ast_defs.FAsync)
      ~default_dropthrough:None
      ~return_value:instr_null
      ~namespace
      ~doc_comment:ast_fun.T.f_doc_comment
      ~immediate_tparams:ast_fun.T.f_tparams
      ~class_tparam_names:[]
      ?call_context
      ast_fun.T.f_params
      (T.hint_of_type_hint ast_fun.T.f_ret)
      [T.Stmt (Pos.none, T.Block ast_body)]
  in
  let is_interceptable = Interceptable.is_function_interceptable ast_fun in
  let normal_function =
    Hhas_function.make
      function_attributes
      renamed_id
      function_body
      (Hhas_pos.pos_to_span ast_fun.T.f_span)
      function_is_async
      function_is_generator
      function_is_pair_generator
      hoisted
      is_no_injection
      is_interceptable
      is_memoize (*is_memoize_impl*)
      function_rx_level
      function_rx_disabled
  in
  if is_memoize then
    [
      normal_function;
      Emit_memoize_function.emit_wrapper_function
        ~original_id
        ~renamed_id
        ~is_method:false
        ~deprecation_info
        ast_fun;
    ]
  else
    [normal_function]

let emit_functions_from_program
    (ast : (Closure_convert.hoist_kind * Tast.def) list) =
  let aux (is_top, def) =
    match def with
    | T.Fun fd -> emit_function (fd, is_top)
    | _ -> []
  in
  List.concat_map ast ~f:aux
