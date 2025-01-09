(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Aast
open Tast
module Reason = Typing_reason
module Env = Typing_env
module SN = Naming_special_names
module Phase = Typing_phase
module EnvFromDef = Typing_env_from_def

type constraint_direction =
  | As
  | Super

module Constraints : sig
  (** [check env tcstr constraint_direction name ty] checks that
  `ty <: tcstr` or `tcstr <: ty` depending on [constraint_direction] *)
  val check :
    Typing_env_types.env ->
    hint option ->
    constraint_direction ->
    pos * string ->
    ty ->
    Typing_env_types.env * Typing_error.t option
end = struct
  let report_cyclic_constraint env cycles cstr_direction =
    match cstr_direction with
    | As ->
      Type_expansions.report cycles
      |> Option.iter ~f:(Typing_error_utils.add_typing_error ~env)
    | Super -> ()

  let check_subtype_constraint env ~ty ~cstr_ty t_pos cstr_direction =
    let (ty_sub, ty_super) =
      match cstr_direction with
      | As -> (ty, cstr_ty)
      | Super -> (cstr_ty, ty)
    in
    Typing_ops.sub_type
      t_pos
      Reason.URnewtype_cstr
      env
      ty_sub
      ty_super
      Typing_error.Callback.newtype_alias_must_satisfy_constraint

  let check env tcstr (cstr_direction : constraint_direction) (t_pos, t_name) ty
      : Typing_env_types.env * Typing_error.t option =
    match tcstr with
    | Some tcstr ->
      let ((env, ty_err_opt1, cycles), cstr_ty) =
        Phase.localize_hint_no_subst_report_cycles
          env
          ~ignore_errors:false
          tcstr
          ~report_cycle:(t_pos, Type_expansions.Expandable.Type_alias t_name)
      in
      report_cyclic_constraint env cycles cstr_direction;
      let (env, ty_err_opt2) =
        check_subtype_constraint env t_pos ~ty ~cstr_ty cstr_direction
      in
      (env, Option.merge ~f:Typing_error.both ty_err_opt1 ty_err_opt2)
    | _ -> (env, None)
end

(** Checks that variants of a case type are disjoint *)
let check_overlapping env (t_pos, t_name) hints =
  (* Given two types with their associated data types, check if
       the data types overlap. If they do report an error *)
  let check_overlapping data_type1 acc data_type2 =
    match
      Typing_case_types.check_overlapping
        ~pos:t_pos
        ~name:t_name
        env
        data_type1
        data_type2
    with
    | None -> acc
    | Some err -> Typing_error.casetype err :: acc
  in

  (* We check for overlaps pairwise between each variant type in the union *)
  let rec pairwise_check acc = function
    | data_type :: rest ->
      let acc = List.fold ~init:acc ~f:(check_overlapping data_type) rest in
      pairwise_check acc rest
    | [] -> acc
  in
  let (env, data_types) =
    hints |> List.fold_map ~init:env ~f:Typing_case_types.data_type_from_hint
  in
  let err = data_types |> pairwise_check [] |> Typing_error.multiple_opt in
  (env, err)

(** Forbid recursive type aliases like `A = A` and `A = ?A`
  (modulo indirection via other type aliases),
  something defined as contractiveness in Pierce's Types and Programming Languages.
  Note that we'd also want to forbid any recursion under union like `A = A | int`,
  but these are already forbidden by `check_overlapping`, so left to check
  are A = A and A = ?A. *)
let check_invalid_recursive_case_type
    (env : Typing_env_types.env) ((pos, name) : sid) (hints : _ list) =
  let rec is_invalid_single_type (ty : Typing_defs.locl_ty) =
    let open Typing_defs in
    match (get_node ty : locl_phase ty_) with
    | Tnewtype (id, _targs, _bound) -> String.equal name id
    | Toption ty -> is_invalid_single_type ty
    | Taccess _
    | Tgeneric _
    | Tprim _
    | Tfun _
    | Ttuple _
    | Tclass_ptr _
    | Tshape _
    | Tnonnull
    | Tvec_or_dict _
    | Tdynamic
    | Tvar _
    | Tclass _
    | Tunapplied_alias _
    | Tany _ ->
      false
    | Tunion tyl
    | Tintersection tyl ->
      List.exists tyl ~f:is_invalid_single_type
    | Tdependent _ ->
      failwith "Unexpected type Tdependent out of localizing a hint"
    | Tneg _ -> failwith "Unexpected type Tneg out of localizing a hint"
    | Tlabel _ -> failwith "Unexpected type Tlabel out of localizing a hint"
  in
  match hints with
  | [hint] ->
    let ty = Decl_hint.hint env.Typing_env_types.decl_env hint in
    let ((env, _err), ty) =
      Typing_phase.localize
        env
        ~ety_env:
          Typing_defs.
            {
              empty_expand_env with
              visibility_behavior = Always_expand_newtype;
              type_expansions =
                Type_expansions.empty_w_cycle_report
                  ~report_cycle:
                    (Some (pos, Type_expansions.Expandable.Type_alias name));
            }
        ty
    in
    let err =
      if is_invalid_single_type ty then
        Some
          (Typing_error.casetype
             (Typing_error.Primary.CaseType.Invalid_recursive { pos; name }))
      else
        None
    in
    (env, err)
  | _ ->
    (* Recursive types like `A = A | int` are already forbidden
       by `check_overlapping`, so we skip the check. *)
    (env, None)

(** Checks that variants of a case type are disjoint, and rule out other invalid recursive case types *)
let casetype_def env typedef =
  let {
    t_annotation = ();
    t_name;
    t_tparams = _;
    t_as_constraint = _;
    t_super_constraint = _;
    t_assignment;
    t_runtime_type = _;
    t_user_attributes = _;
    t_mode = _;
    t_namespace = _;
    t_span = _;
    t_emit_id = _;
    t_is_ctx = _;
    t_file_attributes = _;
    t_internal = _;
    t_module = _;
    t_docs_url = _;
    t_doc_comment = _;
    t_package = _;
  } =
    typedef
  in
  match t_assignment with
  | SimpleTypeDef _ -> env
  | CaseType (variant, variants) ->
    let hints = List.map (variant :: variants) ~f:(fun v -> v.tctv_hint) in
    let (env, err1) = check_overlapping env t_name hints in
    let (env, err2) = check_invalid_recursive_case_type env t_name hints in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) err1;
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) err2;
    env

