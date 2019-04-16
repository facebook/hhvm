(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Aast_defs

module type AnnotationType = sig
  type t
  val pp : Format.formatter -> t -> unit
end

module type ASTAnnotationTypes = sig
  module ExprAnnotation : AnnotationType
  module EnvAnnotation : AnnotationType
  module FuncBodyAnnotation : AnnotationType
end

module AnnotatedAST(Annotations: ASTAnnotationTypes) =
struct

module ExprAnnotation = Annotations.ExprAnnotation
module EnvAnnotation = Annotations.EnvAnnotation
module FuncBodyAnnotation = Annotations.FuncBodyAnnotation

type program = def list
[@@deriving
  show { with_path = false },
  visitors {
    variety = "iter";
    nude = true;
    visit_prefix = "on_";
    ancestors = ["iter_defs"];
  },
  visitors {
    variety = "reduce";
    nude = true;
    visit_prefix = "on_";
    ancestors = ["reduce_defs"];
  },
  visitors {
    variety = "map";
    nude = true;
    visit_prefix = "on_";
    ancestors = ["map_defs"];
  },
  visitors {
    variety = "endo";
    nude = true;
    visit_prefix = "on_";
    ancestors = ["endo_defs"];
  }]

and expr_annotation = ExprAnnotation.t [@visitors.opaque]
and env_annotation = EnvAnnotation.t [@visitors.opaque]
and funcbody_annotation = Annotations.FuncBodyAnnotation.t [@visitors.opaque]

and stmt = pos * stmt_

and stmt_ =
  | Unsafe_block of block
  | Fallthrough
  | Expr of expr
  | Break
  | Continue
  (* is_terminal is new *)
  | Throw of is_terminal * expr
  | Return of expr option
  | GotoLabel of pstring
  | Goto of pstring
  | Awaitall of ((lid option * expr) list * block)
  | If of expr * block * block
  | Do of block * expr
  | While of expr * block
  | Using of using_stmt
  | For of expr * expr * expr * block
  | Switch of expr * case list
  (* Dropped the Pos.t option *)
  | Foreach of expr * as_expr * block
  | Try of block * catch list * block
  | Def_inline of def
  | Let of lid * hint option * expr
  | Noop
  | Block of block
  | Markup of pstring * expr option
  | Declare of (* is_block *) bool * expr * block

and using_stmt = {
  us_is_block_scoped: bool;
  us_has_await: bool;
  us_expr: expr;
  us_block: block;
}

and as_expr =
  | As_v of expr
  | As_kv of expr * expr
  (* This is not in AST *)
  | Await_as_v of pos * expr
  | Await_as_kv of pos * expr * expr

and block = stmt list

(* This is not in AST *)
and class_id = expr_annotation * class_id_
and class_id_ =
  | CIparent
  | CIself
  | CIstatic
  | CIexpr of expr
  | CI of sid

and expr = expr_annotation * expr_
and expr_ =
  | Array of afield list
  | Darray of (targ * targ) option * (expr * expr) list
  | Varray of targ option * expr list
  | Shape of (Ast.shape_field_name * expr) list
    (* TODO: T38184446 Consolidate collections in AAST *)
  | ValCollection of vc_kind * targ option * expr list
    (* TODO: T38184446 Consolidate collections in AAST *)
  | KeyValCollection of kvc_kind * (targ * targ) option * field list
  | Null
  | This
  | True
  | False
  | Omitted
  | Id of sid
  | Lvar of lid
  | ImmutableVar of lid
  | Dollardollar of lid
  | Clone of expr
  | Obj_get of expr * expr * og_null_flavor
  | Array_get of expr * expr option
  | Class_get of class_id * class_get_expr
  | Class_const of class_id * pstring
  | Call of call_type
    * expr (* function *)
    * targ list (* explicit type annotations *)
    * expr list (* positional args *)
    * expr list (* unpacked args *)
  | Int of string
  | Float of string
  | String of string
  | String2 of expr list
  | PrefixedString of string * expr
  | Yield of afield
  | Yield_break
  | Yield_from of expr
  | Await of expr
  | Suspend of expr
  | List of expr list
  | Expr_list of expr list
  | Cast of hint * expr
  | Unop of Ast.uop * expr
  | Binop of Ast.bop * expr * expr
  (** The ID of the $$ that is implicitly declared by this pipe. *)
  | Pipe of lid * expr * expr
  | Eif of expr * expr option * expr
  | InstanceOf of expr * class_id
  | Is of expr * hint
  | As of expr * hint * (* is nullable *) bool
  | New of class_id * targ list * expr list * expr list * (* constructor *) expr_annotation
  | Record of class_id * (expr * expr) list
  | Efun of fun_ * lid list
  | Lfun of fun_ * lid list
  | Xml of sid * xhp_attribute list * expr list
  | Unsafe_expr of expr
  | Callconv of Ast.param_kind * expr
  | Import of import_flavor * expr
  (* TODO: T38184446 Consolidate collections in AAST *)
  | Collection of sid * collection_targ option * afield list
  | BracedExpr of expr
  | ParenthesizedExpr of expr
  (* None of these constructors exist in the AST *)
  | Lplaceholder of pos
  | Fun_id of sid
  | Method_id of expr * pstring
  (* meth_caller('Class name', 'method name') *)
  | Method_caller of sid * pstring
  | Smethod_id of sid * pstring
  | Special_func of special_func
  | Pair of expr * expr
  | Assert of assert_expr
  | Typename of sid
  | PU_atom of string
  | PU_identifier of class_id * pstring * pstring
  | Any

and class_get_expr =
  | CGstring of pstring
  | CGexpr of expr

(* These are "very special" constructs that we look for in, among
 * other places, terminality checks. invariant does not appear here
 * because it gets rewritten to If + AE_invariant_violation.
 *
 * TODO: get rid of assert_expr entirely in favor of rewriting to if
 * and noreturn *)
and assert_expr =
  | AE_assert of expr

and case =
  | Default of block
  | Case of expr * block

and catch = sid * lid * block

and field = expr * expr
and afield =
  | AFvalue of expr
  | AFkvalue of expr * expr

and xhp_attribute =
  | Xhp_simple of pstring * expr
  | Xhp_spread of expr

and special_func =
  | Gena of expr
  | Genva of expr list
  | Gen_array_rec of expr

and is_reference = bool
and is_variadic = bool
and fun_param = {
  param_annotation : expr_annotation;
  param_hint : hint option;
  param_is_reference : is_reference;
  param_is_variadic : is_variadic;
  param_pos : pos;
  param_name : string;
  param_expr : expr option;
  param_callconv : Ast.param_kind option;
  param_user_attributes : user_attribute list;
}

and fun_variadicity = (* does function take varying number of args? *)
  | FVvariadicArg of fun_param (* PHP5.6 ...$args finishes the func declaration *)
  | FVellipsis of pos (* HH ... finishes the declaration; deprecate for ...$args? *)
  | FVnonVariadic (* standard non variadic function *)

and fun_ = {
  f_span     : pos;
  f_annotation : env_annotation;
  f_mode     : FileInfo.mode [@opaque];
  f_ret      : hint option;
  f_name     : sid;
  f_tparams  : tparam list;
  f_where_constraints : where_constraint list;
  f_variadic : fun_variadicity;
  f_params   : fun_param list;
  f_body     : func_body;
  f_fun_kind : Ast.fun_kind;
  f_user_attributes : user_attribute list;
  f_file_attributes : file_attribute list;
  f_external : bool;  (* true if this declaration has no body because it is an
                         external function declaration (e.g. from an HHI file)*)
  f_namespace : nsenv;
  f_doc_comment : string option;
  f_static : bool;
}

(**
 * Naming has two phases and the annotation helps to indicate the phase.
 * In the first pass, it will perform naming on everything except for function
 * and method bodies and collect information needed. Then, another round of
 * naming is performed where function bodies are named. Thus, naming will
 * have named and unnamed variants of the annotation.
 * See BodyNamingAnnotation in nast.ml and the comment in naming.ml
 *)
and func_body = {
  fb_ast : block;
  fb_annotation : funcbody_annotation
}

and user_attribute = {
  ua_name: sid;
  ua_params: expr list (* user attributes are restricted to scalar values *)
}

and file_attribute = {
  fa_user_attributes: user_attribute list;
  fa_namespace: nsenv;
}

and tparam = {
  tp_variance: Ast.variance;
  tp_name: sid;
  tp_constraints: (Ast.constraint_kind * hint) list;
  tp_reified: reify_kind;
  tp_user_attributes: user_attribute list
}

and static_var = class_var
and static_method = method_
and constructor = method_

and class_tparams = {
  c_tparam_list: tparam list;
  (* TODO: remove this and use tp_constraints *)
  (* keeping around the ast version of the constraint only
   * for the purposes of Naming.class_meth_bodies *)
  c_tparam_constraints: (reify_kind * (Ast.constraint_kind * hint) list) SMap.t [@opaque]
}

and class_ = {
  c_span           : pos              ;
  c_annotation     : env_annotation   ;
  c_mode           : FileInfo.mode [@opaque];
  c_final          : bool             ;
  c_is_xhp         : bool;
  c_kind           : Ast.class_kind   ;
  c_name           : sid              ;
  (* The type parameters of a class A<T> (T is the parameter) *)
  c_tparams        : class_tparams    ;
  c_extends        : hint list        ;
  c_uses           : hint list        ;
  c_method_redeclarations : method_redeclaration list;
  c_xhp_attr_uses  : hint list        ;
  c_xhp_category   : (pos * pstring list) option;
  c_req_extends    : hint list        ;
  c_req_implements : hint list        ;
  c_implements     : hint list        ;
  c_consts         : class_const list ;
  c_typeconsts     : class_typeconst list;
  c_static_vars    : static_var list  ;
  c_vars           : class_var list   ;
  c_constructor    : constructor option;
  c_static_methods : static_method list;
  c_methods        : method_ list     ;
  c_attributes     : class_attr list  ;
  c_xhp_children   : (pos * xhp_child) list;
  c_xhp_attrs      : xhp_attr list    ;
  c_namespace      : nsenv            ;
  c_user_attributes: user_attribute list;
  c_file_attributes: file_attribute list;
  c_enum           : enum_ option     ;
  c_pu_enums       : pu_enum list     ;
  c_doc_comment    : string option    ;
}

and xhp_attr = hint option * class_var * bool * ((pos * bool * expr list) option)

and class_attr =
  | CA_name of sid
  | CA_field of ca_field

and ca_field = {
  ca_type: ca_type;
  ca_id: sid;
  ca_value: expr option;
  ca_required: bool;
}

and ca_type =
  | CA_hint of hint
  | CA_enum of string list

(* expr = None indicates an abstract const *)
and class_const = hint option * sid * expr option

and typeconst_abstract_kind =
  | TCAbstract of hint option (* default *)
  | TCPartiallyAbstract
  | TCConcrete

(* This represents a type const definition. If a type const is abstract then
 * then the type hint acts as a constraint. Any concrete definition of the
 * type const must satisfy the constraint.
 *
 * If the type const is not abstract then a type must be specified.
 *)
and class_typeconst = {
  c_tconst_abstract : typeconst_abstract_kind;
  c_tconst_name : sid;
  c_tconst_constraint : hint option;
  c_tconst_type : hint option;
  c_tconst_user_attributes : user_attribute list;
}

and class_var = {
  cv_final           : bool               ;
  cv_is_xhp          : bool               ;
  cv_visibility      : visibility         ;
  cv_type            : hint option        ;
  cv_id              : sid                ;
  cv_expr            : expr option        ;
  cv_user_attributes : user_attribute list;
  cv_doc_comment     : string option      ;
  cv_is_promoted_variadic : bool          ;
}

and method_ = {
  m_span            : pos                 ;
  m_annotation      : env_annotation      ;
  m_final           : bool                ;
  m_abstract        : bool                ;
  m_static          : bool                ;
  m_visibility      : visibility          ;
  m_name            : sid                 ;
  m_tparams         : tparam list         ;
  m_where_constraints : where_constraint list;
  m_variadic        : fun_variadicity     ;
  m_params          : fun_param list      ;
  m_body            : func_body           ;
  m_fun_kind        : Ast.fun_kind        ;
  m_user_attributes : user_attribute list ;
  m_ret             : hint option         ;
  m_external        : bool                ;  (* see f_external above for context *)
  m_doc_comment     : string option       ;
}

and method_redeclaration = {
  mt_final           : bool                ;
  mt_abstract        : bool                ;
  mt_static          : bool                ;
  mt_visibility      : visibility          ;
  mt_name            : sid                 ;
  mt_tparams         : tparam list         ;
  mt_where_constraints : where_constraint list;
  mt_variadic        : fun_variadicity     ;
  mt_params          : fun_param list      ;
  mt_fun_kind        : Ast.fun_kind        ;
  mt_ret             : hint option         ;
  mt_trait           : hint                ;
  mt_method          : pstring             ;
  mt_user_attributes : user_attribute list;
}

and nsenv = Namespace_env.env [@opaque]

and typedef = {
  t_annotation : env_annotation;
  t_name : sid;
  t_tparams : tparam list;
  t_constraint : hint option;
  t_kind : hint;
  t_user_attributes : user_attribute list;
  t_mode : FileInfo.mode [@opaque];
  t_vis : typedef_visibility;
  t_namespace : nsenv;
}

and gconst = {
  cst_annotation : env_annotation;
  cst_mode: FileInfo.mode [@opaque];
  cst_name: sid;
  cst_type: hint option;
  cst_value: expr option;
  cst_namespace: nsenv;
  cst_span: pos;
}

and pu_enum = {
  pu_name: sid;
  pu_is_final: bool;
  pu_case_types: sid list;
  pu_case_values: (sid * hint) list;
  pu_members: pu_member list;
}

and pu_member = {
  pum_atom: sid;
  pum_types: (sid * hint) list;
  pum_exprs: (sid * expr) list;
}

and fun_def = fun_

and def =
  | Fun of fun_def
  | Class of class_
  | Stmt of stmt
  | Typedef of typedef
  | Constant of gconst
  | Namespace of sid * program
  | NamespaceUse of (ns_kind * sid * sid) list
  | SetNamespaceEnv of nsenv
  | FileAttributes of file_attribute

and ns_kind =
  | NSNamespace
  | NSClass
  | NSClassAndNamespace
  | NSFun
  | NSConst

and reify_kind =
  | Erased
  | SoftReified
  | Reified

let expr_to_string expr =
  match expr with
  | Any -> "Any"
  | Array _ -> "Array"
  | Darray _ -> "Darray"
  | Varray _ -> "Varray"
  | Shape _ -> "Shape"
  | ValCollection _ -> "ValCollection"
  | KeyValCollection _ -> "KeyValCollection"
  | This -> "This"
  | Id _ -> "Id"
  | Lvar _ -> "Lvar"
  | ImmutableVar _ -> "ImmutableVar"
  | Lplaceholder _ -> "Lplaceholder"
  | Dollardollar _ -> "Dollardollar"
  | Fun_id _ -> "Fun_id"
  | Method_id _ -> "Method_id"
  | Method_caller _ -> "Method_caller"
  | Smethod_id _ -> "Smethod_id"
  | Obj_get _ -> "Obj_get"
  | Array_get _ -> "Array_get"
  | Class_get _  -> "Class_get"
  | Class_const _  -> "Class_const"
  | Call _  -> "Call"
  | True -> "True"
  | False -> "False"
  | Int _  -> "Int"
  | Float _  -> "Float"
  | Null -> "Null"
  | String _  -> "String"
  | String2 _  -> "String2"
  | PrefixedString _ -> "PrefixedString"
  | Special_func _  -> "Special_func"
  | Yield_break -> "Yield_break"
  | Yield _  -> "Yield"
  | Yield_from _ -> "Yield_from"
  | Await _  -> "Await"
  | Suspend _ -> "Suspend"
  | List _  -> "List"
  | Pair _  -> "Pair"
  | Expr_list _  -> "Expr_list"
  | Cast _  -> "Cast"
  | Unop _  -> "Unop"
  | Binop _  -> "Binop"
  | Pipe _  -> "Pipe"
  | Eif _  -> "Eif"
  | InstanceOf _  -> "InstanceOf"
  | Is _ -> "Is"
  | As _ -> "As"
  | New _  -> "New"
  | Record _ -> "Record"
  | Efun _  -> "Efun"
  | Xml _  -> "Xml"
  | Unsafe_expr _ -> "Unsafe_expr"
  | Callconv _ -> "Callconv"
  | Assert _  -> "Assert"
  | Clone _  -> "Clone"
  | Typename _  -> "Typename"
  | Omitted -> "Omitted"
  | Lfun _ -> "Lfun"
  | Import _ -> "Import"
  | Collection _ -> "Collection"
  | BracedExpr _ -> "BracedExpr"
  | ParenthesizedExpr _ -> "ParenthesizedExpr"
  | PU_atom _ -> "PU_atom"
  | PU_identifier _ -> "PU_identifier"

end (* of AnnotatedAST functor *)
