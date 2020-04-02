(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Aast_defs

(** Aast.program represents the top-level definitions in a Hack program.
 ex: Expression annotation type (when typechecking, the inferred dtype)
 fb: Function body tag (e.g. has naming occurred)
 en: Environment (tracking state inside functions and classes)
 hi: Hint annotation (when typechecking it will be the localized type hint or the
     inferred missing type if the hint is missing)
 *)
type ('ex, 'fb, 'en, 'hi) program = ('ex, 'fb, 'en, 'hi) def list
[@@deriving
  show { with_path = false },
    eq,
    visitors
      {
        variety = "iter";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["iter_defs"];
      },
    visitors
      {
        variety = "reduce";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["reduce_defs"];
      },
    visitors
      {
        variety = "map";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["map_defs"];
      },
    visitors
      {
        variety = "endo";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["endo_defs"];
      }]

and ('ex, 'fb, 'en, 'hi) stmt = pos * ('ex, 'fb, 'en, 'hi) stmt_

and ('ex, 'fb, 'en, 'hi) stmt_ =
  | Fallthrough
  | Expr of ('ex, 'fb, 'en, 'hi) expr
  | Break
  | Continue
  | Throw of ('ex, 'fb, 'en, 'hi) expr
  | Return of ('ex, 'fb, 'en, 'hi) expr option
  | GotoLabel of pstring
  | Goto of pstring
  | Awaitall of
      (lid option * ('ex, 'fb, 'en, 'hi) expr) list * ('ex, 'fb, 'en, 'hi) block
  | If of
      ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) block
      * ('ex, 'fb, 'en, 'hi) block
  | Do of ('ex, 'fb, 'en, 'hi) block * ('ex, 'fb, 'en, 'hi) expr
  | While of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) block
  | Using of ('ex, 'fb, 'en, 'hi) using_stmt
  | For of
      ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) block
  | Switch of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) case list
  (* Dropped the Pos.t option *)
  | Foreach of
      ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) as_expr
      * ('ex, 'fb, 'en, 'hi) block
  | Try of
      ('ex, 'fb, 'en, 'hi) block
      * ('ex, 'fb, 'en, 'hi) catch list
      * ('ex, 'fb, 'en, 'hi) block
  | Noop
  | Block of ('ex, 'fb, 'en, 'hi) block
  | Markup of pstring

and ('ex, 'fb, 'en, 'hi) using_stmt = {
  us_is_block_scoped: bool;
  us_has_await: bool;
  us_expr: ('ex, 'fb, 'en, 'hi) expr;
  us_block: ('ex, 'fb, 'en, 'hi) block;
}

and ('ex, 'fb, 'en, 'hi) as_expr =
  | As_v of ('ex, 'fb, 'en, 'hi) expr
  | As_kv of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr
  (* This is not in AST *)
  | Await_as_v of pos * ('ex, 'fb, 'en, 'hi) expr
  | Await_as_kv of pos * ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr

and ('ex, 'fb, 'en, 'hi) block = ('ex, 'fb, 'en, 'hi) stmt list

(* This is not in AST *)
and ('ex, 'fb, 'en, 'hi) class_id = 'ex * ('ex, 'fb, 'en, 'hi) class_id_

and ('ex, 'fb, 'en, 'hi) class_id_ =
  | CIparent
  | CIself
  | CIstatic
  | CIexpr of ('ex, 'fb, 'en, 'hi) expr
  | CI of sid

and ('ex, 'fb, 'en, 'hi) expr = 'ex * ('ex, 'fb, 'en, 'hi) expr_

and 'hi collection_targ =
  | CollectionTV of 'hi targ
  | CollectionTKV of 'hi targ * 'hi targ

