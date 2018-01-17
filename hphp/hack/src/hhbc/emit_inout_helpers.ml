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

type wrapper_type = InoutWrapper | RefWrapper

let extract_inout_or_ref_param_locations ~is_closure_or_func params =
  let inout_param_locations = List.filter_mapi params
    ~f:(fun i p -> if p.Ast.param_callconv <> Some Ast.Pinout
                   then None else Some i) in
  if List.length inout_param_locations <> 0 then
    Some InoutWrapper, inout_param_locations
  else
    let module O = Hhbc_options in
    let need_wrapper =
      O.create_inout_wrapper_functions !O.compiler_options
      && (Emit_env.is_hh_syntax_enabled ())
      && (O.reffiness_invariance !O.compiler_options || is_closure_or_func) in
    let l =
      if need_wrapper
      then List.filter_mapi params ~f:(fun i p -> Option.some_if p.Ast.param_is_reference i)
      else [] in
    if List.is_empty l then None, []
    else Some RefWrapper, l

let inout_suffix param_location =
  let param_location = List.map ~f:string_of_int param_location in
  "$"
  ^ (String.concat ";" param_location)
  ^ "$inout"

let emit_set_instrs ?(is_last=false) base (index, set_instrs) =
  let index = if is_last then 0 else index in
  gather [
    set_instrs;
    instr_popc;
    instr_basel base H.MemberOpMode.Warn;
    instr_querym 0 H.QueryOp.CGet (H.MemberKey.EI (Int64.of_int index))
  ]

let emit_list_set_for_inout_call local inout_params = Local.scope @@ fun () ->
  if List.length inout_params = 0 then empty else
  let inout_params = List.mapi ~f:(fun i p -> (i + 2, p)) inout_params in
  (* We know that this is safe since there must be at least 1 inout param *)
  let all_but_last, last =
    List.split_n inout_params (List.length inout_params - 1) in
  let last = List.hd_exn last in
  let try_body =
    gather [
      emit_set_instrs local (1, instr_setl local);
      gather @@ List.map all_but_last ~f:(emit_set_instrs local);
      emit_set_instrs ~is_last:true local last
    ]
  in
  let unset_instr = instr_unsetl local in
  let f_label = Label.next_fault () in
  let fault_body = gather [ unset_instr; instr_unwind ] in
  gather [
    instr_try_fault f_label try_body fault_body;
    unset_instr
  ]
