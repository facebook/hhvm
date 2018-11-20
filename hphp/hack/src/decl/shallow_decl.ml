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
    sc_consts = c.c_consts;
    sc_typeconsts = c.c_typeconsts;
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
