(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Core_kernel
open Hhbc_ast
open Instruction_sequence
module A = Ast
module SU = Hhbc_string_utils
module RGH = Reified_generics_helpers

let has_type_constraint env ti ast_param =
  match ti, ast_param.A.param_hint with
  | Some ti, Some h when (Hhas_type_info.has_type_constraint ti) ->
    RGH.has_reified_type_constraint env h
  | _ -> RGH.NoConstraint

let emit_method_prolog ~env ~pos ~params ~ast_params ~should_emit_init_this =
  let instr_list =
    let init_this = if not should_emit_init_this then empty else
      instr (IMisc (InitThisLoc (Local.Named "$this"))) in
    let ast_params = List.filter ast_params
      ~f:(fun p -> not (p.A.param_is_variadic && snd p.A.param_id = "..."))
    in
    init_this ::
    List.filter_map (List.zip_exn params ast_params) (fun (p, ast_p) ->
      if Hhas_param.is_variadic p
      then None else
      let param_type_info = Hhas_param.type_info p in
      let param_name = Param_named (Hhas_param.name p) in
      begin match has_type_constraint env param_type_info ast_p with
      | RGH.NoConstraint -> None
      | RGH.NotReified ->
        Some (instr (IMisc (VerifyParamType param_name)))
      | RGH.MaybeReified ->
        Some (gather [
          Emit_expression.get_type_structure_for_hint env ~targ_map:SMap.empty
            @@ Option.value_exn ast_p.A.param_hint;
          instr (IMisc (VerifyParamTypeTS param_name))
        ])
      | RGH.DefinitelyReified ->
        Some (gather [
          fst @@ Emit_expression.emit_reified_arg env pos ~isas:false
            @@ Option.value_exn ast_p.A.param_hint;
          instr (IMisc (VerifyParamTypeTS param_name))
        ])
      end) in
  if List.is_empty instr_list
  then empty
  else gather (Emit_pos.emit_pos pos :: instr_list)


let tparams_to_strings tparams =
  List.map tparams (fun t -> snd t.A.tp_name)

let rec emit_def env def =
  match def with
  | Ast.Stmt s -> Emit_statement.emit_stmt env s
  | Ast.Constant c ->
    let cns_name = snd c.Ast.cst_name in
    let cns_id = Hhbc_id.Const.from_ast_name cns_name in
      gather [
        Emit_expression.emit_expr ~need_ref:false env c.Ast.cst_value;
        Emit_pos.emit_pos_then c.Ast.cst_span
          @@ instr (IIncludeEvalDefine (DefCns cns_id));
        instr_popc;
      ]
    (* We assume that SetNamespaceEnv does namespace setting *)
  | Ast.Namespace(_, defs) ->
    emit_defs env defs
  | _ ->
    empty

and check_namespace_update env ns =
  let p = Pos.none in
  let prev_ns = Emit_env.get_namespace env in
  let check_map2 prev curr =
    (* Check whether any definition updates an earlier definition *)
    SMap.iter (fun key value ->
                match SMap.get key prev with
                | None -> () (* new element, it is fine *)
                | Some v ->
                  if v = value then () (* same element, it is fine *)
                  else Emit_fatal.raise_fatal_parse p @@ Printf.sprintf
                    "Cannot use namespace %s as %s because the name is already in use"
                    (SU.strip_global_ns value) key
                ) curr
  in
  check_map2 prev_ns.Namespace_env.ns_ns_uses ns.Namespace_env.ns_ns_uses;
  check_map2 prev_ns.Namespace_env.ns_class_uses ns.Namespace_env.ns_class_uses;
  check_map2 prev_ns.Namespace_env.ns_fun_uses ns.Namespace_env.ns_fun_uses;
  check_map2 prev_ns.Namespace_env.ns_const_uses ns.Namespace_env.ns_const_uses

and emit_defs env defs =
  let rec aux env defs =
    match defs with
    | Ast.SetNamespaceEnv ns :: defs ->
      check_namespace_update env ns;
      let env = Emit_env.with_namespace ns env in
      aux env defs
    | [] -> Emit_statement.emit_dropthrough_return env
    (* emit last statement in the list as final statement *)
    | [Ast.Stmt s]
    (* emit statement as final if it is one before the last and last statement is
       empty markup (which will be no-op) *)
    | [Ast.Stmt s; Ast.Stmt (_, Ast.Markup ((_, ""), None))] ->
      Emit_statement.emit_final_statement env s
    | [d] ->
      gather [emit_def env d; Emit_statement.emit_dropthrough_return env]
    | d::defs ->
      let i1 = emit_def env d in
      let i2 = aux env defs in
      gather [i1; i2]
  in
  let rec emit_markup env defs =
    match defs with
    | Ast.Stmt (_, Ast.Markup ((_, s), echo_expr_opt))::defs ->
      let i1 =
        Emit_statement.emit_markup env s echo_expr_opt ~check_for_hashbang:true
      in
      let i2 = aux env defs in
      gather [i1; i2]
    | [_] | [] -> aux env defs
    | d :: defs ->
      let i1 = emit_def env d in
      if is_empty i1 then emit_markup env defs
      else gather [i1; aux env defs]
  in
  emit_markup env defs

let make_body body_instrs decl_vars
              is_memoize_wrapper is_memoize_wrapper_lsb
              params return_type_info doc_comment
              env =
  let body_instrs = rewrite_user_labels body_instrs in
  let body_instrs = rewrite_class_refs body_instrs in
  let params, body_instrs =
    if Hhbc_options.relabel !Hhbc_options.compiler_options
    then Label_rewriter.relabel_function params body_instrs
    else params, body_instrs in
  let num_iters =
    if is_memoize_wrapper then 0 else !Iterator.num_iterators in
  let num_cls_ref_slots = get_num_cls_ref_slots body_instrs in
  Hhas_body.make
    body_instrs
    decl_vars
    num_iters
    num_cls_ref_slots
    is_memoize_wrapper
    is_memoize_wrapper_lsb
    params
    return_type_info
    doc_comment
    env

let emit_return_type_info ~scope ~skipawaitable ~namespace ret =
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun t -> snd t.A.tp_name) in
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
    let error_code = if Emit_env.is_systemlib () then
      8192 (* E_DEPRECATED *) else 16384 (* E_USER_DEPRECATED *)
    in
    if Int64.to_int_exn sampling_rate <= 0 then empty else
    gather [
      trait_instrs;
      instr_string deprecation_string;
      concat_instruction;
      instr_int64 sampling_rate;
      instr_int error_code;
      instr_trigger_sampled_error;
      instr_popc;
    ]

