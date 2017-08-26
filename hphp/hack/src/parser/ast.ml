(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

include Ast_defs

(*****************************************************************************)
(* The Abstract Syntax Tree *)
(*****************************************************************************)

type program = def list

and def =
  | Fun of fun_
  | Class of class_
  | Stmt of stmt
  | Typedef of typedef
  | Constant of gconst
  | Namespace of id * program
  | NamespaceUse of (ns_kind * id * id) list
  | SetNamespaceEnv of Namespace_env.env

and typedef = {
  t_id: id;
  t_tparams: tparam list;
  t_constraint: tconstraint;
  t_kind: typedef_kind;
  t_user_attributes: user_attribute list;
  t_namespace: Namespace_env.env;
  t_mode: FileInfo.mode;
}

and gconst = {
  cst_mode: FileInfo.mode;
  cst_kind: cst_kind;
  cst_name: id;
  cst_type: hint option;
  cst_value: expr;
  cst_namespace: Namespace_env.env;
}

and tparam = variance * id * (constraint_kind * hint) list

and tconstraint = hint option

and typedef_kind =
  | Alias of hint
  | NewType of hint

and class_ = {
  c_mode: FileInfo.mode;
  c_user_attributes: user_attribute list;
  c_final: bool;
  c_kind: class_kind;
  c_is_xhp: bool;
  c_name: id;
  c_tparams: tparam list;
  c_extends: hint list;
  c_implements: hint list;
  c_body: class_elt list;
  c_namespace: Namespace_env.env;
  c_enum: enum_ option;
  c_span: Pos.t;
  c_doc_comment : string option;
}

and enum_ = {
  e_base       : hint;
  e_constraint : hint option;
}

and user_attribute = {
  ua_name: id;
  ua_params: expr list (* user attributes are restricted to scalar values *)
}

and class_elt =
  | Const of hint option * (id * expr) list
  | AbsConst of hint option * id
  | Attributes of class_attr list
  | TypeConst of typeconst
  | ClassUse of hint
  (* as expressions *)
  | ClassUseAlias of id option * pstring * id option * kind option
  (* insteadof expressions *)
  | ClassUsePrecedence of id * pstring * id list
  | XhpAttrUse of hint
  | ClassTraitRequire of trait_req_kind * hint
  | ClassVars of kind list * hint option * class_var list * string option
  | XhpAttr of hint option * class_var * bool *
               ((Pos.t * expr list) option)
  | Method of method_
  | XhpCategory of pstring list
  | XhpChild of xhp_child

and xhp_child =
  | ChildName of id
  | ChildList of xhp_child list
  | ChildUnary of xhp_child * xhp_child_op
  | ChildBinary of xhp_child * xhp_child

and xhp_child_op = ChildStar | ChildPlus | ChildQuestion

and class_attr =
  | CA_name of id
  | CA_field of ca_field

and ca_field = {
  ca_type: ca_type;
  ca_id: id;
  ca_value: expr option;
  ca_required: bool;
}

and ca_type =
  | CA_hint of hint
  | CA_enum of string list

(* id is stored without the $ *)
(* Pos is the span of the the variable definition. What does it mean exactly?
 * At first, one might think that in a definition like:
 *
 *   public ?string $foo = "aaaa";
 *
 * span of "foo" is entire line from the start of "public" to the semicolon
 * (excluded). This is not true though - what here is a single '$foo = "aaaa"'
 * is in fact a list, and "public string" applies to all of it's elements.
 * So it could be as well:
 *
 *  public ?string
 *    $foo = "aaaa",
 *    $bar = "cccc",
 *    $i_have_no_initializer;
 *
 * which makes it hard to include visibility and type into span of $bar
 * (since span is a single continuos range).
 * To make things more complicated, there are also XHP properties, with syntax:
 *
 *  attributes
 *    string xhp_prop = "aaa" @required,
 *    int other_xhp_prop;
 *
 * where each type and "@required" applies only to one property and could be
 * part of span.
 *
 * Moreover, there is also the case of implicit properties defined in
 * constructor:
 *
 *  public function __construct(
 *    public string $property,
 *  )
 *
 * The visibility, type and possible initializer are "per property", but
 * capturing their whole span is annoying from implementation point of view
 * - constructor is just a regular method in AST with special name, so we would
 * have to store additional data for every method argument just to use it in
 * this single case.
 *
 * The "lowest common denominator" of all those cases is to treat the property
 * extent as span of name + initializer, if present.
 *)
and class_var = Pos.t * id * expr option

and method_ = {
  m_kind: kind list ;
  m_tparams: tparam list;
  m_constrs: (hint * constraint_kind * hint) list;
  m_name: id;
  m_params: fun_param list;
  m_body: block;
  m_user_attributes : user_attribute list;
  m_ret: hint option;
  m_ret_by_ref: bool;
  m_fun_kind: fun_kind;
  m_span: Pos.t;
  m_doc_comment: string option;
}

and typeconst = {
  tconst_abstract: bool;
  tconst_name: id;
  tconst_constraint: hint option;
  tconst_type: hint option;
  tconst_span: Pos.t;
}

and is_reference = bool
and is_variadic = bool

and fun_param = {
  param_hint : hint option;
  param_is_reference : is_reference;
  param_is_variadic : is_variadic;
  param_id : id;
  param_expr : expr option;
  (* implicit field via constructor parameter.
   * This is always None except for constructors and the modifier
   * can be only Public or Protected or Private.
   *)
  param_modifier: kind option;
  param_user_attributes: user_attribute list;
}

and fun_ = {
  f_mode            : FileInfo.mode;
  f_tparams         : tparam list;
  f_constrs         : (hint * constraint_kind * hint) list;
  f_ret             : hint option;
  f_ret_by_ref      : bool;
  f_name            : id;
  f_params          : fun_param list;
  f_body            : block;
  f_user_attributes : user_attribute list;
  f_fun_kind        : fun_kind;
  f_namespace       : Namespace_env.env;
  f_span            : Pos.t;
  f_doc_comment     : string option;
  f_static          : bool;
}

and hint = Pos.t * hint_
and hint_ =
  | Hoption of hint
  | Hfun of hint list * bool * hint
  | Htuple of hint list
  | Happly of id * hint list
  | Hshape of shape_info
 (* This represents the use of a type const. Type consts are accessed like
  * regular consts in Hack, i.e.
  *
  * Class::TypeConst
  *
  * Type const access can be chained such as
  *
  * Class::TC1::TC2::TC3
  *
  * This will result in the following representation
  *
  * Haccess ("Class", "TC1", ["TC2", "TC3"])
  *)
  | Haccess of id * id * id list
  | Hsoft of hint

and shape_info = {
  si_allows_unknown_fields : bool;
  si_shape_field_list : shape_field list;
}

and shape_field = {
  sf_optional : bool;
  sf_name : shape_field_name;
  sf_hint : hint;
}

and stmt =
  | Unsafe
  | Fallthrough
  | Expr of expr
  | Block of block
  | Break of Pos.t * int option
  | Continue of Pos.t * int option
  | Throw of expr
  | Return of Pos.t * expr option
  | GotoLabel of pstring
  | Goto of pstring
  | Static_var of expr list
  | Global_var of expr list
  | If of expr * block * block
  | Do of block * expr
  | While of expr * block
  | For of expr * expr * expr * block
  | Switch of expr * case list
  | Foreach of expr * Pos.t option (* await as *) * as_expr * block
  | Try of block * catch list * block
  | Def_inline of def
  | Noop
  | Markup of pstring * expr option

and as_expr =
  | As_v of expr
  | As_kv of expr * expr

and block = stmt list

and expr = Pos.t * expr_
and expr_ =
  | Array of afield list
  | Varray of expr list
  | Darray of (expr * expr) list
  | Shape of (shape_field_name * expr) list
  | Collection of id * afield list
  | Null
  | True
  | False
  | Omitted
  | Id of id
  | Id_type_arguments of id * hint list
  (* Special case: the pipe variable $$ *)
  | Lvar of id
  (**
   * PHP's Variable variable. The int is number of variable indirections
   * (i.e. number of extra $ signs.)
   *
   * Example:
     * $$$sample has int = 2.
   * *)
  | Lvarvar of int * id
  | Clone of expr
  | Obj_get of expr * expr * og_null_flavor
  | Array_get of expr * expr option
  | Class_get of id * expr
  | Class_const of id * pstring
  | Call of expr * hint list * expr list * expr list
  | Int of pstring
  | Float of pstring
  | String of pstring
  | String2 of expr list
  | Yield of afield
  | Yield_break
  | Yield_from of expr
  | Await of expr
  | List of expr list
  | Expr_list of expr list
  | Cast of hint * expr
  | Unop of uop * expr
  | Binop of bop * expr * expr
  | Pipe of expr * expr
  | Eif of expr * expr option * expr
  | NullCoalesce of expr * expr
  | InstanceOf of expr * expr
  | BracedExpr of expr
  | New of expr * expr list * expr list
  (* Traditional PHP-style closure with a use list. Each use element is
    a name and a bool indicating if its a reference or value *)
  | Efun of fun_ * (id * bool) list
  (*
   * Hack-style lambda expressions (no id list, we'll find the captures
   * during name resolution).
   *)
  | Lfun of fun_
  | Xml of id * (id * expr) list * expr list
  | Unsafeexpr of expr
  | Import of import_flavor * expr

and import_flavor =
  | Include
  | Require
  | IncludeOnce
  | RequireOnce

(** "array" field. Fields of array, map, dict, and shape literals. *)
and afield =
  | AFvalue of expr
  | AFkvalue of expr * expr

and case =
| Default of block
| Case of expr * block

and catch = id * id * block

and field = expr * expr

and attr = id * expr

 (* with tarzan *)

type any =
  | AHint of hint
  | AExpr of expr
  | AStmt of stmt
  | ADef of def
  | AProgram of program

 (* with tarzan *)
