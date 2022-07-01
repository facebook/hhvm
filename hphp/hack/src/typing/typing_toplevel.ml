(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This module implements the typing.
 *
 * Given an Nast.program, it infers the type of all the local
 * variables, and checks that all the types are correct (aka
 * consistent) *)
open Hh_prelude
open Common
open Aast
open Typing_defs
open Typing_env_types
open Typing_helpers
module Reason = Typing_reason
module Env = Typing_env
module MakeType = Typing_make_type
module Phase = Typing_phase
module EnvFromDef = Typing_env_from_def
module TCO = TypecheckerOptions
module SN = Naming_special_names
module Profile = Typing_toplevel_profile

(* The two following functions enable us to retrieve the function (or class)
  header from the shared mem. Note that they only return a non None value if
  global inference is on *)
let get_decl_function_header env function_id =
  let is_global_inference_on = TCO.global_inference (Env.get_tcopt env) in
  if is_global_inference_on then
    match Decl_provider.get_fun (Env.get_ctx env) function_id with
    | Some { fe_type; _ } ->
      begin
        match get_node fe_type with
        | Tfun fun_type -> Some fun_type
        | _ -> None
      end
    | _ -> None
  else
    None

let is_literal_with_trivially_inferable_type (_, _, e) =
  Option.is_some @@ Decl_utils.infer_const e

