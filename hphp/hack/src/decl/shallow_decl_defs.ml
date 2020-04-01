(*
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
  scc_abstract: bool;
  scc_expr: Nast.expr option;
  scc_name: Aast.sid;
  scc_type: decl_ty;
}
[@@deriving eq, show]

type shallow_typeconst = {
  stc_abstract: typeconst_abstract_kind;
  stc_constraint: decl_ty option;
  stc_name: Aast.sid;
  stc_type: decl_ty option;
  stc_enforceable: Pos.t * bool;
  stc_reifiable: Pos.t option;
}
[@@deriving eq, show]

type shallow_pu_member = {
  spum_atom: Aast.sid;
  spum_types: (Aast.sid * decl_ty) list;
  spum_exprs: Aast.sid list;
}
[@@deriving eq, show]

type shallow_pu_enum = {
  spu_name: Aast.sid;
  spu_is_final: bool;
  spu_case_types: (Aast.sid * Aast.reify_kind) list;
  spu_case_values: (Aast.sid * decl_ty) list;
  spu_members: shallow_pu_member list;
}
[@@deriving eq, show]

type shallow_prop = {
  sp_const: bool;
  sp_xhp_attr: xhp_attr option;
  sp_lateinit: bool;
  sp_lsb: bool;
  sp_name: Aast.sid;
  sp_needs_init: bool;
  sp_type: decl_ty option;
  sp_abstract: bool;
  sp_visibility: Aast.visibility;
  sp_fixme_codes: ISet.t;
}
[@@deriving eq, show]

type shallow_method = {
  sm_abstract: bool;
  sm_final: bool;
  sm_memoizelsb: bool;
  sm_name: Aast.sid;
  sm_override: bool;
  sm_dynamicallycallable: bool;
  sm_reactivity: Decl_defs.method_reactivity option;
  sm_type: decl_ty;
  sm_visibility: Aast.visibility;
  sm_fixme_codes: ISet.t;
  sm_deprecated: string option;
}
[@@deriving eq, show]

type shallow_method_redeclaration = {
  smr_abstract: bool;
  smr_final: bool;
  smr_static: bool;
  smr_name: Aast.sid;
  smr_type: decl_ty;
  smr_visibility: Aast.visibility;
  smr_trait: Aast.hint;
  smr_method: Aast.pstring;
  smr_fixme_codes: ISet.t;
}
[@@deriving eq, show]

type shallow_class = {
  sc_mode: FileInfo.mode;
  sc_final: bool;
  sc_is_xhp: bool;
  sc_has_xhp_keyword: bool;
  sc_kind: Ast_defs.class_kind;
  sc_name: Aast.sid;
  sc_tparams: decl_tparam list;
  sc_where_constraints: decl_where_constraint list;
  sc_extends: decl_ty list;
  sc_uses: decl_ty list;
  sc_method_redeclarations: shallow_method_redeclaration list;
  sc_xhp_attr_uses: decl_ty list;
  sc_req_extends: decl_ty list;
  sc_req_implements: decl_ty list;
  sc_implements: decl_ty list;
  sc_consts: shallow_class_const list;
  sc_typeconsts: shallow_typeconst list;
  sc_pu_enums: shallow_pu_enum list;
  sc_props: shallow_prop list;
  sc_sprops: shallow_prop list;
  sc_constructor: shallow_method option;
  sc_static_methods: shallow_method list;
  sc_methods: shallow_method list;
  sc_user_attributes: Nast.user_attribute list;
  sc_enum_type: enum_type option;
  sc_decl_errors: Errors.t; [@opaque]
}
[@@deriving eq, show]
