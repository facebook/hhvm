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
    sc_static_vars = c.c_static_vars;
    sc_vars = c.c_vars;
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