let rec is_awaitable h =
  match h with
  | _, A.Happly ((_, "Awaitable"), ([] | [_])) -> true
  | _, (A.Hsoft h | A.Hoption h) -> is_awaitable h
  | _ -> false

let is_mixed_or_dynamic t =
 String_utils.string_ends_with t "HH\\mixed" ||
 String_utils.string_ends_with t "HH\\dynamic"

let emit_verify_out params =
  let is_verifiable p = match Hhas_param.type_info p with
    | None -> false
    | Some { Hhas_type_info.type_info_user_type = Some t; _ } ->
       not @@ is_mixed_or_dynamic t
    | _ -> true
  and emit_verify_out_type_for_refs =
    Hhbc_options.notice_on_byref_argument_typehint_violation
      !Hhbc_options.compiler_options
  in let param_instrs = List.filter_mapi params ~f:(fun i p ->
    if Hhas_param.is_inout p then
      Some (gather [
              instr_cgetl (Local.Named (Hhas_param.name p));
              if is_verifiable p then instr_verifyOutType (Param_unnamed i) else empty
            ])
    else if Hhas_param.is_reference p && is_verifiable p &&
              emit_verify_out_type_for_refs then
      Some (gather [
              instr_cgetl (Local.Named (Hhas_param.name p));
              instr_verifyOutType (Param_unnamed i);
              instr_popc
            ])
    else None
  ) in
  let param_instrs = List.rev param_instrs in
  let inout_cnt = List.count ~f:(fun p -> Hhas_param.is_inout p) params in
  (inout_cnt, gather param_instrs)

