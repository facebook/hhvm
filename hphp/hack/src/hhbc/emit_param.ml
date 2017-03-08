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
open Emit_type_hint
open Instruction_sequence

let from_ast tparams p =
  let param_name = snd p.A.param_id in
  let param_type_info = Option.map p.Ast.param_hint
    (hint_to_type_info ~always_extended:false tparams) in
  let param_default_value = Option.map p.Ast.param_expr
    ~f:(fun e -> Label.next_default_arg (), e)
  in
  Hhas_param.make param_name param_type_info param_default_value

let from_asts tparams params =
  List.map params (from_ast tparams)

let emit_param_default_value_setter params =
  let setters = List.filter_map params (fun p ->
    let param_name = Hhas_param.name p in
    let dvo = Hhas_param.default_value p in
    Option.map dvo (fun (l, e) ->
      gather [
        instr_label l;
        Hhbc_from_nast.from_expr e;
        instr_setl (Local.Named param_name);
        instr_popc;
      ]) )
  in
  if List.length setters = 0
  then empty, empty
  else
    let l = Label.next_regular () in
    instr_label l, gather [gather setters; instr_jmpns l]
