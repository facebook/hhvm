(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Shallow_decl_defs
open Nast
open Typing_deps
open Typing_defs

module Attrs = Attributes

let class_const env c (h, name, e) =
  let pos = fst name in
  match c.c_kind with
  | Ast.Ctrait ->
      let kind = match c.c_kind with
        | Ast.Ctrait -> `trait
        | Ast.Cenum -> `enum
        | _ -> assert false in
      Errors.cannot_declare_constant kind pos c.c_name;
      None
  | Ast.Cnormal | Ast.Cabstract | Ast.Cinterface | Ast.Cenum ->
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
              if c.c_mode = FileInfo.Mstrict && c.c_kind <> Ast.Cenum
              then Errors.missing_typehint pos;
              (Reason.Rwitness pos, Tany), false
          end
        | None, None ->
          if c.c_mode = FileInfo.Mstrict then Errors.missing_typehint pos;
          let r = Reason.Rwitness pos in
          (r, Tany), true
    in
    Some {
      scc_abstract = abstract;
      scc_expr = e;
      scc_name = name;
      scc_type = ty;
    }

let typeconst env c tc =
  match c.c_kind with
  | Ast.Ctrait | Ast.Cenum ->
      let kind = match c.c_kind with
        | Ast.Ctrait -> `trait
        | Ast.Cenum -> `enum
        | _ -> assert false in
      Errors.cannot_declare_constant kind (fst tc.c_tconst_name) c.c_name;
      None
  | Ast.Cinterface | Ast.Cabstract | Ast.Cnormal ->
      let constr = Option.map tc.c_tconst_constraint (Decl_hint.hint env) in
      let ty = Option.map tc.c_tconst_type (Decl_hint.hint env) in
      Some {
        stc_name = tc.c_tconst_name;
        stc_constraint = constr;
        stc_type = ty;
      }

let prop env cv =
  let cv_pos = fst cv.cv_id in
  let ty = Option.map cv.cv_type ~f:begin fun ty' ->
    if cv.cv_is_xhp
    then
      (* If this is an XHP attribute and we're in strict mode,
         relax to partial mode to allow the use of the "array"
         annotation without specifying type parameters. Until
         recently HHVM did not allow "array" with type parameters
         in XHP attribute declarations, so this is a temporary
         hack to support existing code for now. *)
      (* Task #5815945: Get rid of this Hack *)
      let env =
        if Decl_env.mode env = FileInfo.Mstrict
        then { env with Decl_env.mode = FileInfo.Mpartial }
        else env
      in
      Decl_hint.hint env ty'
    else Decl_hint.hint env ty'
  end in
  let const = Attrs.mem SN.UserAttributes.uaConst cv.cv_user_attributes in
  let lateinit = Attrs.mem SN.UserAttributes.uaLateInit cv.cv_user_attributes in
  if cv.cv_final then Errors.final_property cv_pos;
  if lateinit && cv.cv_expr <> None then Errors.lateinit_with_default cv_pos;
  {
    sp_const = const;
    sp_is_xhp_attr = cv.cv_is_xhp;
    sp_lateinit = lateinit;
    sp_lsb = false;
    sp_name = cv.cv_id;
    sp_needs_init = Option.is_none cv.cv_expr;
    sp_type = ty;
    sp_visibility = cv.cv_visibility;
  }

and static_prop env c cv =
  let cv_pos, cv_name = cv.cv_id in
  let ty = Option.map cv.cv_type ~f:(Decl_hint.hint env) in
  let id = "$" ^ cv_name in
  let lateinit = Attrs.mem SN.UserAttributes.uaLateInit cv.cv_user_attributes in
  let lsb = Attrs.mem SN.UserAttributes.uaLSB cv.cv_user_attributes in
  if cv.cv_expr = None && FileInfo.(c.c_mode = Mstrict || c.c_mode = Mpartial)
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
    sp_is_xhp_attr = cv.cv_is_xhp;
    sp_lateinit = lateinit;
    sp_lsb = lsb;
    sp_name = cv_pos, id;
    sp_needs_init = Option.is_none cv.cv_expr;
    sp_type = ty;
    sp_visibility = cv.cv_visibility;
  }

let method_ m =
  {
    sm_final = m.m_final;
    sm_abstract = m.m_abstract;
    sm_visibility = m.m_visibility;
    sm_name = m.m_name;
    sm_tparams = m.m_tparams;
    sm_where_constraints = m.m_where_constraints;
    sm_variadic = m.m_variadic;
    sm_params = m.m_params;
    sm_fun_kind = m.m_fun_kind;
    sm_user_attributes = m.m_user_attributes;
    sm_ret = m.m_ret;
    sm_ret_by_ref = m.m_ret_by_ref;
  }

let class_ env c =
  let hint = Decl_hint.hint env in
  {
    sc_mode = c.c_mode;
    sc_final = c.c_final;
    sc_is_xhp = c.c_is_xhp;
    sc_kind = c.c_kind;
    sc_name = c.c_name;
    sc_tparams = c.c_tparams;
    sc_extends        = List.map ~f:hint c.c_extends;
    sc_uses           = List.map ~f:hint c.c_uses;
    sc_xhp_attr_uses  = List.map ~f:hint c.c_xhp_attr_uses;
    sc_req_extends    = List.map ~f:hint c.c_req_extends;
    sc_req_implements = List.map ~f:hint c.c_req_implements;
    sc_implements     = List.map ~f:hint c.c_implements;
    sc_consts = List.filter_map c.c_consts (class_const env c);
    sc_typeconsts = List.filter_map c.c_typeconsts (typeconst env c);
    sc_props = List.map c.c_vars (prop env);
    sc_sprops = List.map c.c_static_vars (static_prop env c);
    sc_constructor = Option.map c.c_constructor ~f:method_;
    sc_static_methods = List.map c.c_static_methods method_;
    sc_methods = List.map c.c_methods method_;
    sc_user_attributes = c.c_user_attributes;
    sc_enum = c.c_enum;
    sc_decl_errors = Errors.empty;
  }

let class_ tcopt c =
  let _, cls_name = c.c_name in
  let class_dep = Dep.Class cls_name in
  let env = {
    Decl_env.mode = c.c_mode;
    droot = Some class_dep;
    decl_tcopt = tcopt;
  } in
  let errors, sc = Errors.do_ (fun () -> class_ env c) in
  { sc with sc_decl_errors = errors }
