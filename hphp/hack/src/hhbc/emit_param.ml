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

let from_variadic_param_hint_opt ho =
  let p = Pos.none in
  match ho with
  | None -> Some (p, A.Happly ((p, "array"), []))
  | Some h -> Some (p, A.Happly ((p, "array"), [h]))

let from_ast tparams p =
  let param_name = snd p.A.param_id in
  let param_is_variadic = p.Ast.param_is_variadic in
  let param_hint =
    if param_is_variadic
    then from_variadic_param_hint_opt p.Ast.param_hint
    else p.Ast.param_hint
  in
  let tparams = if param_is_variadic then "array"::tparams else tparams in
  let param_type_info = Option.map param_hint
    (hint_to_type_info ~skipawaitable:false ~always_extended:false ~tparams) in
  let param_default_value = Option.map p.Ast.param_expr
    ~f:(fun e -> Label.next_default_arg (), e)
  in
  if param_is_variadic && param_name = "..." then None else
  Some (Hhas_param.make param_name p.A.param_is_reference
        param_type_info param_default_value)

let from_asts tparams params =
  List.filter_map params (from_ast tparams)

let emit_param_default_value_setter params =
  let setters = List.filter_map params (fun p ->
    let param_name = Hhas_param.name p in
    let dvo = Hhas_param.default_value p in
    Option.map dvo (fun (l, e) ->
      gather [
        instr_label l;
        Emit_expression.from_expr e;
        instr_setl (Local.Named param_name);
        instr_popc;
      ]) )
  in
  if List.length setters = 0
  then empty, empty
  else
    let l = Label.next_regular () in
    instr_label l, gather [gather setters; instr_jmpns l]
