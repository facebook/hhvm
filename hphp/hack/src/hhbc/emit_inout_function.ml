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

let emit_set_instrs ?(is_last=false) base (index, local) =
  let index = if is_last then 0 else index in
  gather [
    instr_setl local;
    instr_popc;
    instr_basel base H.MemberOpMode.Warn;
    instr_querym 0 H.QueryOp.CGet (H.MemberKey.EI (Int64.of_int index))
  ]

let emit_body_instrs ~verify_ret _env params renamed_method_id =
  let param_count = List.length params in
  let param_instrs = gather @@
    List.mapi params ~f:(fun i p ->
        gather [
          instr_cgetquietl (Local.Named (Hhas_param.name p));
          instr_fpass H.PassByRefKind.AllowCell i H.Cell
        ])
  in
  let inout_params = List.filter_mapi params ~f:(fun i p ->
      if not @@ Hhas_param.is_inout p then None else
        Some (i + 2, Local.Named (Hhas_param.name p))) in
  (* We know that this is safe since there must be at least 1 inout param *)
  let all_but_last, last =
    List.split_n inout_params (List.length inout_params - 1) in
  let last = List.hd_exn last in
  let local = Local.Unnamed param_count in
  let unset_instr = instr_unsetl local in
  let f_label = Label.next_fault () in
  let try_body = gather [
    emit_set_instrs local (1, local);
    gather @@ List.map all_but_last ~f:(emit_set_instrs local);
    emit_set_instrs ~is_last:true local last
  ] in
  let fault_body = gather [ unset_instr; instr_unwind ] in
  let verify_ret_instr = if verify_ret then instr_verifyRetTypeC else empty in
  gather [
    instr_fpushfuncd param_count renamed_method_id;
    param_instrs;
    instr_fcall param_count;
    instr_unboxr_nop;
    instr_try_fault f_label try_body fault_body;
    unset_instr;
    verify_ret_instr;
    instr_retc
  ]

(* Construct the wrapper function body *)
let make_wrapper_body return_type params instrs =
  Emit_body.make_body
    instrs
    [] (* decl_vars *)
    false (* is_memoize_wrapper *)
    params
    (Some return_type)
    [] (* static_inits: this is intentionally empty *)
    None (* doc *)

let emit_wrapper_function ~original_id ~renamed_id ~verify_ret ast_fun =
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
  let body_instrs = emit_body_instrs ~verify_ret env params renamed_id in
  let fault_instrs = extract_fault_instructions body_instrs in
  let body_instrs = gather [body_instrs; fault_instrs] in
  let params = List.map ~f:Hhas_param.switch_inout_to_reference params in
  let body = make_wrapper_body return_type_info params body_instrs in
  Hhas_function.make
    function_attributes
    original_id
    body
    (Hhas_pos.pos_to_span ast_fun.Ast.f_span)
    false false false true true true