let fun_def ctx fd :
    (Tast.fun_def * Typing_inference_env.t_global_with_pos) option =
  let f = fd.fd_fun in
  let tcopt = Provider_context.get_tcopt ctx in
  Profile.measure_elapsed_time_and_report tcopt None f.f_name @@ fun () ->
  Counters.count Counters.Category.Typecheck @@ fun () ->
  Errors.run_with_span f.f_span @@ fun () ->
  let env = EnvFromDef.fun_env ~origin:Decl_counters.TopLevel ctx fd in
  with_timeout env f.f_name @@ fun env ->
  (* reset the expression dependent display ids for each function body *)
  Reason.expr_display_id_map := IMap.empty;
  let pos = fst f.f_name in
  let decl_header = get_decl_function_header env (snd f.f_name) in
  let env = Env.open_tyvars env (fst f.f_name) in
  let env = Env.set_env_callable_pos env pos in
  let (env, user_attributes) =
    Typing.attributes_check_def env SN.AttributeKinds.fn f.f_user_attributes
  in
  let (env, file_attrs) = Typing.file_attributes env fd.fd_file_attributes in
  let (env, cap_ty, unsafe_cap_ty) =
    Typing_coeffects.type_capability env f.f_ctxs f.f_unsafe_ctxs (fst f.f_name)
  in
  let env = Env.set_module env fd.fd_module in
  let env = Env.set_internal env fd.fd_internal in
  let env =
    if
      Naming_attributes.mem
        SN.UserAttributes.uaSupportDynamicType
        f.f_user_attributes
    then
      Env.set_support_dynamic_type env true
    else
      env
  in
  List.iter ~f:Errors.add_typing_error @@ Typing_type_wellformedness.fun_ env f;
  Typing_env.make_depend_on_module env;
  let (env, ty_err_opt) =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      env
      ~ignore_errors:false
      f.f_tparams
      f.f_where_constraints
  in
  Option.iter ~f:Errors.add_typing_error ty_err_opt;
  let env = Env.set_fn_kind env f.f_fun_kind in
  let (return_decl_ty, params_decl_ty) =
    merge_decl_header_with_hints ~params:f.f_params ~ret:f.f_ret decl_header env
  in
  let hint_pos =
    match f.f_ret with
    | (_, None) -> fst f.f_name
    | (_, Some (pos, _)) -> pos
  in
  let ety_env =
    empty_expand_env_with_on_error
      (Typing_error.Reasons_callback.invalid_type_hint hint_pos)
  in
  let (env, return_ty) =
    Typing_return.make_return_type
      ~ety_env
      env
      ~hint_pos
      ~explicit:return_decl_ty
      ~default:None
  in
  let return =
    Typing_return.make_info
      hint_pos
      f.f_fun_kind
      f.f_user_attributes
      env
      return_ty
  in
  let (env, _) =
    Typing_coeffects.register_capabilities env cap_ty unsafe_cap_ty
  in
  let sound_dynamic_check_saved_env = env in
  let (env, param_tys) =
    Typing_param.make_param_local_tys
      ~dynamic_mode:false
      env
      params_decl_ty
      f.f_params
  in
  let params_need_immutable = Typing_coeffects.get_ctx_vars f.f_ctxs in
  let can_read_globals =
    Typing_subtype.is_sub_type
      env
      cap_ty
      (MakeType.capability (get_reason cap_ty) SN.Capabilities.accessGlobals)
  in
  let (env, typed_params) =
    let bind_param_and_check env param =
      let name = (snd param).param_name in
      let immutable =
        List.exists ~f:(String.equal name) params_need_immutable
      in
      let (env, fun_param) =
        Typing.bind_param ~immutable ~can_read_globals env param
      in
      (env, fun_param)
    in
    List.map_env env (List.zip_exn param_tys f.f_params) ~f:bind_param_and_check
  in
  let env = set_tyvars_variance_in_callable env return_ty.et_type param_tys in
  let local_tpenv = Env.get_tpenv env in
  let disable =
    Naming_attributes.mem
      SN.UserAttributes.uaDisableTypecheckerInternal
      f.f_user_attributes
  in
  Typing_memoize.check_function env f;
  let (env, tb) =
    Typing.fun_
      ~native:(Typing_native.is_native_fun ~env f)
      ~disable
      env
      return
      pos
      f.f_body
      f.f_fun_kind
  in
  begin
    match hint_of_type_hint f.f_ret with
    | None ->
      if not @@ Env.is_hhi env then
        Errors.add_typing_error
          Typing_error.(primary @@ Primary.Expecting_return_type_hint pos)
    | Some _ -> ()
  end;
  let (env, tparams) = List.map_env env f.f_tparams ~f:Typing.type_param in
  let (env, e1) = Typing_solver.close_tyvars_and_solve env in
  let (env, e2) = Typing_solver.solve_all_unsolved_tyvars env in

  if
    TypecheckerOptions.enable_sound_dynamic
      (Provider_context.get_tcopt (Env.get_ctx env))
    && Env.get_support_dynamic_type env
  then
    Typing.function_dynamically_callable
      sound_dynamic_check_saved_env
      f
      params_decl_ty
      return_ty.et_type;

  let fun_ =
    {
      Aast.f_annotation = Env.save local_tpenv env;
      Aast.f_readonly_this = f.f_readonly_this;
      Aast.f_span = f.f_span;
      Aast.f_readonly_ret = f.f_readonly_ret;
      Aast.f_ret = (return_ty.et_type, hint_of_type_hint f.f_ret);
      Aast.f_name = f.f_name;
      Aast.f_tparams = tparams;
      Aast.f_where_constraints = f.f_where_constraints;
      Aast.f_params = typed_params;
      Aast.f_ctxs = f.f_ctxs;
      Aast.f_unsafe_ctxs = f.f_unsafe_ctxs;
      Aast.f_fun_kind = f.f_fun_kind;
      Aast.f_user_attributes = user_attributes;
      Aast.f_body = { Aast.fb_ast = tb };
      Aast.f_external = f.f_external;
      Aast.f_doc_comment = f.f_doc_comment;
    }
  in
  let fundef =
    {
      Aast.fd_mode = fd.fd_mode;
      Aast.fd_fun = fun_;
      Aast.fd_file_attributes = file_attrs;
      Aast.fd_namespace = fd.fd_namespace;
      Aast.fd_internal = fd.fd_internal;
      Aast.fd_module = fd.fd_module;
    }
  in
  let (_env, global_inference_env) = Env.extract_global_inference_env env in
  let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
  Option.iter ~f:Errors.add_typing_error ty_err_opt;
  (fundef, (pos, global_inference_env))

let class_def = Typing_class.class_def

