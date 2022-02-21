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
      let (_, _, expr_) = expr in
      let refs =
        match expr_ with
        | Class_const ((_, _, CI (_, from)), (_, name)) ->
          CCRSet.add (From from, name) refs
        | Class_const ((_, _, CIself), (_, name)) ->
          CCRSet.add (Self, name) refs
        (* Following two cases are for function-pointers "const mixed X = S::i<>".
           These shouldn't really be counted as refs for our purposes. But we're including
           them solely for reason of parity with the direct-decl-parser, which does count them. *)
        | FunctionPointer
            (FP_class_const ((_, _, CI (_, from)), (_, name)), _targs) ->
          CCRSet.add (From from, name) refs
        | FunctionPointer (FP_class_const ((_, _, CIself), (_, name)), _targs)
          ->
          CCRSet.add (Self, name) refs
        | _ -> refs
      in
      CCRSet.union refs acc

    method! on_SFclass_const acc (_, class_id) (_, name) =
      (* TODO: recognize "self::F"... The shape key "shape(self::F => 1)" is
         represented by SFclass_const with class_id just the string literal "self".
         However, by the time this visitor is called, it has already been resolved
         into a class-name. Therefore this place in the code is unable to properly
         reconstruct "self::F" shape keys. *)
      let ref = (From class_id, name) in
      CCRSet.add ref acc
  end

let make_visibility attrs = function
  | Public when Attrs.mem SN.UserAttributes.uaInternal attrs -> Internal
  | vis -> vis

let class_const env (cc : Nast.class_const) =
  let gather_constants = gather_constants#on_expr CCRSet.empty in
  let {
    cc_id = name;
    cc_type = h;
    cc_kind = k;
    cc_doc_comment = _;
    cc_user_attributes = _;
  } =
    cc
  in
  let pos = Decl_env.make_decl_pos env (fst name) in
  let (abstract, scc_refs) =
    match k with
    | CCAbstract (Some default) -> (CCAbstract true, gather_constants default)
    | CCAbstract None -> (CCAbstract false, CCRSet.empty)
    | CCConcrete e -> (CCConcrete, gather_constants e)
  in
  let ty =
    match h with
    | Some h -> Decl_hint.hint env h
    | None ->
      (* Error recovery for when a type hint is missing on a constant *)
      (match k with
      | CCAbstract (Some (_, e_pos, e_))
      | CCConcrete (_, e_pos, e_) ->
        begin
          match Decl_utils.infer_const e_ with
          | Some tprim ->
            mk
              ( Reason.Rwitness_from_decl (Decl_env.make_decl_pos env e_pos),
                Tprim tprim )
          | None ->
            (* Typing will take care of rejecting constants that have neither
             * an initializer nor a literal initializer *)
            mk (Reason.Rwitness_from_decl pos, Typing_defs.make_tany ())
        end
      | CCAbstract None ->
        (* Typing will take care of rejecting constants that have neither
         * an initializer nor a literal initializer *)
        let r = Reason.Rwitness_from_decl pos in
        mk (r, Typing_defs.make_tany ()))
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
  (* We keep contexts as intersection and do not simplify empty or singleton
   * ones.
   *)
  match c.c_kind with
  | Ast_defs.Cenum_class _
  | Ast_defs.Cenum ->
    None
  | Ast_defs.Ctrait
  | Ast_defs.Cinterface
  | Ast_defs.Cclass _ ->
    let stc_kind =
      match tc.c_tconst_kind with
      | Aast.TCAbstract
          {
            c_atc_as_constraint = a;
            c_atc_super_constraint = s;
            c_atc_default = d;
          } ->
        Typing_defs.TCAbstract
          {
            atc_as_constraint = Option.map ~f:(Decl_hint.hint env) a;
            atc_super_constraint = Option.map ~f:(Decl_hint.hint env) s;
            atc_default = Option.map ~f:(Decl_hint.hint env) d;
          }
      | Aast.TCConcrete { c_tc_type = t } ->
        Typing_defs.TCConcrete { tc_type = Decl_hint.hint env t }
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
        stc_name = Decl_env.make_decl_posed env tc.c_tconst_name;
        stc_kind;
        stc_enforceable = enforceable;
        stc_reifiable = Option.map ~f:(Decl_env.make_decl_pos env) reifiable;
        stc_is_ctx = tc.c_tconst_is_ctx;
      }