let modify_prog_for_debugger_eval instr_seq =
  let instr_list = Instruction_sequence.instr_seq_to_list instr_seq in
  let instr_length = List.length instr_list in
  if instr_length < 4 then instr_seq else
  let (h, t) = List.split_n instr_list (instr_length - 3) in
  match t with
    | [ Hhbc_ast.IBasic Hhbc_ast.PopC;
        Hhbc_ast.ILitConst (Hhbc_ast.Int 1L);
        Hhbc_ast.IContFlow Hhbc_ast.RetC ] -> gather [ instrs h; instr_retc ]
    | _ -> instr_seq

let emit_body
  ~pos
  ~scope
  ~is_closure_body
  ~is_memoize
  ~is_native
  ~is_async
  ~is_rx_body
  ~debugger_modify_program
  ~deprecation_info
  ~skipawaitable
  ~default_dropthrough
  ~return_value
  ~namespace
  ~doc_comment immediate_tparams ast_params ret body =
  if is_async && skipawaitable
  then begin
    let report_error =
      not (Option.value_map ret ~default:true ~f:is_awaitable) in
    if report_error
    then begin
      let message =
        if is_closure_body
        then "Return type hint for async closure must be awaitable"
        else
          let kind, name =
            match scope with
            | Ast_scope.ScopeItem.Function fd :: _ ->
              "function", Utils.strip_ns (snd fd.A.f_name)
            | Ast_scope.ScopeItem.Method md :: Ast_scope.ScopeItem.Class cd :: _ ->
              "method", Utils.strip_ns (snd cd.A.c_name) ^ "::" ^ (snd md.A.m_name)
            | _ -> assert false in
          Printf.sprintf "Return type hint for async %s %s() must be awaitable"
            kind name in
      Emit_fatal.raise_fatal_runtime pos message
      end;
  end;
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) (fun t -> snd t.A.tp_name) in
  Label.reset_label ();
  Iterator.reset_iterator ();
  let return_type_info =
    emit_return_type_info ~scope ~skipawaitable ~namespace ret in
  let return_type_info = if not is_native then return_type_info else
    Emit_type_hint.emit_type_constraint_for_native_function
      tparams ret return_type_info
  in
  let is_generator, is_pair_generator = Generator.is_function_generator body in
  let verify_return =
    if return_type_info.Hhas_type_info.type_info_user_type <> Some "" &&
       Hhas_type_info.has_type_constraint return_type_info && not is_generator
    then ret else None in
  let default_dropthrough =
    if default_dropthrough <> None then default_dropthrough else begin
      if is_async && Option.is_some verify_return
      then Some (gather [instr_null; instr_verifyRetTypeC; instr_retc])
      else None
    end
  in
  let params =
    Emit_param.from_asts
      ~namespace ~tparams ~generate_defaults:(not is_memoize) ~scope ast_params
  in
  let params = if is_closure_body
    then List.map ~f:Hhas_param.switch_inout_to_reference params else params in
  let num_out, verify_out = if is_closure_body then 0, empty else emit_verify_out params in
  Emit_statement.set_verify_return verify_return;
  Emit_statement.set_verify_out verify_out;
  Emit_statement.set_num_out num_out;
  Emit_statement.set_default_dropthrough default_dropthrough;
  Emit_statement.set_default_return_value return_value;
  Emit_statement.set_function_pos pos;
  Jump_targets.reset ();

  let remove_this vars =
    List.filter vars (fun s -> s <> "$this") in

  let move_this vars =
    if List.mem ~equal:(=) vars "$this"
    then remove_this vars @ ["$this"]
    else vars in

  let has_this = Ast_scope.Scope.has_this scope in
  let is_toplevel = Ast_scope.Scope.is_toplevel scope in
  (* see comment in decl_vars.ml, method on_efun of declvar_visitor
     why Decl_vars needs 'explicit_use_set' *)
  let explicit_use_set = Emit_env.get_explicit_use_set () in
  let is_in_static_method = Ast_scope.Scope.is_in_static_method scope in
  let needs_local_this, decl_vars =
    Decl_vars.from_ast
      ~is_closure_body
      ~has_this
      ~params
      ~is_toplevel
      ~is_in_static_method
      ~explicit_use_set
      body in
  let decl_vars =
    if is_closure_body
    then
      let ast_class =
        match scope with
        | _ :: _:: Ast_scope.ScopeItem.Class ast_class :: _ -> ast_class
        | _ -> failwith "impossible, closure scope should be lambda->method->class" in
      (* reorder decl_vars to match HHVM *)
      let captured_vars =
        ast_class.Ast.c_body
        |> List.concat_map ~f:(fun item ->
          match item with
          | Ast.ClassVars { Ast.cv_names = cvl; _ } ->
            List.map cvl ~f:(fun (_, (_, id), _) -> ("$" ^ id))
          | _ -> []) in
      "$0Closure" ::
      captured_vars @
      List.filter (move_this decl_vars) (fun v -> not (List.mem ~equal:(=) captured_vars v))
    else
      match scope with
      | _ :: Ast_scope.ScopeItem.Class _ :: _ -> move_this decl_vars
      | _ when Ast_scope.Scope.is_toplevel scope -> move_this decl_vars
      | _ -> decl_vars in
  let decl_vars =
    if List.exists ~f:(fun t -> t.A.tp_reified) immediate_tparams &&
       not is_closure_body
    then SU.Reified.reified_generics_local_name :: decl_vars else decl_vars in

  let function_state_key =
    let open Ast_scope in
    match scope with
    | [] -> Emit_env.get_unique_id_for_main ()
    | ScopeItem.Method md :: ScopeItem.Class cls :: _
    | ScopeItem.Lambda _ :: ScopeItem.Method md :: ScopeItem.Class cls :: _ ->
      Emit_env.get_unique_id_for_method cls md
    | ScopeItem.Function fd :: _ ->
      Emit_env.get_unique_id_for_function fd
    | _ -> failwith @@ "unexpected scope shape:" ^
      "expected either " ^
      "'ScopeItem.Method md :: ScopeItem.Class cls' for method or " ^
      "'ScopeItem.Function fd' for function or " ^
      "empty scope for top level" in

  let should_reserve_locals =
    SSet.mem function_state_key @@ Emit_env.get_functions_with_finally () in

  begin match SMap.get function_state_key @@ Emit_env.get_function_to_labels_map () with
  | Some s ->
    Jump_targets.set_function_has_goto true;
    Jump_targets.set_labels_in_function s;
  | None ->
    Jump_targets.set_function_has_goto false;
    Jump_targets.set_labels_in_function SMap.empty;
  end;

  Local.reset_local (List.length params + List.length decl_vars);
  if should_reserve_locals then Local.reserve_retval_and_label_id_locals ();

  let env = Emit_env.(
    empty |>
    with_namespace namespace |>
    with_needs_local_this needs_local_this |>
    with_scope scope |>
    with_rx_body is_rx_body) in
  let stmt_instrs = if is_native then instr_nativeimpl else
    Emit_env.do_function env body emit_defs in
  let begin_label, default_value_setters =
    Emit_param.emit_param_default_value_setter ~is_native env pos params in
  let generator_instr =
    if is_generator then gather [instr_createcont; instr_popc] else empty
  in
  let first_instruction_is_label =
    match Instruction_sequence.first stmt_instrs with
    | Some (ILabel _) -> true
    | _ -> false
  in
  let should_emit_init_this = not is_in_static_method && (needs_local_this ||
    (is_toplevel && List.exists ~f:(fun x -> x = "$this") decl_vars))
  in
  let header =
    (* per comment in emitter.cpp there should be no
     * jumps to the base of the function - inject EntryNop
     * if first instruction in the statement list is label
     * and header content is empty *)
    let header_content = gather [
      if is_native then empty else
        emit_method_prolog ~env ~pos ~params ~ast_params ~should_emit_init_this;
      emit_deprecation_warning scope deprecation_info;
      generator_instr;
    ] in
    if first_instruction_is_label &&
       Instruction_sequence.is_empty header_content
    then gather [
      begin_label;
      instr_entrynop;
    ]
    else gather [
      begin_label;
      header_content;
    ] in
  let body_instrs = gather [
    header;
    stmt_instrs;
    default_value_setters;
  ] in
  let body_instrs = if debugger_modify_program
    then modify_prog_for_debugger_eval body_instrs
    else body_instrs in
  make_body
    body_instrs
    decl_vars
    false (*is_memoize_wrapper*)
    false (*is_memoize_wrapper_lsb*)
    params
    (Some return_type_info)
    doc_comment
    (Some env),
    is_generator,
    is_pair_generator
