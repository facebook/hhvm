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
  scc_name: Ast_defs.id;
  scc_type: decl_ty;
}
[@@deriving eq, show]

type shallow_typeconst = {
  stc_abstract: typeconst_abstract_kind;
  stc_constraint: decl_ty option;
  stc_name: Ast_defs.id;
  stc_type: decl_ty option;
  stc_enforceable: Pos.t * bool;
  stc_reifiable: Pos.t option;
}
[@@deriving eq, show]

type shallow_pu_member = {
  spum_atom: Ast_defs.id;
  spum_types: (Ast_defs.id * decl_ty) list;
  spum_exprs: Ast_defs.id list;
}
[@@deriving eq, show]

type shallow_pu_enum = {
  spu_name: Ast_defs.id;
  spu_is_final: bool;
  spu_case_types: decl_tparam list;
  spu_case_values: (Ast_defs.id * decl_ty) list;
  spu_members: shallow_pu_member list;
}
[@@deriving eq, show]

type shallow_prop = {
  sp_const: bool;
  sp_xhp_attr: xhp_attr option;
  sp_lateinit: bool;
  sp_lsb: bool;
  sp_name: Ast_defs.id;
  sp_needs_init: bool;
  sp_type: decl_ty option;
  sp_abstract: bool;
  sp_visibility: Ast_defs.visibility;
}
[@@deriving eq, show]

type shallow_method = {
  sm_abstract: bool;
  sm_final: bool;
  sm_memoizelsb: bool;
  sm_name: Ast_defs.id;
  sm_override: bool;
  sm_dynamicallycallable: bool;
  sm_reactivity: Decl_defs.method_reactivity option;
  sm_type: decl_ty;
  sm_visibility: Ast_defs.visibility;
  sm_deprecated: string option;
}
[@@deriving eq, show]

type shallow_class = {
  sc_mode: FileInfo.mode;
  sc_final: bool;
  sc_is_xhp: bool;
  sc_has_xhp_keyword: bool;
  sc_kind: Ast_defs.class_kind;
  sc_name: Ast_defs.id;
  sc_tparams: decl_tparam list;
  sc_where_constraints: decl_where_constraint list;
  sc_extends: decl_ty list;
  sc_uses: decl_ty list;
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
  sc_user_attributes: user_attribute list;
  sc_enum_type: enum_type option;
}
[@@deriving eq, show]
