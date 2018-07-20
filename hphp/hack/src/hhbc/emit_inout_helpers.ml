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

type wrapper_type = InoutWrapper | RefWrapper

let extract_inout_or_ref_param_locations ~is_sync ~is_byref ~is_closure_or_func params =
  let inout_param_locations = List.filter_mapi params
    ~f:(fun i p -> if p.Ast.param_callconv <> Some Ast.Pinout
                   then None else Some i) in
  if List.length inout_param_locations <> 0 then
    Some InoutWrapper, inout_param_locations
  else
    if not is_sync || is_byref
    then None, []
    else
    let module O = Hhbc_options in
    let need_wrapper =
      O.create_inout_wrapper_functions !O.compiler_options
      && (Emit_env.is_hh_syntax_enabled ())
      && (O.reffiness_invariance !O.compiler_options || is_closure_or_func)
      && not @@ List.exists params
        ~f:(fun p -> p.Ast.param_is_variadic && p.Ast.param_is_reference) in
    let l =
      if need_wrapper
      then List.filter_mapi params ~f:(fun i p -> Option.some_if p.Ast.param_is_reference i)
      else [] in
    if List.is_empty l then None, []
    else Some RefWrapper, l

let extract_function_inout_or_ref_param_locations fd =
  let is_byref = fd.Ast.f_ret_by_ref in
  let is_sync = fd.Ast.f_fun_kind = Ast.FSync in
  extract_inout_or_ref_param_locations
    ~is_byref
    ~is_closure_or_func:true
    ~is_sync
    fd.Ast.f_params

let extract_method_inout_or_ref_param_locations md ~is_closure_or_func =
  let is_byref = md.Ast.m_ret_by_ref in
  let is_sync = md.Ast.m_fun_kind = Ast.FSync in
  extract_inout_or_ref_param_locations
    ~is_byref
    ~is_closure_or_func
    ~is_sync
    md.Ast.m_params

let inout_suffix param_location =
  let param_location = List.map ~f:string_of_int param_location in
  "$"
  ^ (String.concat ";" param_location)
  ^ "$inout"

let emit_set_instrs ?(is_last=false) opt_base (index, set_instrs) =
  let index = if is_last then 0 else index in
  gather [
    set_instrs;
    instr_popc;
    match opt_base with
    | Some base ->
      gather [
        instr_basel base H.MemberOpMode.Warn;
        instr_querym 0 H.QueryOp.CGet (H.MemberKey.EI (Int64.of_int index))
      ]
    | None -> empty
  ]

let emit_list_set_for_inout_call local inout_params = Local.scope @@ fun () ->
  if List.length inout_params = 0 then empty else
  let inout_params = List.mapi ~f:(fun i p -> (i + 2, p)) inout_params in
  (* We know that this is safe since there must be at least 1 inout param *)
  let all_but_last, last =
    List.split_n inout_params (List.length inout_params - 1) in
  let last = List.hd_exn last in
  gather [
    instr_setl local;
    instr_popc;
    gather @@ List.map all_but_last ~f:(emit_set_instrs None);
    emit_set_instrs ~is_last:true None last;
    instr_pushl local
  ]