let make_xhp_attr cv =
  Option.map cv.cv_xhp_attr ~f:(fun xai ->
      {
        Xhp_attribute.xa_tag =
          (match xai.xai_tag with
          | None -> None
          | Some Required -> Some Xhp_attribute.Required
          | Some LateInit -> Some Xhp_attribute.LateInit);
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
  let vis = make_visibility cv.cv_user_attributes cv.cv_visibility in
  {
    sp_name = Decl_env.make_decl_posed env cv.cv_id;
    sp_xhp_attr = make_xhp_attr cv;
    sp_type = ty;
    sp_visibility = vis;
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
  let vis = make_visibility cv.cv_user_attributes cv.cv_visibility in
  {
    sp_name = (cv_pos, id);
    sp_xhp_attr = make_xhp_attr cv;
    sp_type = ty;
    sp_visibility = vis;
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

let method_type ~support_dynamic_type env m =
  let ifc_decl = FunUtils.find_policied_attribute m.m_user_attributes in
  let return_disposable =
    FunUtils.has_return_disposable_attribute m.m_user_attributes
  in
  let mt_is_memoized = FunUtils.has_memoize_attribute m.m_user_attributes in
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
  let tparams = List.map m.m_tparams ~f:(FunUtils.type_param env) in
  let where_constraints =
    List.map m.m_where_constraints ~f:(FunUtils.where_constraint env)
  in
  {
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
        ~readonly_this:m.m_readonly_this
        ~support_dynamic_type
        ~is_memoized:mt_is_memoized
        ~variadic:(List.exists m.m_params ~f:(fun p -> p.param_is_variadic));
    ft_ifc_decl = ifc_decl;
  }

let method_ ~support_dynamic_type env m =
  let override = Attrs.mem SN.UserAttributes.uaOverride m.m_user_attributes in
  let (pos, _) = Decl_env.make_decl_posed env m.m_name in
  let has_dynamicallycallable =
    Attrs.mem SN.UserAttributes.uaDynamicallyCallable m.m_user_attributes
  in
  let php_std_lib =
    Attrs.mem SN.UserAttributes.uaPHPStdLib m.m_user_attributes
  in
  (* Method is regarded as SupportDynamicType if the class is or if the
   * method is explicitly marked so *)
  let support_dynamic_type =
    support_dynamic_type
    || Attrs.mem SN.UserAttributes.uaSupportDynamicType m.m_user_attributes
  in
  let ft = method_type ~support_dynamic_type env m in
  let sm_deprecated =
    Naming_attributes_params.deprecated
      ~kind:"method"
      m.m_name
      m.m_user_attributes
  in
  let vis = make_visibility m.m_user_attributes m.m_visibility in
  {
    sm_name = Decl_env.make_decl_posed env m.m_name;
    sm_type = mk (Reason.Rwitness_from_decl pos, Tfun ft);
    sm_visibility = vis;
    sm_deprecated;
    sm_flags =
      MethodFlags.make
        ~abstract:m.m_abstract
        ~final:m.m_final
        ~override
        ~dynamicallycallable:has_dynamicallycallable
        ~php_std_lib
        ~support_dynamic_type;
    sm_attributes = [];
  }

let xhp_enum_values props =
  List.fold props ~init:SMap.empty ~f:(fun acc prop ->
      match prop.cv_xhp_attr with
      | Some { xai_enum_values = []; _ } -> acc
      | Some { xai_enum_values; _ } ->
        SMap.add (snd prop.cv_id) xai_enum_values acc
      | None -> acc)

let class_DEPRECATED ctx c =
  let (errs, result) =
    Errors.do_ @@ fun () ->
    let (_, cls_name) = c.c_name in
    let class_dep = Dep.Type cls_name in
    let env = { Decl_env.mode = c.c_mode; droot = Some class_dep; ctx } in
    let hint = Decl_hint.hint env in
    let (req_extends, req_implements) = split_reqs c.c_reqs in
    let (static_vars, vars) = split_vars c.c_vars in
    let sc_xhp_enum_values = xhp_enum_values vars in
    let (constructor, statics, rest) = split_methods c.c_methods in
    let sc_extends = List.map ~f:hint c.c_extends in
    let sc_uses = List.map ~f:hint c.c_uses in
    let sc_req_extends = List.map ~f:hint req_extends in
    let sc_req_implements = List.map ~f:hint req_implements in
    let sc_implements = List.map ~f:hint c.c_implements in
    let sc_user_attributes =
      List.map
        c.c_user_attributes
        ~f:(Decl_hint.aast_user_attribute_to_decl_user_attribute env)
    in
    let sc_module =
      Naming_attributes_params.get_module_attribute c.c_file_attributes
    in
    let where_constraints =
      List.map c.c_where_constraints ~f:(FunUtils.where_constraint env)
    in
    let support_dynamic_type =
      Attrs.mem SN.UserAttributes.uaSupportDynamicType c.c_user_attributes
    in
    let enum_type hint e =
      {
        te_base = hint e.e_base;
        te_constraint = Option.map e.e_constraint ~f:hint;
        te_includes = List.map e.e_includes ~f:hint;
      }
    in
    let sc_abstract = Ast_defs.is_c_abstract c.c_kind in
    {
      sc_mode = c.c_mode;
      sc_final = c.c_final;
      sc_abstract;
      sc_is_xhp = c.c_is_xhp;
      sc_has_xhp_keyword = c.c_has_xhp_keyword;
      sc_kind = c.c_kind;
      sc_module;
      sc_name = Decl_env.make_decl_posed env c.c_name;
      sc_tparams = List.map c.c_tparams ~f:(FunUtils.type_param env);
      sc_where_constraints = where_constraints;
      sc_extends;
      sc_uses;
      sc_xhp_attr_uses = List.map ~f:hint c.c_xhp_attr_uses;
      sc_xhp_enum_values;
      sc_req_extends;
      sc_req_implements;
      sc_implements;
      sc_support_dynamic_type = support_dynamic_type;
      sc_consts = List.filter_map c.c_consts ~f:(class_const env);
      sc_typeconsts = List.filter_map c.c_typeconsts ~f:(typeconst env c);
      sc_props = List.map ~f:(prop env) vars;
      sc_sprops = List.map ~f:(static_prop env) static_vars;
      sc_constructor =
        Option.map ~f:(method_ ~support_dynamic_type:false env) constructor;
      sc_static_methods =
        List.map ~f:(method_ ~support_dynamic_type env) statics;
      sc_methods = List.map ~f:(method_ ~support_dynamic_type env) rest;
      sc_user_attributes;
      sc_enum_type = Option.map c.c_enum ~f:(enum_type hint);
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
