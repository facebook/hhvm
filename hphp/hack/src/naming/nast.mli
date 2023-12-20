(** Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val show_tprim : Aast.tprim -> string

type program = (unit, unit) Aast.program

val pp_program :
  Ppx_deriving_runtime.Format.formatter -> program -> Ppx_deriving_runtime.unit

val show_program : program -> Ppx_deriving_runtime.string

type def = (unit, unit) Aast.def

type expr = (unit, unit) Aast.expr

val equal_expr : expr -> expr -> Ppx_deriving_runtime.bool

val pp_expr :
  Ppx_deriving_runtime.Format.formatter -> expr -> Ppx_deriving_runtime.unit

val show_expr : expr -> Ppx_deriving_runtime.string

type expr_ = (unit, unit) Aast.expr_

type stmt = (unit, unit) Aast.stmt

type block = (unit, unit) Aast.block

type user_attribute = (unit, unit) Aast.user_attribute

val equal_user_attribute :
  user_attribute -> user_attribute -> Ppx_deriving_runtime.bool

val pp_user_attribute :
  Ppx_deriving_runtime.Format.formatter ->
  user_attribute ->
  Ppx_deriving_runtime.unit

val show_user_attribute : user_attribute -> Ppx_deriving_runtime.string

type class_id_ = (unit, unit) Aast.class_id_

val equal_class_id_ : class_id_ -> class_id_ -> Ppx_deriving_runtime.bool

type class_ = (unit, unit) Aast.class_

type class_var = (unit, unit) Aast.class_var

type method_ = (unit, unit) Aast.method_

type file_attribute = (unit, unit) Aast.file_attribute

type fun_ = (unit, unit) Aast.fun_

type capture_lid = unit Aast.capture_lid

type efun = (unit, unit) Aast.efun

type fun_def = (unit, unit) Aast.fun_def

type func_body = (unit, unit) Aast.func_body

type fun_param = (unit, unit) Aast.fun_param

type typedef = (unit, unit) Aast.typedef

type tparam = (unit, unit) Aast.tparam

type gconst = (unit, unit) Aast.gconst

type class_const = (unit, unit) Aast.class_const

type class_id = (unit, unit) Aast.class_id

type catch = (unit, unit) Aast.catch

type case = (unit, unit) Aast.case

type stmt_match = (unit, unit) Aast.stmt_match

type stmt_match_arm = (unit, unit) Aast.stmt_match_arm

type default_case = (unit, unit) Aast.default_case

type gen_case = (unit, unit) Aast.gen_case

type field = (unit, unit) Aast.field

type afield = (unit, unit) Aast.afield

type xhp_attribute = (unit, unit) Aast.xhp_attribute

type expression_tree = (unit, unit) Aast.expression_tree

type targ = unit Aast.targ

type sid = Aast.sid

val pp_sid :
  Ppx_deriving_runtime.Format.formatter -> sid -> Ppx_deriving_runtime.unit

val show_sid : sid -> Ppx_deriving_runtime.string

type shape_field_name = Ast_defs.shape_field_name

type hint = Aast.hint

type class_hint = Aast.class_hint

type trait_hint = Aast.trait_hint

type xhp_attr_hint = Aast.xhp_attr_hint

type type_hint = unit Aast.type_hint

type module_def = (unit, unit) Aast.module_def

module ShapeMap = Ast_defs.ShapeMap

val class_id_to_str : ('a, 'b) Aast.class_id_ -> Ast_defs.id_

val is_kvc_kind : Hh_prelude.String.t -> bool

val get_kvc_kind : Hh_prelude.String.t -> Aast.kvc_kind

val kvc_kind_to_name : Aast.kvc_kind -> string

val is_vc_kind : Hh_prelude.String.t -> bool

val get_vc_kind : Hh_prelude.String.t -> Aast.vc_kind

val vc_kind_to_name : Aast.vc_kind -> string

val map_xhp_attr :
  (Aast.pstring -> Aast.pstring) ->
  (expr -> expr) ->
  (unit, unit) Aast.xhp_attribute ->
  (unit, unit) Aast.xhp_attribute

val get_xhp_attr_expr : ('a, 'b) Aast.xhp_attribute -> ('a, 'b) Aast.expr

val get_simple_xhp_attrs :
  ('a, 'b) Aast.xhp_attribute list -> (Aast.pstring * ('a, 'b) Aast.expr) list

(** Definitions appearing in a Nast.program *)
type defs = {
  funs: (FileInfo.id * fun_def) list;
  classes: (FileInfo.id * class_) list;
  typedefs: (FileInfo.id * typedef) list;
  constants: (FileInfo.id * gconst) list;
  modules: (FileInfo.id * module_def) list;
  stmts: (FileInfo.id * stmt) list;
}

(** Given a Nast.program, give me the list of entities it defines *)
val get_defs : program -> defs

val get_def_names : program -> FileInfo.ids

type ignore_attribute_env = { ignored_attributes: string list }

val deregister_ignored_attributes : program -> (unit, unit) Aast.def list

val remove_pos_and_docblock :
  (unit, unit) Aast.def list -> (unit, unit) Aast.def list

val generate_ast_decl_hash : (unit, unit) Aast.def list -> OpaqueDigest.t

module Visitor_DEPRECATED : sig
  type id = Aast.lid

  class type ['a] visitor_type =
    object
      method on_afield : 'a -> afield -> 'a

      method on_array_get : 'a -> expr -> expr option -> 'a

      method on_as : 'a -> expr -> hint -> bool -> 'a

      method on_as_expr : 'a -> (unit, unit) Aast.as_expr -> 'a

      method on_await : 'a -> expr -> 'a

      method on_awaitall : 'a -> (id * expr) list -> block -> 'a

      method on_binop : 'a -> Ast_defs.bop -> expr -> expr -> 'a

      method on_block : 'a -> block -> 'a

      method on_break : 'a -> 'a

      method on_call :
        'a -> expr -> (Ast_defs.param_kind * expr) list -> expr option -> 'a

      method on_case : 'a -> case -> 'a

      method on_cast : 'a -> hint -> expr -> 'a

      method on_catch : 'a -> catch -> 'a

      method on_class_ : 'a -> class_ -> 'a

      method on_class_c_const : 'a -> class_const -> 'a

      method on_class_const : 'a -> class_id -> Aast.pstring -> 'a

      method on_class_get :
        'a -> class_id -> (unit, unit) Aast.class_get_expr -> 'a

      method on_class_id : 'a -> class_id -> 'a

      method on_class_id_ : 'a -> class_id_ -> 'a

      method on_class_req : 'a -> hint * Aast.require_kind -> 'a

      method on_class_typeconst_def :
        'a -> (unit, unit) Aast.class_typeconst_def -> 'a

      method on_class_use : 'a -> hint -> 'a

      method on_class_var : 'a -> class_var -> 'a

      method on_clone : 'a -> expr -> 'a

      method on_collection :
        'a -> unit Aast.collection_targ option -> afield list -> 'a

      method on_concurrent : 'a -> block -> 'a

      method on_continue : 'a -> 'a

      method on_declare_local : 'a -> Aast.lid -> hint -> expr option -> 'a

      method on_def : 'a -> def -> 'a

      method on_default_case : 'a -> default_case -> 'a

      method on_do : 'a -> block -> expr -> 'a

      method on_dollardollar : 'a -> id -> 'a

      method on_efun : 'a -> efun -> 'a

      method on_eif : 'a -> expr -> expr option -> expr -> 'a

      method on_enum_class_label : 'a -> sid option -> string -> 'a

      method on_et_splice : 'a -> expr -> 'a

      method on_expr : 'a -> expr -> 'a

      method on_expr_ : 'a -> expr_ -> 'a

      method on_expression_tree : 'a -> expression_tree -> 'a

      method on_fallthrough : 'a -> 'a

      method on_false : 'a -> 'a

      method on_field : 'a -> field -> 'a

      method on_float : 'a -> string -> 'a

      method on_for : 'a -> expr list -> expr option -> expr list -> block -> 'a

      method on_foreach : 'a -> expr -> (unit, unit) Aast.as_expr -> block -> 'a

      method on_fun_ : 'a -> fun_ -> 'a

      method on_fun_def : 'a -> fun_def -> 'a

      method on_func_body : 'a -> func_body -> 'a

      method on_function_pointer :
        'a -> (unit, unit) Aast.function_ptr_id -> targ list -> 'a

      method on_function_ptr_id : 'a -> (unit, unit) Aast.function_ptr_id -> 'a

      method on_gconst : 'a -> gconst -> 'a

      method on_hint : 'a -> hint -> 'a

      method on_id : 'a -> sid -> 'a

      method on_if : 'a -> expr -> block -> block -> 'a

      method on_int : 'a -> string -> 'a

      method on_is : 'a -> expr -> hint -> 'a

      method on_keyValCollection :
        'a ->
        Aast.pos * Aast.kvc_kind ->
        (targ * targ) option ->
        field list ->
        'a

      method on_lfun : 'a -> fun_ -> capture_lid list -> 'a

      method on_list : 'a -> expr list -> 'a

      method on_lvar : 'a -> id -> 'a

      method on_markup : 'a -> Aast.pstring -> 'a

      method on_method_ : 'a -> method_ -> 'a

      method on_method_caller : 'a -> sid -> Aast.pstring -> 'a

      method on_new : 'a -> class_id -> expr list -> expr option -> 'a

      method on_noop : 'a -> 'a

      method on_null : 'a -> 'a

      method on_obj_get : 'a -> expr -> expr -> 'a

      method on_omitted : 'a -> 'a

      method on_pair : 'a -> (targ * targ) option -> expr -> expr -> 'a

      method on_param_kind : 'a -> Ast_defs.param_kind -> 'a

      method on_pat_refinement : 'a -> Aast.pat_refinement -> 'a

      method on_pat_var : 'a -> Aast.pat_var -> 'a

      method on_pattern : 'a -> Aast.pattern -> 'a

      method on_pipe : 'a -> id -> expr -> expr -> 'a

      method on_program : 'a -> program -> 'a

      method on_readonly_expr : 'a -> expr -> 'a

      method on_record : 'a -> sid -> (expr * expr) list -> 'a

      method on_return : 'a -> expr option -> 'a

      method on_shape : 'a -> (Ast_defs.shape_field_name * expr) list -> 'a

      method on_stmt : 'a -> stmt -> 'a

      method on_stmt_ : 'a -> (unit, unit) Aast.stmt_ -> 'a

      method on_stmt_match : 'a -> stmt_match -> 'a

      method on_stmt_match_arm : 'a -> stmt_match_arm -> 'a

      method on_string : 'a -> string -> 'a

      method on_string2 : 'a -> expr list -> 'a

      method on_switch : 'a -> expr -> case list -> default_case option -> 'a

      method on_targ : 'a -> targ -> 'a

      method on_this : 'a -> 'a

      method on_throw : 'a -> expr -> 'a

      method on_true : 'a -> 'a

      method on_try : 'a -> block -> catch list -> block -> 'a

      method on_type_hint : 'a -> type_hint -> 'a

      method on_typedef : 'a -> typedef -> 'a

      method on_unop : 'a -> Ast_defs.uop -> expr -> 'a

      method on_upcast : 'a -> expr -> hint -> 'a

      method on_using : 'a -> (unit, unit) Aast.using_stmt -> 'a

      method on_valCollection :
        'a -> Aast.pos * Aast.vc_kind -> targ option -> expr list -> 'a

      method on_while : 'a -> expr -> block -> 'a

      method on_xml : 'a -> sid -> xhp_attribute list -> expr list -> 'a

      method on_yield : 'a -> afield -> 'a

      method on_yield_break : 'a -> 'a
    end

  class virtual ['a] visitor : ['a] visitor_type
end
[@@ocaml.deprecated
  "This has holes and needs manual upates. Use [Nast.iter] or [Nast.reduce]"]
