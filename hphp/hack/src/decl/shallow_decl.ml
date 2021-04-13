(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shallow_decl_defs
open Aast
open Typing_deps
open Typing_defs
module Attrs = Naming_attributes
module FunUtils = Decl_fun_utils
module SN = Naming_special_names

(* gather class constants used in a constant initializer *)
let gather_constants =
  object (_ : 'self)
    inherit [_] Aast.reduce as super

    method zero = CCRSet.empty

    method plus s1 s2 = CCRSet.union s1 s2

    method! on_expr acc expr =
      let refs = super#on_expr acc expr in
      let refs =
        match snd expr with
        | Class_const (class_id, (_, name)) ->
          begin
            match snd class_id with
            | CI from -> CCRSet.add (From (snd from), name) refs
            | CIself -> CCRSet.add (Self, name) refs
            (* not allowed *)
            | CIexpr _
            | CIstatic
            | CIparent ->
              refs
          end
        | _ -> refs
      in
      CCRSet.union refs acc
  end

let class_const env (cc : Nast.class_const) =
  let gather_constants = gather_constants#on_expr CCRSet.empty in
  let { cc_id = name; cc_type = h; cc_expr = e; cc_doc_comment = _ } = cc in
  let pos = Decl_env.make_decl_pos env (fst name) in
  let (ty, abstract, scc_refs) =
    (* Optional hint h, optional expression e *)
    match (h, e) with
    | (Some h, Some e) -> (Decl_hint.hint env h, false, gather_constants e)
    | (Some h, None) -> (Decl_hint.hint env h, true, CCRSet.empty)
    | (None, Some e) ->
      let (e_pos, e_) = e in
      let cc_refs = gather_constants e in
      begin
        match Decl_utils.infer_const e_ with
        | Some tprim ->
          ( mk
              ( Reason.Rwitness_from_decl (Decl_env.make_decl_pos env e_pos),
                Tprim tprim ),
            false,
            cc_refs )
        | None ->
          (* Typing will take care of rejecting constants that have neither
           * an initializer nor a literal initializer *)
          ( mk (Reason.Rwitness_from_decl pos, Typing_defs.make_tany ()),
            false,
            cc_refs )
      end
    | (None, None) ->
      (* Typing will take care of rejecting constants that have neither
       * an initializer nor a literal initializer *)
      let r = Reason.Rwitness_from_decl pos in
      (mk (r, Typing_defs.make_tany ()), true, CCRSet.empty)
  in
  (* dropping to list to avoid the memory cost of a set. We don't really
   * need a set once the elements are generated.
   *)
  let scc_refs = CCRSet.elements scc_refs in
  Some
    {
      scc_abstract = abstract;
      scc_name = Decl_env.make_decl_posed env name;
      scc_type = ty;
      scc_refs;
    }

let typeconst env c tc =
  match c.c_kind with
  | Ast_defs.Cenum -> None
  | Ast_defs.Ctrait
  | Ast_defs.Cinterface
  | Ast_defs.Cabstract
  | Ast_defs.Cnormal ->
    let (abstract, as_constraint, super_constraint, ty) =
      match tc.c_tconst_kind with
      | Aast.TCAbstract
          { c_atc_as_constraint; c_atc_super_constraint; c_atc_default } ->
        ( TCAbstract (Option.map ~f:(Decl_hint.hint env) c_atc_default),
          Option.map ~f:(Decl_hint.hint env) c_atc_as_constraint,
          Option.map ~f:(Decl_hint.hint env) c_atc_super_constraint,
          None )
      | Aast.TCConcrete { c_tc_type } ->
        (TCConcrete, None, None, Some (Decl_hint.hint env c_tc_type))
      | Aast.TCPartiallyAbstract { c_patc_constraint; c_patc_type } ->
        ( TCPartiallyAbstract,
          Some (Decl_hint.hint env c_patc_constraint),
          None,
          Some (Decl_hint.hint env c_patc_type) )
    in
    let attributes = tc.c_tconst_user_attributes in
    let enforceable =
      match Attrs.find SN.UserAttributes.uaEnforceable attributes with
      | Some { ua_name = (pos, _); _ } -> (Decl_env.make_decl_pos env pos, true)
      | None -> (Pos_or_decl.none, false)
    in
    let reifiable =
      match Attrs.find SN.UserAttributes.uaReifiable attributes with
      | Some { ua_name = (pos, _); _ } -> Some pos
      | None -> None
    in
    Some
      {
        stc_abstract = abstract;
        stc_name = Decl_env.make_decl_posed env tc.c_tconst_name;
        stc_as_constraint = as_constraint;
        stc_super_constraint = super_constraint;
        stc_type = ty;
        stc_enforceable = enforceable;
        stc_reifiable = Option.map ~f:(Decl_env.make_decl_pos env) reifiable;
      }

let make_xhp_attr cv =
  Option.map cv.cv_xhp_attr (fun xai ->
      {
        xa_tag =
          (match xai.xai_tag with
          | None -> None
          | Some Required -> Some Required
          | Some LateInit -> Some Lateinit);
        xa_has_default = Option.is_some cv.cv_expr;
      })

let prop env cv =
  let cv_pos = Decl_env.make_decl_pos env @@ fst cv.cv_id in
  let ty =
    FunUtils.hint_to_type_opt
      env
      ~is_lambda:false
      (Reason.Rglobal_class_prop cv_pos)
      (hint_of_type_hint cv.cv_type)
  in
  let const = Attrs.mem SN.UserAttributes.uaConst cv.cv_user_attributes in
  let lateinit = Attrs.mem SN.UserAttributes.uaLateInit cv.cv_user_attributes in
  let php_std_lib =
    Attrs.mem SN.UserAttributes.uaPHPStdLib cv.cv_user_attributes
  in
  {
    sp_name = Decl_env.make_decl_posed env cv.cv_id;
    sp_xhp_attr = make_xhp_attr cv;
    sp_type = ty;
    sp_visibility = cv.cv_visibility;
    sp_flags =
      PropFlags.make
        ~const
        ~lateinit
        ~lsb:false
        ~needs_init:(Option.is_none cv.cv_expr)
        ~abstract:cv.cv_abstract
        ~php_std_lib
        ~readonly:cv.cv_readonly;
  }

and static_prop env cv =
  let (cv_pos, cv_name) = Decl_env.make_decl_posed env cv.cv_id in
  let ty =
    FunUtils.hint_to_type_opt
      env
      ~is_lambda:false
      (Reason.Rglobal_class_prop cv_pos)
      (hint_of_type_hint cv.cv_type)
  in
  let id = "$" ^ cv_name in
  let lateinit = Attrs.mem SN.UserAttributes.uaLateInit cv.cv_user_attributes in
  let lsb = Attrs.mem SN.UserAttributes.uaLSB cv.cv_user_attributes in
  let const = Attrs.mem SN.UserAttributes.uaConst cv.cv_user_attributes in
  let php_std_lib =
    Attrs.mem SN.UserAttributes.uaPHPStdLib cv.cv_user_attributes
  in
  {
    sp_name = (cv_pos, id);
    sp_xhp_attr = make_xhp_attr cv;
    sp_type = ty;
    sp_visibility = cv.cv_visibility;
    sp_flags =
      PropFlags.make
        ~const
        ~lateinit
        ~lsb
        ~needs_init:(Option.is_none cv.cv_expr)
        ~abstract:cv.cv_abstract
        ~php_std_lib
        ~readonly:cv.cv_readonly;
  }

let method_type env m =
  let ifc_decl = FunUtils.find_policied_attribute m.m_user_attributes in
  let return_disposable =
    FunUtils.has_return_disposable_attribute m.m_user_attributes
  in
  let params = FunUtils.make_params env ~is_lambda:false m.m_params in
  let (_pos, capability) =
    Decl_hint.aast_contexts_to_decl_capability env m.m_ctxs (fst m.m_name)
  in
  let ret =
    FunUtils.ret_from_fun_kind
      ~is_lambda:false
      ~is_constructor:(String.equal (snd m.m_name) SN.Members.__construct)
      env
      (fst m.m_name)
      m.m_fun_kind
      (hint_of_type_hint m.m_ret)
  in
  let arity =
    match m.m_variadic with
    | FVvariadicArg param ->
      assert param.param_is_variadic;
      Fvariadic (FunUtils.make_param_ty env ~is_lambda:false param)
    | FVellipsis p -> Fvariadic (FunUtils.make_ellipsis_param_ty env p)
    | FVnonVariadic -> Fstandard
  in
  let tparams = List.map m.m_tparams (FunUtils.type_param env) in
  let where_constraints =
    List.map m.m_where_constraints (FunUtils.where_constraint env)
  in
  {
    ft_arity = arity;
    ft_tparams = tparams;
    ft_where_constraints = where_constraints;
    ft_params = params;
    ft_implicit_params = { capability };
    ft_ret = { et_type = ret; et_enforced = Unenforced };
    ft_flags =
      make_ft_flags
        m.m_fun_kind
        ~return_disposable
        ~returns_readonly:(Option.is_some m.m_readonly_ret)
        ~readonly_this:m.m_readonly_this;
    ft_ifc_decl = ifc_decl;
  }

let method_ env m =
  let override = Attrs.mem SN.UserAttributes.uaOverride m.m_user_attributes in
  let (pos, _) = Decl_env.make_decl_posed env m.m_name in
  let has_dynamicallycallable =
    Attrs.mem SN.UserAttributes.uaDynamicallyCallable m.m_user_attributes
  in
  let php_std_lib =
    Attrs.mem SN.UserAttributes.uaPHPStdLib m.m_user_attributes
  in
  let sound_dynamic_callable =
    Attrs.mem SN.UserAttributes.uaSoundDynamicCallable m.m_user_attributes
  in
  let ft = method_type env m in
  let sm_deprecated =
    Naming_attributes_deprecated.deprecated
      ~kind:"method"
      m.m_name
      m.m_user_attributes
  in
  {
    sm_name = Decl_env.make_decl_posed env m.m_name;
    sm_type = mk (Reason.Rwitness_from_decl pos, Tfun ft);
    sm_visibility = m.m_visibility;
    sm_deprecated;
    sm_flags =
      MethodFlags.make
        ~abstract:m.m_abstract
        ~final:m.m_final
        ~override
        ~dynamicallycallable:has_dynamicallycallable
        ~php_std_lib
        ~sound_dynamic_callable;
  }

let class_ ctx c =
  let (errs, result) =
    Errors.do_ @@ fun () ->
    let (_, cls_name) = c.c_name in
    let class_dep = Dep.Type cls_name in
    let env = { Decl_env.mode = c.c_mode; droot = Some class_dep; ctx } in
    let hint = Decl_hint.hint env in
    let (req_extends, req_implements) = split_reqs c in
    let (static_vars, vars) = split_vars c in
    let (constructor, statics, rest) = split_methods c in
    let sc_extends = List.map ~f:hint c.c_extends in
    let sc_uses = List.map ~f:hint c.c_uses in
    let sc_req_extends = List.map ~f:hint req_extends in
    let sc_req_implements = List.map ~f:hint req_implements in
    let sc_implements = List.map ~f:hint c.c_implements in
    let where_constraints =
      List.map c.c_where_constraints (FunUtils.where_constraint env)
    in
    let enum_type hint e =
      {
        te_base = hint e.e_base;
        te_constraint = Option.map e.e_constraint hint;
        te_includes = List.map e.e_includes hint;
        te_enum_class = e.e_enum_class;
      }
    in
    {
      sc_mode = c.c_mode;
      sc_final = c.c_final;
      sc_is_xhp = c.c_is_xhp;
      sc_has_xhp_keyword = c.c_has_xhp_keyword;
      sc_kind = c.c_kind;
      sc_name = Decl_env.make_decl_posed env c.c_name;
      sc_tparams = List.map c.c_tparams (FunUtils.type_param env);
      sc_where_constraints = where_constraints;
      sc_extends;
      sc_uses;
      sc_xhp_attr_uses = List.map ~f:hint c.c_xhp_attr_uses;
      sc_req_extends;
      sc_req_implements;
      sc_implements;
      sc_implements_dynamic = c.c_implements_dynamic;
      sc_consts = List.filter_map c.c_consts (class_const env);
      sc_typeconsts = List.filter_map c.c_typeconsts (typeconst env c);
      sc_props = List.map ~f:(prop env) vars;
      sc_sprops = List.map ~f:(static_prop env) static_vars;
      sc_constructor = Option.map ~f:(method_ env) constructor;
      sc_static_methods = List.map ~f:(method_ env) statics;
      sc_methods = List.map ~f:(method_ env) rest;
      sc_user_attributes =
        List.map
          c.c_user_attributes
          ~f:(Decl_hint.aast_user_attribute_to_decl_user_attribute env);
      sc_enum_type = Option.map c.c_enum (enum_type hint);
    }
  in
  if not (Errors.is_empty errs) then (
    let reason =
      Errors.get_error_list errs
      |> Errors.convert_errors_to_string
      |> List.map ~f:(fun err ->
             Printf.sprintf
               "%s\nCallstack:\n%s"
               err
               (Caml.Printexc.raw_backtrace_to_string
                  (Caml.Printexc.get_callstack 500)))
      |> String.concat ~sep:"\n"
    in
    HackEventLogger.shallow_decl_errors_emitted reason;
    Errors.merge_into_current errs
  );
  result
