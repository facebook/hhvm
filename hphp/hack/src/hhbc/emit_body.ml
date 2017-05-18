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

let emit_method_prolog ~params ~needs_local_this =
  gather (
    (if needs_local_this
    then instr (IMisc (InitThisLoc (Local.Named "$this")))
    else empty)
    ::
    List.filter_map params (fun p ->
    let param_type_info = Hhas_param.type_info p in
    let param_name = Hhas_param.name p in
    if has_type_constraint param_type_info
    then Some (instr (IMisc (VerifyParamType (Param_named param_name))))
    else None))

let tparams_to_strings tparams =
  List.map tparams (fun (_, (_, s), _) -> s)

let rec emit_def def =
  match def with
  | Ast.Stmt s -> Emit_statement.emit_stmt s
  | Ast.Constant c ->
    let cns_name = snd c.Ast.cst_name in
    let cns_id =
      if c.Ast.cst_kind = Ast.Cst_define
      then Hhbc_id.Const.from_raw_string cns_name
      else Hhbc_id.Const.from_ast_name cns_name in
    gather [
      Emit_expression.from_expr ~need_ref:false c.Ast.cst_value;
      instr (IIncludeEvalDefine (DefCns cns_id));
      instr_popc;
    ]
  | Ast.Class cd ->
    instr (IIncludeEvalDefine
      (DefCls (Hhbc_id.Class.from_ast_name (snd cd.Ast.c_name))))
  | Ast.Typedef td ->
    instr (IIncludeEvalDefine
      (DefTypeAlias (Hhbc_id.Class.from_ast_name (snd td.Ast.t_id))))
    (* We assume that SetNamespaceEnv does namespace setting *)
  | Ast.Namespace(_, defs) ->
    gather (List.map defs emit_def)
  | Ast.SetNamespaceEnv nsenv ->
    Emit_expression.set_namespace nsenv;
    empty
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

let emit_body
  ~scope
  ~is_closure_body
  ~is_memoize_wrapper
  ~skipawaitable
  ~default_dropthrough
  ~return_value
  ~namespace params ret body =
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _) -> s) in
  Label.reset_label ();
  Iterator.reset_iterator ();
  Emit_expression.set_scope scope;
  let return_type_info =
    match ret with
    | None ->
      Some (Hhas_type_info.make (Some "") (Hhas_type_constraint.make None []))
    | Some h ->
      Some (hint_to_type_info
        ~skipawaitable ~always_extended:true ~tparams ~namespace h) in
  let verify_return =
    match return_type_info with
    | None -> false
    | Some x when x. Hhas_type_info.type_info_user_type = Some "" -> false
    | Some x -> Hhas_type_info.has_type_constraint x in
  Emit_statement.set_verify_return verify_return;
  Emit_statement.set_default_dropthrough default_dropthrough;
  Emit_statement.set_default_return_value return_value;
  Emit_expression.set_namespace namespace;
  let params = Emit_param.from_asts ~namespace ~tparams ~params in
  let has_this = Ast_scope.Scope.has_this scope in
  let needs_local_this, decl_vars =
    Decl_vars.from_ast ~is_closure_body ~has_this ~params body in
  Local.reset_local (List.length params + List.length decl_vars);
  Emit_expression.set_needs_local_this needs_local_this;
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
    emit_method_prolog ~params ~needs_local_this;
    generator_instr;
    stmt_instrs;
    default_value_setters;
    fault_instrs;
  ] in
  let body_instrs = rewrite_user_labels body_instrs in
  let body_instrs = rewrite_class_refs body_instrs in
  let params, body_instrs =
    Label_rewriter.relabel_function params body_instrs in
  let num_iters = !Iterator.num_iterators in
  let num_cls_ref_slots = get_num_cls_ref_slots body_instrs in
  Hhas_body.make
    body_instrs
    decl_vars
    num_iters
    num_cls_ref_slots
    is_memoize_wrapper
    params
    return_type_info,
    is_generator,
    is_pair_generator
