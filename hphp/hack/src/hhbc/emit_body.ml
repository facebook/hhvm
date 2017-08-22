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
module SU = Hhbc_string_utils

let has_type_constraint ti =
  match ti with
  | Some ti when (Hhas_type_info.has_type_constraint ti) -> true
  | _ -> false

let emit_method_prolog ~pos ~params ~needs_local_this =
  Emit_pos.emit_pos_then pos @@
  gather (
    (if needs_local_this
    then instr (IMisc (InitThisLoc (Local.Named "$this")))
    else empty)
    ::
    List.filter_map params (fun p ->
    if Hhas_param.is_variadic p
    then None else
    let param_type_info = Hhas_param.type_info p in
    let param_name = Hhas_param.name p in
    if has_type_constraint param_type_info
    then Some (instr (IMisc (VerifyParamType (Param_named param_name))))
    else None))

let tparams_to_strings tparams =
  List.map tparams (fun (_, (_, s), _) -> s)

let rec emit_def env def =
  match def with
  | Ast.Stmt s -> Emit_statement.emit_stmt env s
  | Ast.Constant c ->
    let cns_name = snd c.Ast.cst_name in
    let cns_id =
      if c.Ast.cst_kind = Ast.Cst_define
      then
        (* names of constants declared using 'define function' are always
          prefixed with '\\', see 'def' function in 'namespaces.ml' *)
        Hhbc_id.Const.from_raw_string (SU.strip_global_ns cns_name)
      else Hhbc_id.Const.from_ast_name cns_name in
    gather [
      Emit_expression.emit_expr ~need_ref:false env c.Ast.cst_value;
      instr (IIncludeEvalDefine (DefCns cns_id));
      instr_popc;
    ]
    (* We assume that SetNamespaceEnv does namespace setting *)
  | Ast.Namespace(_, defs) ->
    emit_defs env defs
  | _ ->
    empty

and emit_defs env defs =
  let rec aux env defs =
    match defs with
    | Ast.SetNamespaceEnv ns :: defs ->
      let env = Emit_env.with_namespace ns env in
      aux env defs
    | [] -> Emit_statement.emit_dropthrough_return env
    (* emit last statement in the list as final statement *)
    | [Ast.Stmt s]
    (* emit statement as final if it is one before the last and last statement is
       empty markup (which will be no-op) *)
    | [Ast.Stmt s; Ast.Stmt (Ast.Markup ((_, ""), None))] ->
      Emit_statement.emit_final_statement env s
    | [d] ->
      gather [emit_def env d; Emit_statement.emit_dropthrough_return env]
    | d::defs ->
      let i1 = emit_def env d in
      let i2 = aux env defs in
      gather [i1; i2]
  in
  match defs with
  | Ast.Stmt (Ast.Markup ((_, s), echo_expr_opt))::defs ->
    let i1 =
      Emit_statement.emit_markup env s echo_expr_opt ~check_for_hashbang:true
    in
    let i2 = aux env defs in
    gather [i1; i2]
  | defs -> aux env defs

let make_body body_instrs decl_vars is_memoize_wrapper params return_type_info
              static_inits doc_comment =
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
    return_type_info
    static_inits
    doc_comment

let emit_return_type_info ~scope ~skipawaitable ~namespace ret =
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _) -> s) in
  match ret with
  | None ->
    Hhas_type_info.make (Some "") (Hhas_type_constraint.make None [])
  | Some h ->
    Emit_type_hint.(hint_to_type_info
      ~kind:Return ~nullable:false ~skipawaitable ~tparams ~namespace h)

let emit_deprecation_warning scope = function
  | None -> empty
  | Some args ->
    (* Drop the indexes *)
    let args = List.filteri ~f:(fun i _ -> i mod 2 <> 0) args in
    let strip_id id = SU.strip_global_ns (snd id) in
    let class_name, trait_instrs, concat_instruction =
      match Ast_scope.Scope.get_class scope with
      | None -> "", empty, empty
      | Some c when c.Ast.c_kind = Ast.Ctrait ->
        "::", gather [instr_self; instr_clsrefname;], instr_concat
      | Some c -> strip_id c.Ast.c_name ^ "::", empty, empty
    in
    let fn_name =
      match scope with
      | Ast_scope.ScopeItem.Function fd :: _ -> strip_id fd.Ast.f_name
      | Ast_scope.ScopeItem.Method md :: _ -> strip_id md.Ast.m_name
      | _ -> failwith "deprecated functions must have names"
    in
    let deprecation_string = class_name ^ fn_name ^ ": " ^
      match args with
      | [] -> "deprecated function"
      | Typed_value.String s :: _ -> s
      | _ -> failwith "deprecated attribute first argument is not a string"
    in
    let sampling_rate =
      match args with
      | [] | [_] -> Int64.one
      | _ :: Typed_value.Int i :: _ -> i
      | _ -> failwith "deprecated attribute second argument is not an integer"
    in
    (* TODO: if the function is system function or a native function,
     * emit PHP_DEPRECATED error code *)
    let user_deprecated_error_code = 16384 (* USER_DEPRECATED *) in
    if Int64.to_int sampling_rate <= 0 then empty else
    gather [
      trait_instrs;
      instr_string deprecation_string;
      concat_instruction;
      instr_int64 sampling_rate;
      instr_int user_deprecated_error_code;
      instr_trigger_sampled_error;
      instr_popr;
    ]