let check_cycles env (t_pos, t_name) hints =
  let (env, cycles) =
    List.fold_left hints ~init:(env, []) ~f:(fun (env, cycles) hint ->
        let ((env, _ty_err_opt, new_cycles), _ty) =
          Phase.localize_hint_no_subst_report_cycles
            env
            ~ignore_errors:true
            ~report_cycle:(t_pos, Type_expansions.Expandable.Type_alias t_name)
            hint
        in
        (env, new_cycles @ cycles))
  in
  Type_expansions.report cycles
  |> Option.iter ~f:(Typing_error_utils.add_typing_error ~env);
  env

let typedef_def ctx typedef =
  let env = EnvFromDef.typedef_env ~origin:Decl_counters.TopLevel ctx typedef in
  let {
    t_annotation = ();
    t_name;
    t_tparams;
    t_as_constraint;
    t_super_constraint;
    t_assignment;
    t_runtime_type;
    t_user_attributes;
    t_mode;
    t_namespace;
    t_span;
    t_emit_id;
    t_is_ctx;
    t_file_attributes;
    t_internal;
    t_module;
    t_docs_url;
    t_doc_comment;
    t_package;
  } =
    typedef
  in
  let (do_report_cycles, hints) =
    match t_assignment with
    | SimpleTypeDef { tvh_vis = _; tvh_hint } -> ((fun _env -> true), [tvh_hint])
    (* TODO T201569125 - do I need to do something with the where constraints here? *)
    | CaseType (variant, variants) ->
      ( (fun env ->
          let recursive_case_types env =
            (Env.get_tcopt env).GlobalOptions.recursive_case_types
          in
          not (recursive_case_types env)),
        List.map (variant :: variants) ~f:(fun v -> v.tctv_hint) )
  in
  let env = Env.set_current_module env t_module in
  let env = Env.set_internal env t_internal in
  let env = Env.set_current_package_membership env t_package in
  let (env, ty_err_opt1) =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      env
      ~ignore_errors:false
      t_tparams
      []
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
  Typing_type_wellformedness.typedef env typedef
  |> List.iter ~f:(Typing_error_utils.add_typing_error ~env);
  Env.make_depend_on_current_module env;
  Typing_variance.typedef env typedef;

  let env =
    if do_report_cycles env then
      check_cycles env t_name hints
    else
      env
  in
  let env =
    let (env, ty_err_opt2, tys) =
      List.fold_left
        hints
        ~init:(env, None, [])
        ~f:(fun (env, ty_err_opt2, tys) hint ->
          let ((env, new_ty_err_opt2), ty) =
            Phase.localize_hint_no_subst env ~ignore_errors:false hint
          in
          ( env,
            Option.merge ~f:Typing_error.both new_ty_err_opt2 ty_err_opt2,
            ty :: tys ))
    in
    let env = casetype_def env typedef in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt2;

    let (env, ty_err_opt3) =
      List.fold_left_env env tys ~init:None ~f:(fun env acc_err ty ->
          let (env, err) = Constraints.check env t_as_constraint As t_name ty in
          (env, Option.merge ~f:Typing_error.both acc_err err))
    in
    let (env, ty_err_opt4) =
      List.fold_left_env env tys ~init:None ~f:(fun env acc_err ty ->
          let (env, err) =
            Constraints.check env t_super_constraint Super t_name ty
          in
          (env, Option.merge ~f:Typing_error.both acc_err err))
    in
    Option.iter
      ~f:(Typing_error_utils.add_typing_error ~env)
      (Option.merge ~f:Typing_error.both ty_err_opt3 ty_err_opt4);
    env
  in

  let env =
    List.fold_left hints ~init:env ~f:(fun env hint ->
        match hint with
        | (_pos, Hshape { nsi_allows_unknown_fields = _; nsi_field_map }) ->
          let get_name sfi = sfi.sfi_name in
          Typing_shapes.check_shape_keys_validity
            env
            (List.map ~f:get_name nsi_field_map)
        | _ -> env)
  in
  let (env, user_attributes) =
    Typing.attributes_check_def
      env
      SN.AttributeKinds.typealias
      t_user_attributes
  in
  let (env, tparams) = List.map_env env t_tparams ~f:Typing.type_param in
  let (env, file_attributes) = Typing.file_attributes env t_file_attributes in
  {
    Aast.t_annotation = Env.save (Env.get_tpenv env) env;
    Aast.t_name;
    Aast.t_mode;
    Aast.t_user_attributes = user_attributes;
    Aast.t_as_constraint;
    Aast.t_super_constraint;
    Aast.t_assignment;
    Aast.t_runtime_type;
    Aast.t_tparams = tparams;
    Aast.t_namespace;
    Aast.t_span;
    Aast.t_emit_id;
    Aast.t_is_ctx;
    Aast.t_file_attributes = file_attributes;
    Aast.t_internal;
    Aast.t_module;
    Aast.t_docs_url;
    Aast.t_doc_comment;
    Aast.t_package;
  }
