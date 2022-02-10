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
module Profile = Typing_toplevel_profile

let typedef_def ctx typedef =
  let tcopt = Provider_context.get_tcopt ctx in
  Profile.measure_elapsed_time_and_report tcopt None typedef.t_name @@ fun () ->
  let env = EnvFromDef.typedef_env ~origin:Decl_counters.TopLevel ctx typedef in
  let env =
    Env.set_module env
    @@ Naming_attributes_params.get_module_attribute typedef.t_file_attributes
  in
  let env =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      env
      ~ignore_errors:false
      typedef.t_tparams
      []
  in
  List.iter ~f:Errors.add_typing_error
  @@ Typing_type_wellformedness.typedef env typedef;
  Typing_variance.typedef env typedef;
  let {
    t_annotation = ();
    t_name = (t_pos, t_name);
    t_tparams = _;
    t_constraint = tcstr;
    t_kind = hint;
    t_user_attributes = _;
    t_vis = _;
    t_mode = _;
    t_namespace = _;
    t_span = _;
    t_emit_id = _;
    t_is_ctx = _;
    t_file_attributes = _;
  } =
    typedef
  in
  let (env, ty) =
    Phase.localize_hint_no_subst
      env
      ~ignore_errors:false
      ~report_cycle:(t_pos, t_name)
      hint
  in
  let env =
    match tcstr with
    | Some tcstr ->
      let (env, cstr) =
        Phase.localize_hint_no_subst env ~ignore_errors:false tcstr
      in
      Typing_ops.sub_type
        t_pos
        Reason.URnewtype_cstr
        env
        ty
        cstr
        Typing_error.Callback.newtype_alias_must_satisfy_constraint
    | _ -> env
  in
  let env =
    match hint with
    | (_pos, Hshape { nsi_allows_unknown_fields = _; nsi_field_map }) ->
      let get_name sfi = sfi.sfi_name in
      Typing_shapes.check_shape_keys_validity
        env
        (List.map ~f:get_name nsi_field_map)
    | _ -> env
  in
  let (env, user_attributes) =
    Typing.attributes_check_def
      env
      SN.AttributeKinds.typealias
      typedef.t_user_attributes
  in
  let (env, tparams) =
    List.map_env env typedef.t_tparams ~f:Typing.type_param
  in
  let (env, file_attributes) =
    Typing.file_attributes env typedef.t_file_attributes
  in
  {
    Aast.t_annotation = Env.save (Env.get_tpenv env) env;
    Aast.t_name = typedef.t_name;
    Aast.t_mode = typedef.t_mode;
    Aast.t_vis = typedef.t_vis;
    Aast.t_user_attributes = user_attributes;
    Aast.t_constraint = typedef.t_constraint;
    Aast.t_kind = typedef.t_kind;
    Aast.t_tparams = tparams;
    Aast.t_namespace = typedef.t_namespace;
    Aast.t_span = typedef.t_span;
    Aast.t_emit_id = typedef.t_emit_id;
    Aast.t_is_ctx = typedef.t_is_ctx;
    Aast.t_file_attributes = file_attributes;
  }