let gconst_def ctx cst =
  let tcopt = Provider_context.get_tcopt ctx in
  Profile.measure_elapsed_time_and_report tcopt None cst.cst_name @@ fun () ->
  Counters.count Counters.Category.Typecheck @@ fun () ->
  Errors.run_with_span cst.cst_span @@ fun () ->
  let env = EnvFromDef.gconst_env ~origin:Decl_counters.TopLevel ctx cst in
  List.iter ~f:Errors.add_typing_error
  @@ Typing_type_wellformedness.global_constant env cst;
  let (typed_cst_value, (env, ty_err_opt)) =
    let value = cst.cst_value in
    match cst.cst_type with
    | Some hint ->
      let ty = Decl_hint.hint env.decl_env hint in
      let ty = Typing_enforceability.compute_enforced_ty env ty in
      let ((env, ty_err_opt1), dty) =
        Phase.localize_possibly_enforced_no_subst env ~ignore_errors:false ty
      in
      let (env, te, value_type) =
        let expected =
          ExpectedTy.make_and_allow_coercion (fst hint) Reason.URhint dty
        in
        Typing.expr_with_pure_coeffects env ~expected value
      in
      let (env, ty_err_opt2) =
        Typing_coercion.coerce_type
          (fst hint)
          Reason.URhint
          env
          value_type
          dty
          Typing_error.Callback.unify_error
      in
      let ty_err_opt =
        Option.merge ~f:Typing_error.both ty_err_opt1 ty_err_opt2
      in
      (te, (env, ty_err_opt))
    | None ->
      if
        (not (is_literal_with_trivially_inferable_type value))
        && not (Env.is_hhi env)
      then
        Errors.add_naming_error
        @@ Naming_error.Missing_typehint (fst cst.cst_name);
      let (env, te, _value_type) = Typing.expr_with_pure_coeffects env value in
      (te, (env, None))
  in
  Option.iter ty_err_opt ~f:Errors.add_typing_error;
  {
    Aast.cst_annotation = Env.save (Env.get_tpenv env) env;
    Aast.cst_mode = cst.cst_mode;
    Aast.cst_name = cst.cst_name;
    Aast.cst_type = cst.cst_type;
    Aast.cst_value = typed_cst_value;
    Aast.cst_namespace = cst.cst_namespace;
    Aast.cst_span = cst.cst_span;
    Aast.cst_emit_id = cst.cst_emit_id;
  }

let module_def ctx md =
  Counters.count Counters.Category.Typecheck @@ fun () ->
  Errors.run_with_span md.md_span @@ fun () ->
  let env = EnvFromDef.module_env ~origin:Decl_counters.TopLevel ctx md in
  let (env, user_attributes) =
    Typing.attributes_check_def
      env
      SN.AttributeKinds.module_
      md.md_user_attributes
  in
  {
    md with
    Aast.md_annotation = Env.save (Env.get_tpenv env) env;
    Aast.md_user_attributes = user_attributes;
  }

let nast_to_tast_gienv ~(do_tast_checks : bool) ctx nast :
    _ * Typing_inference_env.t_global_with_pos list =
  let convert_def = function
    (* Sometimes typing will just return `None` but that should only be the case
     * if an error had already been registered e.g. in naming
     *)
    | Fun f ->
      begin
        match fun_def ctx f with
        | Some (f, env) -> Some (Aast.Fun f, [env])
        | None -> None
      end
    | Constant gc -> Some (Aast.Constant (gconst_def ctx gc), [])
    | Typedef td -> Some (Aast.Typedef (Typing_typedef.typedef_def ctx td), [])
    | Class c ->
      begin
        match class_def ctx c with
        | Some (c, envs) -> Some (Aast.Class c, envs)
        | None -> None
      end
    (* We don't typecheck top level statements:
     * https://docs.hhvm.com/hack/unsupported/top-level
     * so just create the minimal env for us to construct a Stmt.
     *)
    | Stmt s ->
      let env = Typing_env_types.empty ctx Relative_path.default ~droot:None in
      Some (Aast.Stmt (snd (Typing.stmt env s)), [])
    | Module md -> Some (Aast.Module (module_def ctx md), [])
    | SetModule sm -> Some (Aast.SetModule sm, [])
    | Namespace _
    | NamespaceUse _
    | SetNamespaceEnv _
    | FileAttributes _ ->
      failwith
        "Invalid nodes in NAST. These nodes should be removed during naming."
  in
  if do_tast_checks then Nast_check.program ctx nast;
  let (tast, envs) = List.unzip @@ List.filter_map nast ~f:convert_def in
  let envs = List.concat envs in
  if do_tast_checks then Tast_check.program ctx tast;
  (tast, envs)

let nast_to_tast ~do_tast_checks ctx nast =
  let (tast, _gienvs) = nast_to_tast_gienv ~do_tast_checks ctx nast in
  tast
