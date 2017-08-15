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
open Hhbc_ast.MemberOpMode
open Core
open Emit_memoize_helpers

let make_memoize_function_no_params_code ~non_null_return renamed_function_id =
  let local_cache = Local.Unnamed 0 in
  let local_guard = Local.Unnamed 1 in
  let label_0 = Label.Regular 0 in
  let label_1 = Label.Regular 1 in
  let label_2 = Label.Regular 2 in
  let label_3 = Label.Regular 3 in
  let label_4 = Label.Regular 4 in
  let label_5 = Label.Regular 5 in
  let needs_guard = not non_null_return in
  gather [
    optional needs_guard [
      instr_false; instr_staticlocinit local_guard static_memoize_cache_guard;
    ];
    instr_null;
    instr_staticlocinit local_cache static_memoize_cache;
    optional needs_guard [
      instr_null;
      instr_ismemotype;
      instr_jmpnz label_0];
    instr_cgetl local_cache;
    instr_dup;
    instr_istypec Hhbc_ast.OpNull;
    instr_jmpnz label_1;
    instr_retc;
    instr_label label_1;
    instr_popc;
    instr_label label_0;
    optional needs_guard [
      instr_null;
      instr_maybememotype;
      instr_jmpz label_2;
      instr_cgetl local_guard;
      instr_jmpz label_2;
      instr_null;
      instr_retc;
      instr_label label_2;
      instr_null;
      instr_ismemotype;
      instr_jmpnz label_3
      ];
    instr_fpushfuncd 0 renamed_function_id;
    instr_fcall 0;
    instr_unboxr;
    instr_setl local_cache;
    optional needs_guard [
      instr_jmp label_4;
      instr_label label_3;
      instr_fpushfuncd 0 renamed_function_id;
      instr_fcall 0;
      instr_unboxr;
      instr_label label_4;
      instr_null;
      instr_maybememotype;
      instr_jmpz label_5;
      instr_true;
      instr_setl local_guard;
      instr_popc;
      instr_label label_5;
      ];
    instr_retc ]

let make_memoize_function_with_params_code ~pos env params renamed_method_id =
  let param_count = List.length params in
  let static_local = Local.Unnamed param_count in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed (param_count + 1) in
  let begin_label, default_value_setters =
    (* Default value setters belong in the wrapper function not in the original function *)
    Emit_param.emit_param_default_value_setter env params
  in
  gather [
    begin_label;
    Emit_body.emit_method_prolog ~pos ~params:params ~needs_local_this:false;
    instr_typedvalue (Typed_value.Dict []);
    instr_staticlocinit static_local static_memoize_cache;
    param_code_sets params (param_count + 1);
    instr_basel static_local Warn;
    instr_memoget 0 first_local param_count;
    instr_isuninit;
    instr_jmpnz label;
    instr_cgetcunop;
    instr_retc;
    instr_label label;
    instr_ugetcunop;
    instr_popu;
    instr_fpushfuncd param_count renamed_method_id;
    param_code_gets params;
    instr_fcall param_count;
    instr_unboxr;
    instr_basel static_local Define;
    instr_memoset 0 first_local param_count;
    instr_retc;
    default_value_setters
  ]

let make_memoize_function_code ~pos ~non_null_return env params renamed_method_id =
  if List.is_empty params
  then make_memoize_function_no_params_code ~non_null_return renamed_method_id
  else make_memoize_function_with_params_code ~pos env params renamed_method_id

(* Construct the wrapper function body *)
let make_wrapper_body return_type params instrs =
  Emit_body.make_body
    instrs
    [] (* decl_vars *)
    true (* is_memoize_wrapper *)
    params
    (Some return_type)
    [] (* static_inits: this is intentionally empty *)
    None (* doc *)

let emit_wrapper_function ~original_id ~renamed_id ast_fun =
  Emit_memoize_helpers.check_memoize_possible (fst ast_fun.Ast.f_name)
    ~ret_by_ref: ast_fun.Ast.f_ret_by_ref
    ~params: ast_fun.Ast.f_params;
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let namespace = ast_fun.Ast.f_namespace in
  let pos = ast_fun.Ast.f_span in
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
  let non_null_return =
    cannot_return_null ast_fun.Ast.f_fun_kind ast_fun.Ast.f_ret in
  let body_instrs =
    make_memoize_function_code ~pos ~non_null_return env params renamed_id in
  let memoized_body =
    make_wrapper_body return_type_info params body_instrs in
  Hhas_function.make
    function_attributes
    original_id
    memoized_body
    (Hhas_pos.pos_to_span ast_fun.Ast.f_span)
    false false false true