and ('ex, 'fb, 'en, 'hi) expr_ =
  | Array of ('ex, 'fb, 'en, 'hi) afield list
  | Darray of
      ('hi targ * 'hi targ) option
      * (('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr) list
  | Varray of 'hi targ option * ('ex, 'fb, 'en, 'hi) expr list
  | Shape of (Ast_defs.shape_field_name * ('ex, 'fb, 'en, 'hi) expr) list
  | ValCollection of vc_kind * 'hi targ option * ('ex, 'fb, 'en, 'hi) expr list
      (** TODO: T38184446 Consolidate collections in AAST *)
  | KeyValCollection of
      kvc_kind * ('hi targ * 'hi targ) option * ('ex, 'fb, 'en, 'hi) field list
      (** TODO: T38184446 Consolidate collections in AAST *)
  | Null
  | This
  | True
  | False
  | Omitted
  | Id of sid
  | Lvar of lid
  | Dollardollar of lid
  | Clone of ('ex, 'fb, 'en, 'hi) expr
  | Obj_get of
      ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr * og_null_flavor
  | Array_get of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr option
  | Class_get of
      ('ex, 'fb, 'en, 'hi) class_id * ('ex, 'fb, 'en, 'hi) class_get_expr
  | Class_const of ('ex, 'fb, 'en, 'hi) class_id * pstring
  | Call of
      call_type
      * ('ex, 'fb, 'en, 'hi) expr
      (* function *)
      * 'hi targ list
      (* explicit type annotations *)
      * ('ex, 'fb, 'en, 'hi) expr list
      (* positional args *)
      * ('ex, 'fb, 'en, 'hi) expr option (* unpacked arg *)
  | FunctionPointer of ('ex, 'fb, 'en, 'hi) expr * 'hi targ list
  | Int of string
  | Float of string
  | String of string
  | String2 of ('ex, 'fb, 'en, 'hi) expr list
  | PrefixedString of string * ('ex, 'fb, 'en, 'hi) expr
  | Yield of ('ex, 'fb, 'en, 'hi) afield
  | Yield_break
  | Yield_from of ('ex, 'fb, 'en, 'hi) expr
  | Await of ('ex, 'fb, 'en, 'hi) expr
  | Suspend of ('ex, 'fb, 'en, 'hi) expr
  | List of ('ex, 'fb, 'en, 'hi) expr list
  | Expr_list of ('ex, 'fb, 'en, 'hi) expr list
  | Cast of hint * ('ex, 'fb, 'en, 'hi) expr
  | Unop of Ast_defs.uop * ('ex, 'fb, 'en, 'hi) expr
  | Binop of
      Ast_defs.bop * ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr
  | Pipe of lid * ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr
      (** The lid is the ID of the $$ that is implicitly declared by this pipe. *)
  | Eif of
      ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) expr option
      * ('ex, 'fb, 'en, 'hi) expr
  | Is of ('ex, 'fb, 'en, 'hi) expr * hint
  | As of ('ex, 'fb, 'en, 'hi) expr * hint * (* is nullable *) bool
  | New of
      ('ex, 'fb, 'en, 'hi) class_id
      * 'hi targ list
      * ('ex, 'fb, 'en, 'hi) expr list
      * ('ex, 'fb, 'en, 'hi) expr option
      * 'ex (* constructor *)
  | Record of
      sid
      * (* is array *) bool
      * (('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr) list
  | Efun of ('ex, 'fb, 'en, 'hi) fun_ * lid list
  | Lfun of ('ex, 'fb, 'en, 'hi) fun_ * lid list
  | Xml of
      sid
      * ('ex, 'fb, 'en, 'hi) xhp_attribute list
      * ('ex, 'fb, 'en, 'hi) expr list
  | Callconv of Ast_defs.param_kind * ('ex, 'fb, 'en, 'hi) expr
  | Import of import_flavor * ('ex, 'fb, 'en, 'hi) expr
  | Collection of
      sid * 'hi collection_targ option * ('ex, 'fb, 'en, 'hi) afield list
      (** TODO: T38184446 Consolidate collections in AAST *)
  | BracedExpr of ('ex, 'fb, 'en, 'hi) expr
  | ParenthesizedExpr of ('ex, 'fb, 'en, 'hi) expr
  (* None of these constructors exist in the AST *)
  | Lplaceholder of pos
  | Fun_id of sid
  | Method_id of ('ex, 'fb, 'en, 'hi) expr * pstring
  | Method_caller of sid * pstring
      (** meth_caller('Class name', 'method name') *)
  | Smethod_id of sid * pstring
  | Pair of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr
  | Assert of ('ex, 'fb, 'en, 'hi) assert_expr
  | Typename of sid
  | PU_atom of string
  | PU_identifier of ('ex, 'fb, 'en, 'hi) class_id * pstring * pstring
  | Any

and ('ex, 'fb, 'en, 'hi) class_get_expr =
  | CGstring of pstring
  | CGexpr of ('ex, 'fb, 'en, 'hi) expr

(* These are "very special" constructs that we look for in, among
 * other places, terminality checks. invariant does not appear here
 * because it gets rewritten to If + AE_invariant_violation.
 *
 * TODO: get rid of assert_expr entirely in favor of rewriting to if
 * and noreturn *)
and ('ex, 'fb, 'en, 'hi) assert_expr = AE_assert of ('ex, 'fb, 'en, 'hi) expr

and ('ex, 'fb, 'en, 'hi) case =
  | Default of pos * ('ex, 'fb, 'en, 'hi) block
  | Case of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) block

and ('ex, 'fb, 'en, 'hi) catch = sid * lid * ('ex, 'fb, 'en, 'hi) block

and ('ex, 'fb, 'en, 'hi) field =
  ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr

and ('ex, 'fb, 'en, 'hi) afield =
  | AFvalue of ('ex, 'fb, 'en, 'hi) expr
  | AFkvalue of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr

and ('ex, 'fb, 'en, 'hi) xhp_attribute =
  | Xhp_simple of pstring * ('ex, 'fb, 'en, 'hi) expr
  | Xhp_spread of ('ex, 'fb, 'en, 'hi) expr

and is_variadic = bool

and ('ex, 'fb, 'en, 'hi) fun_param = {
  param_annotation: 'ex;
  param_type_hint: 'hi type_hint;
  param_is_variadic: is_variadic;
  param_pos: pos;
  param_name: string;
  param_expr: ('ex, 'fb, 'en, 'hi) expr option;
  param_callconv: Ast_defs.param_kind option;
  param_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  param_visibility: visibility option;
}

(** does function take varying number of args? *)
and ('ex, 'fb, 'en, 'hi) fun_variadicity =
  | FVvariadicArg of ('ex, 'fb, 'en, 'hi) fun_param
      (** PHP5.6 ...$args finishes the func declaration *)
  | FVellipsis of pos
      (** HH ... finishes the declaration; deprecate for ...$args? *)
  | FVnonVariadic  (** standard non variadic function *)

and ('ex, 'fb, 'en, 'hi) fun_ = {
  f_span: pos;
  f_annotation: 'en;
  f_mode: FileInfo.mode; [@visitors.opaque]
  f_ret: 'hi type_hint;
  f_name: sid;
  f_tparams: ('ex, 'fb, 'en, 'hi) tparam list;
  f_where_constraints: where_constraint list;
  f_variadic: ('ex, 'fb, 'en, 'hi) fun_variadicity;
  f_params: ('ex, 'fb, 'en, 'hi) fun_param list;
  f_body: ('ex, 'fb, 'en, 'hi) func_body;
  f_fun_kind: Ast_defs.fun_kind;
  f_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  f_file_attributes: ('ex, 'fb, 'en, 'hi) file_attribute list;
  f_external: bool;
      (** true if this declaration has no body because it is an
                         external function declaration (e.g. from an HHI file)*)
  f_namespace: nsenv;
  f_doc_comment: string option;
  f_static: bool;
}

(**
 * Naming has two phases and the annotation helps to indicate the phase.
 * In the first pass, it will perform naming on everything except for function
 * and method bodies and collect information needed. Then, another round of
 * naming is performed where function bodies are named. Thus, naming will
 * have named and unnamed variants of the annotation.
 * See BodyNamingAnnotation in nast.ml and the comment in naming.ml
 *)
and ('ex, 'fb, 'en, 'hi) func_body = {
  fb_ast: ('ex, 'fb, 'en, 'hi) block;
  fb_annotation: 'fb;
}

(** A type annotation is two things:
  - the localized hint, or if the hint is missing, the inferred type
  - The typehint associated to this expression if it exists *)
and 'hi type_hint = 'hi * type_hint_

(** Explicit type argument to function, constructor, or collection literal.
 * 'hi = unit in NAST
 * 'hi = Typing_defs.(locl ty) in TAST,
 * and is used to record inferred type arguments, with wildcard hint.
 *)
and 'hi targ = 'hi * hint

and type_hint_ = hint option

and ('ex, 'fb, 'en, 'hi) user_attribute = {
  ua_name: sid;
  ua_params: ('ex, 'fb, 'en, 'hi) expr list;
      (** user attributes are restricted to scalar values *)
}

and ('ex, 'fb, 'en, 'hi) file_attribute = {
  fa_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  fa_namespace: nsenv;
}

and ('ex, 'fb, 'en, 'hi) tparam = {
  tp_variance: Ast_defs.variance;
  tp_name: sid;
  tp_constraints: (Ast_defs.constraint_kind * hint) list;
  tp_reified: reify_kind;
  tp_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
}

and ('ex, 'fb, 'en, 'hi) class_tparams = {
  c_tparam_list: ('ex, 'fb, 'en, 'hi) tparam list;
  c_tparam_constraints:
    (reify_kind * (Ast_defs.constraint_kind * hint) list) SMap.t;
      [@visitors.opaque]
      (** keeping around the ast version of the constraint only
       * for the purposes of Naming.class_meth_bodies
       * TODO: remove this and use tp_constraints *)
}

and use_as_alias = sid option * pstring * sid option * use_as_visibility list

and insteadof_alias = sid * pstring * sid list

and is_extends = bool

and emit_id =
  (* For globally defined type, the ID used in the .main function. *)
  | Emit_id of int
  (* Closures are hoisted to classes, but they don't get an entry in .main. *)
  | Anonymous

and ('ex, 'fb, 'en, 'hi) class_ = {
  c_span: pos;
  c_annotation: 'en;
  c_mode: FileInfo.mode; [@visitors.opaque]
  c_final: bool;
  c_is_xhp: bool;
  c_has_xhp_keyword: bool;
  c_kind: Ast_defs.class_kind;
  c_name: sid;
  c_tparams: ('ex, 'fb, 'en, 'hi) class_tparams;
      (** The type parameters of a class A<T> (T is the parameter) *)
  c_extends: hint list;
  c_uses: hint list;
  c_use_as_alias: use_as_alias list;
  c_insteadof_alias: insteadof_alias list;
  c_method_redeclarations: ('ex, 'fb, 'en, 'hi) method_redeclaration list;
  c_xhp_attr_uses: hint list;
  c_xhp_category: (pos * pstring list) option;
  c_reqs: (hint * is_extends) list;
  c_implements: hint list;
  c_where_constraints: where_constraint list;
  c_consts: ('ex, 'fb, 'en, 'hi) class_const list;
  c_typeconsts: ('ex, 'fb, 'en, 'hi) class_typeconst list;
  c_vars: ('ex, 'fb, 'en, 'hi) class_var list;
  c_methods: ('ex, 'fb, 'en, 'hi) method_ list;
  c_attributes: ('ex, 'fb, 'en, 'hi) class_attr list;
  c_xhp_children: (pos * xhp_child) list;
  c_xhp_attrs: ('ex, 'fb, 'en, 'hi) xhp_attr list;
  c_namespace: nsenv;
  c_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  c_file_attributes: ('ex, 'fb, 'en, 'hi) file_attribute list;
  c_enum: enum_ option;
  c_pu_enums: ('ex, 'fb, 'en, 'hi) pu_enum list;
  c_doc_comment: string option;
  c_emit_id: emit_id option;
}

and xhp_attr_tag =
  | Required
  | LateInit

and ('ex, 'fb, 'en, 'hi) xhp_attr =
  'hi type_hint
  * ('ex, 'fb, 'en, 'hi) class_var
  * xhp_attr_tag option
  * (pos * bool * ('ex, 'fb, 'en, 'hi) expr list) option

and ('ex, 'fb, 'en, 'hi) class_attr =
  | CA_name of sid
  | CA_field of ('ex, 'fb, 'en, 'hi) ca_field

and ('ex, 'fb, 'en, 'hi) ca_field = {
  ca_type: ca_type;
  ca_id: sid;
  ca_value: ('ex, 'fb, 'en, 'hi) expr option;
  ca_required: bool;
}

and ca_type =
  | CA_hint of hint
  | CA_enum of string list

and ('ex, 'fb, 'en, 'hi) class_const = {
  cc_type: hint option;
  cc_id: sid;
  cc_expr: ('ex, 'fb, 'en, 'hi) expr option;
      (** expr = None indicates an abstract const *)
  cc_doc_comment: string option;
}

and typeconst_abstract_kind =
  | TCAbstract of hint option (* default *)
  | TCPartiallyAbstract
  | TCConcrete

(** This represents a type const definition. If a type const is abstract then
 * then the type hint acts as a constraint. Any concrete definition of the
 * type const must satisfy the constraint.
 *
 * If the type const is not abstract then a type must be specified.
 *)
and ('ex, 'fb, 'en, 'hi) class_typeconst = {
  c_tconst_abstract: typeconst_abstract_kind;
  c_tconst_name: sid;
  c_tconst_constraint: hint option;
  c_tconst_type: hint option;
  c_tconst_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  c_tconst_span: pos;
  c_tconst_doc_comment: string option;
}

and xhp_attr_info = { xai_tag: xhp_attr_tag option }

and ('ex, 'fb, 'en, 'hi) class_var = {
  cv_final: bool;
  cv_xhp_attr: xhp_attr_info option;
  cv_abstract: bool;
  cv_visibility: visibility;
  cv_type: 'hi type_hint;
  cv_id: sid;
  cv_expr: ('ex, 'fb, 'en, 'hi) expr option;
  cv_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  cv_doc_comment: string option;
  cv_is_promoted_variadic: bool;
  cv_is_static: bool;
  cv_span: pos;
}

and ('ex, 'fb, 'en, 'hi) method_ = {
  m_span: pos;
  m_annotation: 'en;
  m_final: bool;
  m_abstract: bool;
  m_static: bool;
  m_visibility: visibility;
  m_name: sid;
  m_tparams: ('ex, 'fb, 'en, 'hi) tparam list;
  m_where_constraints: where_constraint list;
  m_variadic: ('ex, 'fb, 'en, 'hi) fun_variadicity;
  m_params: ('ex, 'fb, 'en, 'hi) fun_param list;
  m_body: ('ex, 'fb, 'en, 'hi) func_body;
  m_fun_kind: Ast_defs.fun_kind;
  m_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  m_ret: 'hi type_hint;
  m_external: bool;
      (** true if this declaration has no body because it is an external method
          declaration (e.g. from an HHI file) *)
  m_doc_comment: string option;
}

and ('ex, 'fb, 'en, 'hi) method_redeclaration = {
  mt_final: bool;
  mt_abstract: bool;
  mt_static: bool;
  mt_visibility: visibility;
  mt_name: sid;
  mt_tparams: ('ex, 'fb, 'en, 'hi) tparam list;
  mt_where_constraints: where_constraint list;
  mt_variadic: ('ex, 'fb, 'en, 'hi) fun_variadicity;
  mt_params: ('ex, 'fb, 'en, 'hi) fun_param list;
  mt_fun_kind: Ast_defs.fun_kind;
  mt_ret: 'hi type_hint;
  mt_trait: hint;
  mt_method: pstring;
  mt_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
}

and nsenv = (Namespace_env.env[@visitors.opaque])

and ('ex, 'fb, 'en, 'hi) typedef = {
  t_annotation: 'en;
  t_name: sid;
  t_tparams: ('ex, 'fb, 'en, 'hi) tparam list;
  t_constraint: hint option;
  t_kind: hint;
  t_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  t_mode: FileInfo.mode; [@visitors.opaque]
  t_vis: typedef_visibility;
  t_namespace: nsenv;
  t_emit_id: emit_id option;
}

and ('ex, 'fb, 'en, 'hi) gconst = {
  cst_annotation: 'en;
  cst_mode: FileInfo.mode; [@visitors.opaque]
  cst_name: sid;
  cst_type: hint option;
  cst_value: ('ex, 'fb, 'en, 'hi) expr;
  cst_namespace: nsenv;
  cst_span: pos;
  cst_emit_id: emit_id option;
}

and ('ex, 'fb, 'en, 'hi) record_def = {
  rd_annotation: 'en;
  rd_name: sid;
  rd_extends: hint option;
  rd_abstract: bool;
  rd_fields: (sid * hint * ('ex, 'fb, 'en, 'hi) expr option) list;
  rd_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  rd_namespace: nsenv;
  rd_span: pos;
  rd_doc_comment: string option;
  rd_emit_id: emit_id option;
}

(** Pocket Universe Enumeration, e.g.

```
   enum Foo { // pu_name
     // pu_case_types
     case type T0;
     case type T1;

     // pu_case_values
     case ?T0 default_value;
     case T1 foo;

     // pu_members
     :@A( // pum_atom
       // pum_types
       type T0 = string,
       type T1 = int,

       // pum_exprs
       default_value = null,
       foo = 42,
     );
     :@B( ... )
     ...
   }
```
*)
and ('ex, 'fb, 'en, 'hi) pu_enum = {
  pu_annotation: 'en;
  pu_name: sid;
  pu_is_final: bool;
  pu_case_types: (sid * reify_kind) list;
  pu_case_values: (sid * hint) list;
  pu_members: ('ex, 'fb, 'en, 'hi) pu_member list;
}

and ('ex, 'fb, 'en, 'hi) pu_member = {
  pum_atom: sid;
  pum_types: (sid * hint) list;
  pum_exprs: (sid * ('ex, 'fb, 'en, 'hi) expr) list;
}

and ('ex, 'fb, 'en, 'hi) fun_def = ('ex, 'fb, 'en, 'hi) fun_

and ('ex, 'fb, 'en, 'hi) def =
  | Fun of ('ex, 'fb, 'en, 'hi) fun_def
  | Class of ('ex, 'fb, 'en, 'hi) class_
  | RecordDef of ('ex, 'fb, 'en, 'hi) record_def
  | Stmt of ('ex, 'fb, 'en, 'hi) stmt
  | Typedef of ('ex, 'fb, 'en, 'hi) typedef
  | Constant of ('ex, 'fb, 'en, 'hi) gconst
  | Namespace of sid * ('ex, 'fb, 'en, 'hi) program
  | NamespaceUse of (ns_kind * sid * sid) list
  | SetNamespaceEnv of nsenv
  | FileAttributes of ('ex, 'fb, 'en, 'hi) file_attribute

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

(* Splits the methods on a class into the constructor, statics, dynamics *)

(**
 * Methods, properties, and requirements are order dependent in bytecode
 * emission, which is observable in user code via `ReflectionClass`.
 *)
let split_methods class_ =
  let (constr, statics, res) =
    List.fold_left
      (fun (constr, statics, rest) m ->
        if snd m.m_name = "__construct" then
          (Some m, statics, rest)
        else if m.m_static then
          (constr, m :: statics, rest)
        else
          (constr, statics, m :: rest))
      (None, [], [])
      class_.c_methods
  in
  (constr, List.rev statics, List.rev res)

(* Splits class properties into statics, dynamics *)
let split_vars class_ =
  let (statics, res) =
    List.fold_left
      (fun (statics, rest) v ->
        if v.cv_is_static then
          (v :: statics, rest)
        else
          (statics, v :: rest))
      ([], [])
      class_.c_vars
  in
  (List.rev statics, List.rev res)

(* Splits `require`s into extends, implements *)
let split_reqs class_ =
  let (extends, implements) =
    List.fold_left
      (fun (extends, implements) (h, is_extends) ->
        if is_extends then
          (h :: extends, implements)
        else
          (extends, h :: implements))
      ([], [])
      class_.c_reqs
  in
  (List.rev extends, List.rev implements)

type break_continue_level =
  | Level_ok of int option
  | Level_non_literal
  | Level_non_positive

let get_break_continue_level level_opt =
  match level_opt with
  | (_, Int s) ->
    let i = int_of_string s in
    if i <= 0 then
      Level_non_positive
    else
      Level_ok (Some i)
  | _ -> Level_non_literal
  | exception _ -> Level_non_literal

(* map a function over the second part of a type annotation *)
let type_hint_option_map ~f ta =
  let mapped_hint =
    match snd ta with
    | Some hint -> Some (f hint)
    | None -> None
  in
  (fst ta, mapped_hint)

(* extract an hint from a type annotation *)
let hint_of_type_hint = snd
