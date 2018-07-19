(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Ast_defs

(*****************************************************************************)
(* The Abstract Syntax Tree *)
(*****************************************************************************)

type program = def list
[@@deriving show { with_path = false },
            visitors {
              variety = "endo";
              nude=true;
              visit_prefix="on_";
              ancestors=["endo_defs"];
            },
            visitors {
              variety = "reduce";
              nude=true;
              visit_prefix="on_";
              ancestors=["reduce_defs"];
            },
            visitors {
              variety = "map";
              nude=true;
              visit_prefix="on_";
              ancestors=["map_defs"];
            },
            visitors {
              variety = "iter";
              nude=true;
              visit_prefix="on_";
              ancestors=["iter_defs"];
            }]

and nsenv = Namespace_env.env [@opaque]
and fimode = FileInfo.mode [@visitors.opaque]

and def =
  | Fun of fun_
  | Class of class_
  | Stmt of stmt
  | Typedef of typedef
  | Constant of gconst
  | Namespace of id * program
  | NamespaceUse of (ns_kind * id * id) list
  | SetNamespaceEnv of nsenv

and cst_kind =
  (* The constant was introduced with: define('X', ...); *)
  | Cst_define
  (* The constant was introduced with: const X = ...; *)
  | Cst_const

and ns_kind =
  | NSNamespace
  | NSClass
  | NSClassAndNamespace
  | NSFun
  | NSConst

and trait_req_kind =
  | MustExtend
  | MustImplement

and kind =
  | Final
  | Static
  | Abstract
  | Private
  | Public
  | Protected

and typedef = {
  t_id: id;
  t_tparams: tparam list;
  t_constraint: tconstraint;
  t_kind: typedef_kind;
  t_user_attributes: user_attribute list;
  t_namespace: nsenv;
  t_mode: fimode;
}

and gconst = {
  cst_mode: fimode;
  cst_kind: cst_kind;
  cst_name: id;
  cst_type: hint option;
  cst_value: expr;
  cst_namespace: nsenv;
  cst_span: pos;
}

and targ = hint * reified

and tparam = variance * id * (constraint_kind * hint) list * reified

and tconstraint = hint option

and typedef_kind =
  | Alias of hint
  | NewType of hint

and class_ = {
  c_mode: fimode;
  c_user_attributes: user_attribute list;
  c_final: bool;
  c_kind: class_kind;
  c_is_xhp: bool;
  c_name: id;
  c_tparams: tparam list;
  c_extends: hint list;
  c_implements: hint list;
  c_body: class_elt list;
  c_namespace: nsenv;
  c_enum: enum_ option;
  c_span: pos;
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
  | ClassUseAlias of id option * pstring * id option * kind list
  (* insteadof expressions *)
  | ClassUsePrecedence of id * pstring * id list
  | XhpAttrUse of hint
  | ClassTraitRequire of trait_req_kind * hint
  | ClassVars of class_vars_
  | XhpAttr of hint option * class_var * bool *
               ((pos * bool * expr list) option)
  | Method of method_
  | XhpCategory of pos * (pstring list)
  | XhpChild of pos * xhp_child

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
and class_var = pos * id * expr option

and class_vars_ = {
  cv_kinds: kind list;
  cv_hint: hint option;
  cv_is_promoted_variadic: is_variadic;
  cv_names: class_var list;
  cv_doc_comment: string option;
  cv_user_attributes: user_attribute list;
}

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
  m_span: pos;
  m_doc_comment: string option;
}

and typeconst = {
  tconst_abstract: bool;
  tconst_name: id;
  tconst_tparams: tparam list;
  tconst_constraint: hint option;
  tconst_type: hint option;
  tconst_span: pos;
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
  param_callconv: param_kind option;
  param_user_attributes: user_attribute list;
}

and fun_ = {
  f_mode            : fimode;
  f_tparams         : tparam list;
  f_constrs         : (hint * constraint_kind * hint) list;
  f_ret             : hint option;
  f_ret_by_ref      : bool;
  f_name            : id;
  f_params          : fun_param list;
  f_body            : block;
  f_user_attributes : user_attribute list;
  f_fun_kind        : fun_kind;
  f_namespace       : nsenv;
  f_span            : pos;
  f_doc_comment     : string option;
  f_static          : bool;
}

and is_coroutine = bool
and hint = pos * hint_
and variadic_hint =
  | Hvariadic of hint option
  | Hnon_variadic
and hint_ =
  | Hoption of hint
  | Hfun of is_coroutine * hint list * param_kind option list * variadic_hint * hint
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

and using_stmt = {
  us_is_block_scoped: bool;
  us_has_await: bool;
  us_expr: expr;
  us_block: block;
}

and stmt = pos * stmt_
and stmt_ =
  | Unsafe
  | Fallthrough
  | Expr of expr
  | Block of block
  | Break of expr option
  | Continue of expr option
  | Throw of expr
  | Return of expr option
  | GotoLabel of pstring
  | Goto of pstring
  | Static_var of expr list
  | Global_var of expr list
  | If of expr * block * block
  | Do of block * expr
  | While of expr * block
  | For of expr * expr * expr * block
  | Switch of expr * case list
  | Foreach of expr * pos option (* await as *) * as_expr * block
  | Try of block * catch list * block
  | Def_inline of def
  | Noop
  | Markup of pstring * expr option
  | Using of using_stmt
  | Declare of (* is_block *) bool * expr * block
  | Let of id * hint option * expr

and as_expr =
  | As_v of expr
  | As_kv of expr * expr

and xhp_attribute =
  | Xhp_simple of id * expr
  | Xhp_spread of expr

and block = stmt list

and expr = pos * expr_
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
  (* Special case: the pipe variable $$ *)
  | Lvar of id
  (* General dollar expression e.g. ${"a" . $y}. Braced expression
   * is a separate node, so we can distinguish $f->${x} from $f->{x}
   * and $f->$x, each of which has different semantics. $$x is represented
   * by Dollar (Lvar "$x")
   *)
  | Dollar of expr
  | Clone of expr
  | Obj_get of expr * expr * og_null_flavor
  | Array_get of expr * expr option
  | Class_get of expr * expr
  | Class_const of expr * pstring
  | Call of expr * targ list * expr list * expr list
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
  | Unop of uop * expr
  | Binop of bop * expr * expr
  | Pipe of expr * expr
  | Eif of expr * expr option * expr
  | InstanceOf of expr * expr
  | Is of expr * hint
  | As of expr * hint * (* is nullable *) bool
  | BracedExpr of expr
  | ParenthesizedExpr of expr
  | New of expr * targ list * expr list * expr list
  | NewAnonClass of expr list * expr list * class_
  (* Traditional PHP-style closure with a use list. Each use element is
    a name and a bool indicating if its a reference or value *)
  | Efun of fun_ * (id * bool) list
  (*
   * Hack-style lambda expressions (no id list, we'll find the captures
   * during name resolution).
   *)
  | Lfun of fun_
  | Xml of id * xhp_attribute list * expr list
  | Unsafeexpr of expr
  | Import of import_flavor * expr
  | Callconv of param_kind * expr
  | Execution_operator of expr list

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

let string_of_kind = function
  | Final -> "final"
  | Static -> "static"
  | Abstract -> "abstract"
  | Private -> "private"
  | Public -> "public"
  | Protected -> "protected"
