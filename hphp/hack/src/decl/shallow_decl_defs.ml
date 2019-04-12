(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Pp_type

type shallow_class_const = {
  scc_abstract : bool;
  scc_expr     : Nast.expr option;
  scc_name     : Aast.sid;
  scc_type     : decl ty;
} [@@deriving show]

type shallow_typeconst = {
  stc_abstract    : Nast.typeconst_abstract_kind;
  stc_constraint  : decl ty option;
  stc_name        : Aast.sid;
  stc_type        : decl ty option;
  stc_enforceable : Pos.t * bool;
} [@@deriving show]

type shallow_prop = {
  sp_const       : bool;
  sp_is_xhp_attr : bool;
  sp_lateinit    : bool;
  sp_lsb         : bool;
  sp_name        : Aast.sid;
  sp_needs_init  : bool;
  sp_type        : decl ty option;
  sp_visibility  : Aast.visibility;
} [@@deriving show]

type shallow_method = {
  sm_abstract   : bool;
  sm_final      : bool;
  sm_memoizelsb : bool;
  sm_name       : Aast.sid;
  sm_override   : bool;
  sm_reactivity : Decl_defs.method_reactivity option;
  sm_type       : decl fun_type;
  sm_visibility : Aast.visibility;
} [@@deriving show]

type shallow_method_redeclaration = {
  smr_abstract   : bool;
  smr_final      : bool;
  smr_static     : bool;
  smr_name       : Aast.sid;
  smr_type       : decl fun_type;
  smr_visibility : Aast.visibility;
  smr_trait      : Aast.hint;
  smr_method     : Aast.pstring;
} [@@deriving show]

type shallow_class = {
  sc_mode            : FileInfo.mode;
  sc_final           : bool;
  sc_is_xhp          : bool;
  sc_kind            : Ast.class_kind;
  sc_name            : Aast.sid;
  sc_tparams         : decl tparam list;
  sc_extends         : decl ty list;
  sc_uses            : decl ty list;
  sc_method_redeclarations : shallow_method_redeclaration list;
  sc_xhp_attr_uses   : decl ty list;
  sc_req_extends     : decl ty list;
  sc_req_implements  : decl ty list;
  sc_implements      : decl ty list;
  sc_consts          : shallow_class_const list;
  sc_typeconsts      : shallow_typeconst list;
  sc_props           : shallow_prop list;
  sc_sprops          : shallow_prop list;
  sc_constructor     : shallow_method option;
  sc_static_methods  : shallow_method list;
  sc_methods         : shallow_method list;
  sc_user_attributes : Nast.user_attribute list;
  sc_enum_type       : enum_type option;
  sc_decl_errors     : Errors.t [@opaque];
} [@@deriving show]
