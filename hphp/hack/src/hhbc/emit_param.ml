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
open Instruction_sequence
module A = Ast

let from_variadic_param_hint_opt ho =
  let p = Pos.none in
  match ho with
  | None -> Some (p, A.Happly ((p, "array"), []))
  | Some h -> Some (p, A.Happly ((p, "array"), [h]))

let from_ast ~tparams ~namespace ~generate_defaults p =
  let param_name = snd p.A.param_id in
  let param_is_variadic = p.Ast.param_is_variadic in
  let param_hint =
    if param_is_variadic
    then from_variadic_param_hint_opt p.Ast.param_hint
    else p.Ast.param_hint
  in
  let tparams = if param_is_variadic then "array"::tparams else tparams in
  let nullable =
    match p.A.param_expr with
    | Some (_, A.Null) -> true
    | _ -> false in
  let param_type_info = Option.map param_hint
    Emit_type_hint.(hint_to_type_info
      ~kind:Param ~skipawaitable:false ~nullable ~namespace ~tparams) in
  let param_default_value =
    if generate_defaults
    then Option.map p.Ast.param_expr ~f:(fun e -> Label.next_default_arg (), e)
    else None
  in
  if param_is_variadic && param_name = "..." then None else
  Some (Hhas_param.make param_name p.A.param_is_reference param_is_variadic
    param_type_info param_default_value)

let rename_params params =
  let names = Core.List.fold_left params
    ~init:SSet.empty ~f:(fun n p -> SSet.add (Hhas_param.name p) n) in
  let rec rename param_counts param =
    let name = Hhas_param.name param in
    match SMap.get name param_counts with
    | None ->
      (SMap.add name 0 param_counts, param)
    | Some count ->
      let param_counts = SMap.add name (count + 1) param_counts in
      let newname = name ^ string_of_int count in
      if SSet.mem newname names
      then rename param_counts param
      else param_counts, (Hhas_param.with_name newname param)
  in
    List.rev (snd (Core.List.map_env SMap.empty (List.rev params) rename))

let from_asts ~namespace ~tparams ~generate_defaults ast_params =
  let hhas_params = List.filter_map ast_params
    (from_ast ~tparams ~namespace ~generate_defaults) in
  rename_params hhas_params

let emit_param_default_value_setter env params =
  let setters = List.filter_map params (fun p ->
    let param_name = Hhas_param.name p in
    let dvo = Hhas_param.default_value p in
    Option.map dvo (fun (l, e) ->
      gather [
        instr_label l;
        Emit_expression.emit_expr ~need_ref:false env e;
        instr_setl (Local.Named param_name);
        instr_popc;
      ]) )
  in
  if List.length setters = 0
  then empty, empty
  else
    let l = Label.next_regular () in
    instr_label l, gather [gather setters; instr_jmpns l]
