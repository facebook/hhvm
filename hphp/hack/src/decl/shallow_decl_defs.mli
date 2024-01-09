(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

module PropFlags : sig
  type t [@@deriving eq, show]

  val empty : t

  val get_abstract    : t -> bool
  val get_const       : t -> bool
  val get_lateinit    : t -> bool
  val get_lsb         : t -> bool
  val get_needs_init  : t -> bool
  val get_php_std_lib : t -> bool
  val get_readonly    : t -> bool
  val get_safe_global_variable: t -> bool
  val get_no_auto_likes : t -> bool

  val set_abstract    : bool -> t -> t
  val set_const       : bool -> t -> t
  val set_lateinit    : bool -> t -> t
  val set_lsb         : bool -> t -> t
  val set_needs_init  : bool -> t -> t
  val set_php_std_lib : bool -> t -> t
  val set_readonly    : bool -> t -> t
  val set_safe_global_variable: bool -> t -> t
  val set_no_auto_likes : bool -> t -> t

  val make :
    abstract:bool ->
    const:bool ->
    lateinit:bool ->
    lsb:bool ->
    needs_init:bool ->
    php_std_lib:bool ->
    readonly: bool ->
    safe_global_variable: bool ->
    no_auto_likes: bool ->
    t
end
[@@ocamlformat "disable"]

module MethodFlags : sig
  type t [@@deriving eq, show]

  val empty : t

  val get_abstract               : t -> bool
  val get_final                  : t -> bool
  val get_override               : t -> bool
  val get_dynamicallycallable    : t -> bool
  val get_php_std_lib            : t -> bool
  val get_support_dynamic_type : t -> bool

  val set_abstract               : bool -> t -> t
  val set_final                  : bool -> t -> t
  val set_override               : bool -> t -> t
  val set_dynamicallycallable    : bool -> t -> t
  val set_php_std_lib            : bool -> t -> t
  val set_support_dynamic_type : bool -> t -> t

  val make :
    abstract:bool ->
    final:bool ->
    override:bool ->
    dynamicallycallable:bool ->
    php_std_lib:bool ->
    support_dynamic_type:bool ->
    t
end
[@@ocamlformat "disable"]

type shallow_class_const = {
  scc_abstract: Typing_defs.class_const_kind;
  scc_name: Typing_defs.pos_id;
  scc_type: decl_ty;
  scc_refs: Typing_defs.class_const_ref list;
  scc_value: string option;
}
[@@deriving eq, show]

type shallow_typeconst = {
  stc_name: Typing_defs.pos_id;
  stc_kind: Typing_defs.typeconst;
  stc_enforceable: Pos_or_decl.t * bool;
  stc_reifiable: Pos_or_decl.t option;
  stc_is_ctx: bool;
}
[@@deriving eq, show]

type shallow_prop = {
  sp_name: Typing_defs.pos_id;
  sp_xhp_attr: xhp_attr option;
  sp_type: decl_ty;
  sp_visibility: Ast_defs.visibility;
  sp_flags: PropFlags.t;
}
[@@deriving eq, show]

type shallow_method = {
  sm_name: Typing_defs.pos_id;
  sm_type: decl_ty;
  sm_visibility: Ast_defs.visibility;
  sm_deprecated: string option;
  sm_flags: MethodFlags.t;
  sm_attributes: user_attribute list;
  sm_sort_text: string option;
}
[@@deriving eq, show]

type xhp_enum_values = Ast_defs.xhp_enum_value list SMap.t [@@deriving eq, show]

type shallow_class = {
  sc_mode: FileInfo.mode;
  sc_final: bool;
  sc_abstract: bool;
  sc_is_xhp: bool;
  sc_internal: bool;
  sc_has_xhp_keyword: bool;
  sc_kind: Ast_defs.classish_kind;
  sc_module: Ast_defs.id option;
  sc_name: Typing_defs.pos_id;
  sc_tparams: decl_tparam list;
  sc_where_constraints: decl_where_constraint list;
  sc_extends: decl_ty list;
  sc_uses: decl_ty list;
  sc_xhp_attr_uses: decl_ty list;
  sc_xhp_enum_values: xhp_enum_values;
  sc_xhp_marked_empty: bool;
  sc_req_extends: decl_ty list;
  sc_req_implements: decl_ty list;
  sc_req_class: decl_ty list;
  sc_implements: decl_ty list;
  sc_support_dynamic_type: bool;
  sc_consts: shallow_class_const list;
  sc_typeconsts: shallow_typeconst list;
  sc_props: shallow_prop list;
  sc_sprops: shallow_prop list;
  sc_constructor: shallow_method option;
  sc_static_methods: shallow_method list;
  sc_methods: shallow_method list;
  sc_user_attributes: user_attribute list;
  sc_enum_type: enum_type option;
  sc_docs_url: string option;
}
[@@deriving eq, show]

val sp_abstract : shallow_prop -> bool

val sp_const : shallow_prop -> bool

val sp_lateinit : shallow_prop -> bool

val sp_lsb : shallow_prop -> bool

val sp_needs_init : shallow_prop -> bool

val sp_php_std_lib : shallow_prop -> bool

val sp_readonly : shallow_prop -> bool

val sp_safe_global_variable : shallow_prop -> bool

val sm_abstract : shallow_method -> bool

val sm_final : shallow_method -> bool

val sm_override : shallow_method -> bool

val sm_dynamicallycallable : shallow_method -> bool

val sm_php_std_lib : shallow_method -> bool

val sm_support_dynamic_type : shallow_method -> bool

type fun_decl = fun_elt [@@deriving show]

type class_decl = shallow_class [@@deriving show]

type typedef_decl = typedef_type [@@deriving show]

type const_decl = Typing_defs.const_decl [@@deriving show]

type module_decl = Typing_defs.module_def_type [@@deriving show]

type decl =
  | Class of class_decl
  | Fun of fun_decl
  | Typedef of typedef_decl
  | Const of const_decl
  | Module of module_decl
[@@deriving show]

type named_decl =
  | NClass of string * class_decl
  | NFun of string * fun_decl
  | NTypedef of string * typedef_decl
  | NConst of string * const_decl
  | NModule of string * module_decl
[@@deriving show]

val to_class_decl_opt : decl -> class_decl option

val to_fun_decl_opt : decl -> fun_decl option

val to_typedef_decl_opt : decl -> typedef_decl option

val to_const_decl_opt : decl -> const_decl option

val to_module_decl_opt : decl -> module_decl option