let emit_body
  ~pos
  ~scope
  ~is_closure_body
  ~is_memoize
  ~is_async
  ~deprecation_info
  ~skipawaitable
  ~is_return_by_ref
  ~default_dropthrough
  ~return_value
  ~namespace
  ~doc_comment params ret body =
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _) -> s) in
  Label.reset_label ();
  Iterator.reset_iterator ();
  let return_type_info =
    emit_return_type_info ~scope ~skipawaitable ~namespace ret in
  let is_generator, is_pair_generator = Generator.is_function_generator body in
  let verify_return =
    return_type_info.Hhas_type_info.type_info_user_type <> Some "" &&
    Hhas_type_info.has_type_constraint return_type_info && not is_generator in
  let default_dropthrough =
    if default_dropthrough <> None then default_dropthrough else begin
    let verify_return_instr =
      match ret with
      | None
      | Some (_, (A.Happly ((_, "void"), [])
                | A.Happly ((_, "WaitHandle"), [])
                | A.Happly ((_, "Awaitable"),
                  [_, A.Happly ((_, "void"), [])])
                | A.Happly ((_, "WaitHandle"),
                  [_, A.Happly ((_, "void"), [])]))) -> empty
      | _ when is_generator || is_pair_generator -> empty
      | _ -> instr_verifyRetTypeC
    in
    if is_async
    then Some (gather [instr_null; verify_return_instr; instr_retc])
    else None end
  in
  Emit_statement.set_verify_return verify_return;
  Emit_statement.set_default_dropthrough default_dropthrough;
  Emit_statement.set_default_return_value return_value;
  Emit_statement.set_return_by_ref is_return_by_ref;
  let params =
    Emit_param.from_asts
      ~namespace ~tparams ~generate_defaults:(not is_memoize) ~scope params
  in
  let has_this = Ast_scope.Scope.has_this scope in
  let needs_local_this, decl_vars =
    Decl_vars.from_ast ~is_closure_body ~has_this ~params:params body in
  let adjust_closure = if is_closure_body then 1 else 0 in
  Jump_targets.reset ();
  Local.reset_local
    (List.length params + List.length decl_vars + adjust_closure);
  let env = Emit_env.(
    empty |>
    with_namespace namespace |>
    with_needs_local_this needs_local_this |>
    with_scope scope) in
  let stmt_instrs = emit_defs env body in
  let begin_label, default_value_setters =
    Emit_param.emit_param_default_value_setter env params in
  let generator_instr =
    if is_generator then gather [instr_createcont; instr_popc] else empty
  in
  let svar_map = Static_var.make_static_map body in
  let stmt_instrs =
    rewrite_static_instrseq svar_map
                    (Emit_expression.emit_expr ~need_ref:false) env stmt_instrs
  in
  let first_instruction_is_label =
    match Instruction_sequence.first stmt_instrs with
    | Some (ILabel _) -> true
    | _ -> false
  in
  let header = gather [
        begin_label;
        emit_method_prolog ~pos ~params ~needs_local_this;
        emit_deprecation_warning scope deprecation_info;
        generator_instr;
      ]
  in
  (* per comment in emitter.cpp there should be no
   * jumps to the base of the function - inject EntryNop
   * if first instruction in the statement list is label
   * and prologue is empty *)
  let header =
    if first_instruction_is_label
      && Instruction_sequence.is_empty header
    then instr_entrynop
    else header
  in
  let svar_instrs = SMap.fold (fun name _ lst -> name::lst) svar_map [] in
  let body_instrs = gather [
    header;
    stmt_instrs;
    default_value_setters;
  ] in
  let fault_instrs = extract_fault_instructions body_instrs in
  let body_instrs = gather [body_instrs; fault_instrs] in
  make_body
    body_instrs
    decl_vars
    false (*is_memoize_wrapper*)
    params
    (Some return_type_info)
    svar_instrs
    doc_comment,
    is_generator,
    is_pair_generator
