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
open Emit_memoize_helpers

module R = Hhbc_string_utils.Reified

let make_memoize_function_no_params_code
  ~deprecation_info env renamed_function_id is_async =
  let notfound = Label.next_regular() in
  let suspended_get = Label.next_regular() in
  let eager_set = Label.next_regular() in
  let scope = Emit_env.get_scope env in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info in
  let fcall_args =
    if is_async then make_fcall_args ~async_eager_label:eager_set 0
    else make_fcall_args 0
  in
  gather [
    deprecation_body;
    if is_async then
      gather [
        instr_memoget_eager notfound suspended_get None;
        instr_retc;
        instr_label suspended_get;
        instr_retc_suspended
      ] else
      gather [
        instr_memoget notfound None;
        instr_retc
      ];
    instr_label notfound;
    instr_nulluninit; instr_nulluninit; instr_nulluninit;
    instr_fcallfuncd fcall_args renamed_function_id;
    instr_memoset None;
    if is_async then
      gather [
        instr_retc_suspended;
        instr_label eager_set;
        instr_memoset_eager None;
        instr_retc
      ] else
      gather [ instr_retc ]
  ]

let make_memoize_function_with_params_code
  ~pos ~deprecation_info
  env params ast_params renamed_method_id is_async is_reified =
  let param_count = List.length params in
  let notfound = Label.next_regular() in
  let suspended_get = Label.next_regular() in
  let eager_set = Label.next_regular() in
  (* The local that contains the reified generics is the first non parameter
   * local, so the first local is parameter count + 1 when there are reified =
   * generics *)
  let add_reified = if is_reified then 1 else 0 in
  let first_local = Local.Unnamed (param_count + add_reified) in
  let begin_label, default_value_setters =
    (* Default value setters belong in the wrapper function not in the original function *)
    Emit_param.emit_param_default_value_setter env pos params
  in
  let scope = Emit_env.get_scope env in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info in
  let fcall_args =
    if is_async then make_fcall_args ~async_eager_label:eager_set param_count
    else make_fcall_args param_count
  in
  let fcall_instrs =
    if not is_reified
    then instr_fcallfuncd fcall_args renamed_method_id
    else gather [
      instr_cgetl (Local.Named R.reified_generics_local_name);
      instr_reified_name
        (Hhbc_id.Function.to_raw_string renamed_method_id);
      instr_fcallfunc fcall_args []
    ]
  in
  let reified_memokeym =
    if not is_reified then empty else
    gather @@ getmemokeyl param_count (param_count + add_reified)
                R.reified_generics_local_name in
  let param_count = param_count + add_reified in
  gather [
    begin_label;
    Emit_body.emit_method_prolog
      ~env ~pos ~params ~ast_params ~should_emit_init_this:false;
    deprecation_body;
    param_code_sets params param_count;
    reified_memokeym;
    if is_async then
      gather [
        instr_memoget_eager notfound suspended_get (Some (first_local, param_count));
        instr_retc;
        instr_label suspended_get;
        instr_retc_suspended
      ] else
      gather [
        instr_memoget notfound (Some (first_local, param_count));
        instr_retc
      ];
    instr_label notfound;
    instr_nulluninit; instr_nulluninit; instr_nulluninit;
    param_code_gets params;
    fcall_instrs;
    instr_memoset (Some (first_local, param_count));
    if is_async then
      gather [
        instr_retc_suspended;
        instr_label eager_set;
        instr_memoset_eager (Some (first_local, param_count));
        instr_retc
      ] else
      gather [ instr_retc ];
    default_value_setters
  ]

let make_memoize_function_code
  ~pos ~deprecation_info
  env params ast_params renamed_method_id is_async is_reified =
  Emit_pos.emit_pos_then pos @@
  if List.is_empty params && not is_reified
  then make_memoize_function_no_params_code
         ~deprecation_info env renamed_method_id is_async
  else make_memoize_function_with_params_code
        ~pos ~deprecation_info env params ast_params renamed_method_id
        is_async is_reified

(* Construct the wrapper function body *)
let make_wrapper_body env return_type params instrs is_reified =
  Emit_body.make_body
    instrs
    (if is_reified then [R.reified_generics_local_name] else []) (* decl_vars *)
    true (* is_memoize_wrapper *)
    false (* is_memoize_wrapper_lsb *)
    params
    (Some return_type)
    None (* doc *)
    (Some env)

let emit_wrapper_function
  ~original_id ~renamed_id ~is_method ~deprecation_info ast_fun =
  Emit_memoize_helpers.check_memoize_possible (fst ast_fun.Ast.f_name)
    ~params: ast_fun.Ast.f_params
    ~is_method;
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let namespace = ast_fun.Ast.f_namespace in
  let pos = ast_fun.Ast.f_span in
  let env = Emit_env.(
    empty |> with_namespace namespace |> with_scope scope
    ) in
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun t -> snd t.Ast.tp_name) in
  let params = Emit_param.from_asts ~namespace ~tparams ~generate_defaults:true
    ~scope ast_fun.Ast.f_params in
  let function_attributes =
    Emit_attribute.from_asts namespace ast_fun.Ast.f_user_attributes in
  let function_attributes = Emit_attribute.add_reified_attribute
    function_attributes ast_fun.Ast.f_tparams in
  let function_is_async = ast_fun.Ast.f_fun_kind = Ast_defs.FAsync in
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let return_type_info =
    Emit_body.emit_return_type_info
      ~scope ~skipawaitable:function_is_async ~namespace ast_fun.Ast.f_ret in
  let is_reified =
    List.exists ~f:(fun t -> t.A.tp_reified) ast_fun.Ast.f_tparams in
  let body_instrs =
    make_memoize_function_code
      ~pos ~deprecation_info
      env params ast_fun.Ast.f_params renamed_id function_is_async is_reified
  in
  let function_rx_level = Rx.rx_level_from_ast ast_fun.Ast.f_user_attributes
    |> Option.value ~default:Rx.NonRx in
  let env = Emit_env.with_rx_body (function_rx_level <> Rx.NonRx) env in
  let memoized_body =
    make_wrapper_body env return_type_info params body_instrs is_reified in
  let is_interceptable =
    Interceptable.is_function_interceptable namespace ast_fun in
  Hhas_function.make
    function_attributes
    original_id
    memoized_body
    (Hhas_pos.pos_to_span ast_fun.Ast.f_span)
    function_is_async
    false (* is_generator *)
    false (* is_pair_generator *)
    Closure_convert.TopLevel
    false (* no_injection *)
    false (* inout_wrapper *)
    is_interceptable
    false (* is_memoize_impl *)
    function_rx_level
    false (* function_rx_disabled *)
