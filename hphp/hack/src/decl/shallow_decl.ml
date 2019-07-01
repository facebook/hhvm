(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Decl_defs
open Decl_fun_utils
open Shallow_decl_defs
open Nast
open Typing_deps
open Typing_defs

module Attrs = Attributes
module Partial = Partial_provider

let class_const env c (h, name, e) =
  let pos = fst name in
  match c.c_kind with
  | Ast_defs.Ctrait ->
      let kind = match c.c_kind with
        | Ast_defs.Ctrait -> `trait
        | Ast_defs.Cenum -> `enum
        | Ast_defs.Crecord -> `record
        | _ -> assert false in
      Errors.cannot_declare_constant kind pos c.c_name;
      None
  | Ast_defs.Cnormal | Ast_defs.Cabstract | Ast_defs.Cinterface | Ast_defs.Cenum | Ast_defs.Crecord ->
    let ty, abstract =
      (* Optional hint h, optional expression e *)
      match h, e with
      | Some h, Some _ ->
        Decl_hint.hint env h, false
      | Some h, None ->
        Decl_hint.hint env h, true
      | None, Some e ->
          begin match Decl_utils.infer_const e with
            | Some ty -> ty, false
            | None ->
              if Partial.should_check_error c.c_mode 2035 && c.c_kind <> Ast_defs.Cenum
              then Errors.missing_typehint pos;
              (Reason.Rwitness pos, Tany), false
          end
        | None, None ->
          if Partial.should_check_error c.c_mode 2035 then Errors.missing_typehint pos;
          let r = Reason.Rwitness pos in
          (r, Tany), true
    in
    Some {
      scc_abstract = abstract;
      scc_expr = e;
      scc_name = name;
      scc_type = ty;
    }

let typeconst_abstract_kind env = function
  | Nast.TCAbstract default -> TCAbstract (Option.map default (Decl_hint.hint env))
  | Nast.TCPartiallyAbstract -> TCPartiallyAbstract
  | Nast.TCConcrete -> TCConcrete

let typeconst env c tc =
  match c.c_kind with
  | Ast_defs.Ctrait | Ast_defs.Cenum | Ast_defs.Crecord->
      let kind = match c.c_kind with
        | Ast_defs.Ctrait -> `trait
        | Ast_defs.Cenum -> `enum
        | Ast_defs.Crecord -> `record
        | _ -> assert false in
      Errors.cannot_declare_constant kind (fst tc.c_tconst_name) c.c_name;
      None
  | Ast_defs.Cinterface | Ast_defs.Cabstract | Ast_defs.Cnormal ->
      let constr = Option.map tc.c_tconst_constraint (Decl_hint.hint env) in
      let ty = Option.map tc.c_tconst_type (Decl_hint.hint env) in
      let enforceable =
        match Attrs.find SN.UserAttributes.uaEnforceable tc.c_tconst_user_attributes with
        | Some { ua_name = (pos, _); _ } -> pos, true
        | None -> Pos.none, false in
      Some {
        stc_abstract = typeconst_abstract_kind env tc.c_tconst_abstract;
        stc_name = tc.c_tconst_name;
        stc_constraint = constr;
        stc_type = ty;
        stc_enforceable = enforceable;
      }
let make_xhp_attr cv = Option.map cv.cv_xhp_attr (fun xai -> {
  xa_tag = (match xai.xai_tag with
    | None -> None
    | Some Required -> Some Required
    | Some LateInit -> Some Lateinit
  );
  xa_has_default = Option.is_some cv.cv_expr;
})

let prop env cv =
  let cv_pos = fst cv.cv_id in
  let ty = Option.map cv.cv_type ~f:begin fun ty' ->
    if Option.is_some cv.cv_xhp_attr
    then
      (* If this is an XHP attribute and we're in strict mode,
         relax to partial mode to allow the use of the "array"
         annotation without specifying type parameters. Until
         recently HHVM did not allow "array" with type parameters
         in XHP attribute declarations, so this is a temporary
         hack to support existing code for now. *)
      (* Task #5815945: Get rid of this Hack *)
      let env =
        if FileInfo.is_strict (Decl_env.mode env)
        then { env with Decl_env.mode = FileInfo.Mpartial }
        else env
      in
      Decl_hint.hint env ty'
    else Decl_hint.hint env ty'
  end in
  let const = Attrs.mem SN.UserAttributes.uaConst cv.cv_user_attributes in
  let lateinit = Attrs.mem2
    SN.UserAttributes.uaLateInit SN.UserAttributes.uaSoftLateInit
    cv.cv_user_attributes in
  if cv.cv_final then Errors.final_property cv_pos;
  if lateinit && cv.cv_expr <> None then Errors.lateinit_with_default cv_pos;
  {
    sp_const = const;
    sp_xhp_attr = make_xhp_attr cv;
    sp_lateinit = lateinit;
    sp_lsb = false;
    sp_name = cv.cv_id;
    sp_needs_init = Option.is_none cv.cv_expr;
    sp_type = ty;
    sp_visibility = cv.cv_visibility;
    sp_fixme_codes = Fixme_provider.get_fixme_codes_for_pos cv_pos;
  }

and static_prop env c cv =
  let cv_pos, cv_name = cv.cv_id in
  let ty = Option.map cv.cv_type ~f:(Decl_hint.hint env) in
  let id = "$" ^ cv_name in
  let lateinit = Attrs.mem2
    SN.UserAttributes.uaLateInit
    SN.UserAttributes.uaSoftLateInit
    cv.cv_user_attributes in
  let lsb = Attrs.mem SN.UserAttributes.uaLSB cv.cv_user_attributes in
  if cv.cv_expr = None && FileInfo.(is_strict c.c_mode || c.c_mode = Mpartial)
  then begin match cv.cv_type with
    | None
    | Some (_, Hmixed)
    | Some (_, Hoption _) -> ()
    | _ when not lateinit -> Errors.missing_assign cv_pos
    | _ -> ()
  end;
  if lateinit && cv.cv_expr <> None then Errors.lateinit_with_default cv_pos;
  {
    sp_const = false; (* unsupported for static properties *)
    sp_xhp_attr = make_xhp_attr cv;
    sp_lateinit = lateinit;
    sp_lsb = lsb;
    sp_name = cv_pos, id;
    sp_needs_init = Option.is_none cv.cv_expr;
    sp_type = ty;
    sp_visibility = cv.cv_visibility;
    sp_fixme_codes = Fixme_provider.get_fixme_codes_for_pos cv_pos;
  }

let method_type env m =
  check_params env m.m_params;
  let reactivity = fun_reactivity env m.m_user_attributes in
  let mut = get_param_mutability m.m_user_attributes in
  let returns_mutable = fun_returns_mutable m.m_user_attributes in
  let returns_void_to_rx = fun_returns_void_to_rx m.m_user_attributes in
  let return_disposable = has_return_disposable_attribute m.m_user_attributes in
  let arity_min = minimum_arity m.m_params in
  let params = make_params env m.m_params in
  let ret = match m.m_ret with
    | None -> ret_from_fun_kind (fst m.m_name) m.m_fun_kind
    | Some ret -> Decl_hint.hint env ret in
  let arity = match m.m_variadic with
    | FVvariadicArg param ->
      assert param.param_is_variadic;
      assert (param.param_expr = None);
      Fvariadic (arity_min, make_param_ty env param)
    | FVellipsis p  -> Fellipsis (arity_min, p)
    | FVnonVariadic -> Fstandard (arity_min, List.length m.m_params)
  in
  let tparams = List.map m.m_tparams (type_param env) in
  let where_constraints =
    List.map m.m_where_constraints (where_constraint env) in
  {
    ft_pos      = fst m.m_name;
    ft_deprecated =
      Attrs.deprecated ~kind:"method" m.m_name m.m_user_attributes;
    ft_abstract = m.m_abstract;
    ft_is_coroutine = m.m_fun_kind = Ast_defs.FCoroutine;
    ft_arity    = arity;
    ft_tparams  = (tparams, FTKtparams);
    ft_where_constraints = where_constraints;
    ft_params   = params;
    ft_ret      = ret;
    ft_reactive = reactivity;
    ft_mutability = mut;
    ft_returns_mutable = returns_mutable;
    ft_return_disposable = return_disposable;
    ft_fun_kind = m.m_fun_kind;
    ft_decl_errors = None;
    ft_returns_void_to_rx = returns_void_to_rx;
  }

let method_redeclaration_type env m =
  check_params env m.mt_params;
  let arity_min = minimum_arity m.mt_params in
  let params = make_params env m.mt_params in
  let ret = match m.mt_ret with
    | None -> ret_from_fun_kind (fst m.mt_name) m.mt_fun_kind
    | Some ret -> Decl_hint.hint env ret in
  let arity = match m.mt_variadic with
    | FVvariadicArg param ->
      assert param.param_is_variadic;
      assert (param.param_expr = None);
      Fvariadic (arity_min, make_param_ty env param)
    | FVellipsis p  -> Fellipsis (arity_min, p)
    | FVnonVariadic -> Fstandard (arity_min, List.length m.mt_params)
  in
  let tparams = List.map m.mt_tparams (type_param env) in
  let where_constraints =
    List.map m.mt_where_constraints (where_constraint env) in
  {
    ft_pos      = fst m.mt_name;
    ft_deprecated = None;
    ft_abstract = m.mt_abstract;
    ft_is_coroutine = m.mt_fun_kind = Ast_defs.FCoroutine;
    ft_arity    = arity;
    ft_tparams  = (tparams, FTKtparams);
    ft_where_constraints = where_constraints;
    ft_params   = params;
    ft_ret      = ret;
    ft_reactive = Nonreactive;
    ft_mutability = None;
    ft_returns_mutable = false;
    ft_fun_kind = m.mt_fun_kind;
    ft_return_disposable = false;
    ft_decl_errors = None;
    ft_returns_void_to_rx = false;
  }

let method_ env c m =
  let override = Attrs.mem SN.UserAttributes.uaOverride m.m_user_attributes in
  if m.m_visibility = Private && override then begin
    let pos, id = m.m_name in
    Errors.private_override pos (snd c.c_name) id;
  end;
  let has_memoizelsb =
    Attrs.mem SN.UserAttributes.uaMemoizeLSB m.m_user_attributes in
  let ft = method_type env m in
  let reactivity =
    match ft.ft_reactive with
    | Reactive (Some (_, Tapply ((_, cls), []))) ->
      Some (Method_reactive (Some cls))
    | Reactive None ->
      Some (Method_reactive None)
    | Shallow (Some (_, Tapply ((_, cls), []))) ->
      Some (Method_shallow (Some cls))
    | Shallow None ->
      Some (Method_shallow None)
    | Local (Some (_, Tapply ((_, cls), [])))  ->
      Some (Method_local (Some cls))
    | Local None ->
      Some (Method_local None)
    | _ -> None
  in
  {
    sm_abstract = ft.ft_abstract;
    sm_final = m.m_final;
    sm_memoizelsb = has_memoizelsb;
    sm_name = m.m_name;
    sm_override = override;
    sm_reactivity = reactivity;
    sm_type = ft;
    sm_visibility = m.m_visibility;
    sm_fixme_codes = Fixme_provider.get_fixme_codes_for_pos ft.ft_pos;
  }

let method_redeclaration env m =
  let ft = method_redeclaration_type env m in
  {
    smr_abstract = ft.ft_abstract;
    smr_final = m.mt_final;
    smr_static = m.mt_static;
    smr_name = m.mt_name;
    smr_type = ft;
    smr_visibility = m.mt_visibility;
    smr_trait = m.mt_trait;
    smr_method = m.mt_method;
    smr_fixme_codes = Fixme_provider.get_fixme_codes_for_pos ft.ft_pos;
  }

let enum_type hint e =
  {
    te_base = hint e.e_base;
    te_constraint = Option.map e.e_constraint hint;
  }

let class_ env c =
  let hint = Decl_hint.hint env in
  let req_extends, req_implements = split_reqs c in
  let static_vars, vars = split_vars c in
  let constructor, statics, rest = split_methods c in
  let sc_extends = List.map ~f:hint c.c_extends in
  let sc_uses = List.map ~f:hint c.c_uses in
  let sc_req_extends = List.map ~f:hint req_extends in
  let sc_req_implements = List.map ~f:hint req_implements in
  let sc_implements = List.map ~f:hint c.c_implements in
  let additional_parents =
    (* In an abstract class or a trait, we assume the interfaces
       will be implemented in the future, so we take them as
       part of the class (as requested by dependency injection implementers) *)
    match c.c_kind with
    | Ast_defs.Cabstract -> sc_implements
    | Ast_defs.Ctrait -> sc_implements @ sc_req_implements
    | _ -> []
  in
  let add_cstr_dep ty =
    let _, (_, class_name), _ = Decl_utils.unwrap_class_type ty in
    Decl_env.add_constructor_dependency env class_name
  in
  List.iter ~f:add_cstr_dep sc_extends;
  List.iter ~f:add_cstr_dep sc_uses;
  List.iter ~f:add_cstr_dep sc_req_extends;
  List.iter ~f:add_cstr_dep additional_parents;
  {
    sc_mode = c.c_mode;
    sc_final = c.c_final;
    sc_is_xhp = c.c_is_xhp;
    sc_kind = c.c_kind;
    sc_name = c.c_name;
    sc_tparams = List.map c.c_tparams.c_tparam_list (type_param env);
    sc_extends;
    sc_uses;
    sc_method_redeclarations =
      List.map c.c_method_redeclarations (method_redeclaration env);
    sc_xhp_attr_uses  = List.map ~f:hint c.c_xhp_attr_uses;
    sc_req_extends;
    sc_req_implements;
    sc_implements;
    sc_consts = List.filter_map c.c_consts (class_const env c);
    sc_typeconsts = List.filter_map c.c_typeconsts (typeconst env c);
    sc_props = List.map ~f:(prop env) vars;
    sc_sprops = List.map ~f:(static_prop env c) static_vars;
    sc_constructor = Option.map ~f:(method_ env c) constructor;
    sc_static_methods = List.map ~f:(method_ env c) statics;
    sc_methods = List.map ~f:(method_ env c) rest;
    sc_user_attributes = c.c_user_attributes;
    sc_enum_type = Option.map c.c_enum (enum_type hint);
    sc_decl_errors = Errors.empty;
  }

let class_ c =
  let cls_pos, cls_name = c.c_name in
  let class_dep = Dep.Class cls_name in
  let env = {
    Decl_env.mode = c.c_mode;
    droot = Some class_dep;
    decl_tcopt = GlobalNamingOptions.get ();
  } in
  let errors, sc =
    Errors.run_in_context (Pos.filename cls_pos) Errors.Decl
      (fun () -> Errors.do_ (fun () -> class_ env c)) in
  { sc with sc_decl_errors = errors }
