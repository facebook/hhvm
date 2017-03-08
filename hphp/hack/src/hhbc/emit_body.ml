(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open Core
open Hhbc_ast
open Instruction_sequence
open Emit_type_hint

let has_type_constraint ti =
  match ti with
  | Some ti when (Hhas_type_info.has_type_constraint ti) -> true
  | _ -> false

let emit_method_prolog params =
  gather (List.filter_map params (fun p ->
    let param_type_info = Hhas_param.type_info p in
    let param_name = Hhas_param.name p in
    if has_type_constraint param_type_info
    then Some (instr (IMisc (VerifyParamType (Param_named param_name))))
    else None))

let tparams_to_strings tparams =
  List.map tparams (fun (_, (_, s), _) -> s)

let verify_returns body =
  let rewriter i =
    match i with
    | IContFlow RetC ->
      [ (IMisc VerifyRetTypeC); (IContFlow RetC) ]
    | _ -> [ i ] in
  InstrSeq.flat_map body ~f:rewriter

let from_ast tparams params ret b =
  let tparams = tparams_to_strings tparams in
  Label.reset_label ();
  Local.reset_local ();
  Iterator.reset_iterator ();
  let params = Emit_param.from_asts tparams params in
  let return_type_info = Option.map ret
    (hint_to_type_info ~always_extended:true tparams) in
  let stmt_instrs = Hhbc_from_nast.from_stmts b in
  let stmt_instrs =
    if has_type_constraint return_type_info then
      verify_returns stmt_instrs
    else
      stmt_instrs in
  let ret_instrs =
    match List.last b with Some (A.Return _) -> empty | _ ->
    gather [instr_null; instr_retc]
  in
  let fault_instrs = extract_fault_instructions stmt_instrs in
  let begin_label, default_value_setters =
    Emit_param.emit_param_default_value_setter params
  in
  let body_instrs = gather [
    begin_label;
    emit_method_prolog params;
    stmt_instrs;
    ret_instrs;
    default_value_setters;
    fault_instrs;
  ] in
  body_instrs, params, return_type_info
