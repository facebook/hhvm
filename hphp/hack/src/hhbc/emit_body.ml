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

module ULS = Unique_list_string

let add_local locals (_, name) =
  if name = "$GLOBALS" then locals else ULS.add locals name

class declvar_visitor = object(this)
  inherit [ULS.t] Ast_visitor.ast_visitor as _super

  method! on_lvar locals id = add_local locals id
  method! on_efun locals _fn use_list =
    List.fold_left use_list ~init:locals
      ~f:(fun locals (x, _isref) -> add_local locals x)
  method! on_catch locals (_, x, b) =
    this#on_block (add_local locals x) b
  method! on_class_ locals _ = locals
  method! on_fun_ locals _ = locals
end


(* Given an AST for a statement sequence, compute the local variables that
 * are referenced or defined in the block, in the order in which they appear.
 * Do not include function parameters or $GLOBALS, but *do* include $this
 *)
let decl_vars_from_ast params b =
  let visitor = new declvar_visitor in
  let decl_vars = visitor#on_program ULS.empty b in
  let param_names =
    List.fold_left
      params
        ~init:ULS.empty
        ~f:(fun l p -> ULS.add l @@ Hhas_param.name p)
  in
  let decl_vars = ULS.diff decl_vars param_names in
  List.rev (ULS.items decl_vars)

let emit_def def =
  match def with
  | Ast.Stmt s -> Emit_statement.from_stmt s
  | Ast.Constant c ->
    gather [
      Emit_expression.from_expr c.Ast.cst_value;
      instr (IIncludeEvalDefine (DefCns (snd c.Ast.cst_name)));
      instr_popc;
    ]
  | Ast.Class cd ->
    instr (IIncludeEvalDefine (DefCls (snd cd.Ast.c_name)))
  | Ast.Typedef td ->
    instr (IIncludeEvalDefine (DefTypeAlias (snd td.Ast.t_id)))
  | _ ->
    empty

let rec emit_defs defs =
  match defs with
  | [] -> Emit_statement.emit_dropthrough_return ()
  | [Ast.Stmt s] -> Emit_statement.emit_final_statement s
  | [d] ->
    gather [emit_def d; Emit_statement.emit_dropthrough_return ()]
  | d::defs ->
    let i1 = emit_def d in
    let i2 = emit_defs defs in
    gather [i1; i2]

let from_ast ~scope ~skipawaitable ~default_dropthrough ~return_value
  params ret body =
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _) -> s) in
  Label.reset_label ();
  Local.reset_local ();
  Iterator.reset_iterator ();
  Emit_expression.set_scope scope;
  let params = Emit_param.from_asts tparams params in
  let return_type_info =
    match ret with
    | None ->
      Some (Hhas_type_info.make (Some "") (Hhas_type_constraint.make None []))
    | Some h ->
      Some (hint_to_type_info
        ~skipawaitable ~always_extended:true ~tparams h) in
  let verify_return =
    match return_type_info with
    | None -> false
    | Some x when x. Hhas_type_info.type_info_user_type = Some "" -> false
    | Some x -> Hhas_type_info.has_type_constraint x in
  Emit_statement.set_verify_return verify_return;
  Emit_statement.set_default_dropthrough default_dropthrough;
  Emit_statement.set_default_return_value return_value;
  let stmt_instrs = emit_defs body in
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
    default_value_setters;
    fault_instrs;
  ] in
  let body_instrs = rewrite_user_labels body_instrs in
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
