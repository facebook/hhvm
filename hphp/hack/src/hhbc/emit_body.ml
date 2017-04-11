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

module ULS = Unique_list_string

let add_local locals (_, name) =
  if name = "$GLOBALS" then locals else ULS.add locals name

class declvar_visitor = object(_this)
  inherit [ULS.t] Ast_visitor.ast_visitor as _super

  method! on_lvar locals id = add_local locals id
  method! on_efun locals _fn use_list =
    List.fold_left use_list ~init:locals
      ~f:(fun locals (x, _isref) -> add_local locals x)
  method! on_catch locals (_, x, _) = add_local locals x
end


(* Given an AST for a statement sequence, compute the local variables that
 * are referenced or defined in the block, in the order in which they appear.
 * Do not include function parameters or $GLOBALS, but *do* include $this
 *)
let decl_vars_from_ast params b =
  let visitor = new declvar_visitor in
  let decl_vars = visitor#on_block ULS.empty b in
  let param_names =
    List.fold_left
      params
        ~init:ULS.empty
        ~f:(fun l p -> ULS.add l @@ Hhas_param.name p)
  in
  let decl_vars = ULS.diff decl_vars param_names in
  List.rev (ULS.items decl_vars)

let from_ast ~class_name ~function_name ~has_this ~skipawaitable
  tparams params ret body default_instrs =
  let tparams = tparams_to_strings tparams in
  Label.reset_label ();
  Local.reset_local ();
  Iterator.reset_iterator ();
  Emit_expression.set_class_name class_name;
  Emit_expression.set_function_name function_name;
  Emit_expression.set_method_has_this has_this;
  let params = Emit_param.from_asts tparams params in
  let return_type_info =
    match ret with
    | None ->
      Some (Hhas_type_info.make (Some "") (Hhas_type_constraint.make None []))
    | Some h ->
      Some (hint_to_type_info
        ~skipawaitable ~always_extended:true ~tparams h) in
  let stmt_instrs = Emit_statement.from_stmts body in
  let stmt_instrs =
    if has_type_constraint return_type_info then
      verify_returns stmt_instrs
    else
      stmt_instrs in
  let ret_instrs =
    match List.last body with
    | Some (A.Return _) | Some (A.Throw _) -> empty
    | Some _ -> gather [instr_null; instr_retc]
    | None -> default_instrs return_type_info in
  let fault_instrs = extract_fault_instructions stmt_instrs in
  let begin_label, default_value_setters =
    Emit_param.emit_param_default_value_setter params
  in
  let (is_generator, is_pair_generator) = is_function_generator stmt_instrs in
  let generator_instr =
    if is_generator then gather [instr_createcont; instr_popc] else empty
  in
  let body_instrs = gather [
    begin_label;
    emit_method_prolog params;
    generator_instr;
    stmt_instrs;
    ret_instrs;
    default_value_setters;
    fault_instrs;
  ] in
  let body_instrs = rewrite_class_refs body_instrs in
  let params, body_instrs =
    Label_rewriter.relabel_function params body_instrs in
  let function_decl_vars = decl_vars_from_ast params body in
  let num_iters = !Iterator.num_iterators in
  let num_cls_ref_slots = get_num_cls_ref_slots body_instrs in
  body_instrs,
  function_decl_vars,
  num_iters,
  num_cls_ref_slots,
  params,
  return_type_info,
  is_generator,
  is_pair_generator
