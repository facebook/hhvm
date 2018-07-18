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
  module ClassIdAnnotation : AnnotationType
end

module AnnotatedAST(Annotations: ASTAnnotationTypes) =
struct

module ExprAnnotation = Annotations.ExprAnnotation
module EnvAnnotation = Annotations.EnvAnnotation
module ClassIdAnnotation = Annotations.ClassIdAnnotation

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
and class_id_annotation = ClassIdAnnotation.t [@visitors.opaque]

and stmt =
  | Fallthrough
  | Expr of expr
  (* AST has Block of block *)
  | Break of pos
  | Continue of pos
  (* is_terminal is new *)
  | Throw of is_terminal * expr
  | Return of pos * expr option
  | GotoLabel of pstring
  | Goto of pstring
  | Static_var of expr list
  | Global_var of expr list
  | If of expr * block * block
  | Do of block * expr
  | While of expr * block
  | Using of bool (* await? *) * expr * block
  | For of expr * expr * expr * block
  | Switch of expr * case list
  (* Dropped the Pos.t option *)
  | Foreach of expr * as_expr * block
  | Try of block * catch list * block
  | Let of lid * hint option * expr
  | Noop

and as_expr =
  | As_v of expr
  | As_kv of expr * expr
  (* This is not in AST *)
  | Await_as_v of pos * expr
  | Await_as_kv of pos * expr * expr

and block = stmt list

(* This is not in AST *)
and class_id = class_id_annotation * class_id_
and class_id_ =
  | CIparent
  | CIself
  | CIstatic
  | CIexpr of expr
  | CI of instantiated_sid

and expr = expr_annotation * expr_
and expr_ =
  | Array of afield list
  | Darray of (expr * expr) list
  | Varray of expr list
  (* This is more abstract than the AST but forgets evaluation order *)
  | Shape of expr shape_map
  | ValCollection of vc_kind * expr list
  | KeyValCollection of kvc_kind * field list
  | Null
  | This
  | True
  | False
  | Id of sid
  | Lvar of lid
  | ImmutableVar of lid
  | Dollar of expr
  | Dollardollar of lid
  | Clone of expr
  | Obj_get of expr * expr * og_null_flavor
  | Array_get of expr * expr option
  | Class_get of class_id * pstring
  | Class_const of class_id * pstring
  | Call of call_type
    * expr (* function *)
    * hint list (* explicit type annotations *)
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
  | New of class_id * expr list * expr list
  | Efun of fun_ * lid list
  | Xml of sid * xhp_attribute list * expr list
  | Callconv of Ast.param_kind * expr

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
  | Any

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
  f_ret_by_ref : bool;
}

and func_body =
  | UnnamedBody of func_unnamed_body
  | NamedBody of func_named_body

and func_unnamed_body = {
  (* Unnamed AST for the function body *)
  fub_ast       : Ast.block [@opaque];
  (* Unnamed AST for the function type params *)
  fub_tparams   : Ast.tparam list [@opaque];
  (* Namespace info *)
  fub_namespace : Namespace_env.env [@opaque];
}

and func_named_body = {
  (* Named AST for the function body *)
  fnb_nast     : block;
  (* True if there are any UNSAFE blocks; the presence of any unsafe
   * block in the function makes comparing the function body to the
   * declared return type impossible, since that block could return;
   * functions declared in Mdecl are by definition UNSAFE
   *)
  fnb_unsafe   : bool;
}

and user_attribute = {
  ua_name: sid;
  ua_params: expr list (* user attributes are restricted to scalar values *)
}

and static_var = class_var
and static_method = method_
and constructor = method_

and class_ = {
  c_annotation     : env_annotation   ;
  c_mode           : FileInfo.mode [@opaque];
  c_final          : bool             ;
  c_is_xhp         : bool;
  c_kind           : Ast.class_kind   ;
  c_name           : sid              ;
  (* The type parameters of a class A<T> (T is the parameter) *)
  c_tparams :
    tparam list *
    (* keeping around the ast version of the constraint only
     * for the purposes of Naming.class_meth_bodies *)
    ((Ast.constraint_kind * Ast.hint) list SMap.t) [@opaque];
  c_extends        : hint list        ;
  c_uses           : hint list        ;
  c_xhp_attr_uses  : hint list        ;
  c_xhp_category   : pstring list     ;
  c_req_extends    : hint list        ;
  c_req_implements : hint list        ;
  c_implements     : hint list        ;
  c_consts         : class_const list ;
  c_typeconsts     : class_typeconst list   ;
  c_static_vars    : static_var list  ;
  c_vars           : class_var list   ;
  c_constructor    : constructor option;
  c_static_methods : static_method list;
  c_methods        : method_ list     ;
  c_user_attributes : user_attribute list;
  c_enum           : enum_ option     ;
}


(* expr = None indicates an abstract const *)
and class_const = hint option * sid * expr option

(* This represents a type const definition. If a type const is abstract then
 * then the type hint acts as a constraint. Any concrete definition of the
 * type const must satisfy the constraint.
 *
 * If the type const is not abstract then a type must be specified.
 *)
and class_typeconst = {
  c_tconst_name : sid;
  c_tconst_constraint : hint option;
  c_tconst_type : hint option;
}

and class_var = {
  cv_final           : bool               ;
  cv_is_xhp          : bool               ;
  cv_visibility      : visibility         ;
  cv_type            : hint option        ;
  cv_id              : sid                ;
  cv_expr            : expr option        ;
  cv_user_attributes : user_attribute list;
}

and method_ = {
  m_annotation      : env_annotation      ;
  m_final           : bool                ;
  m_abstract        : bool                ;
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
  m_ret_by_ref      : bool                ;
}

and typedef = {
  t_annotation : env_annotation;
  t_name : sid;
  t_tparams : tparam list;
  t_constraint : hint option;
  t_kind : hint;
  t_user_attributes : user_attribute list;
  t_mode : FileInfo.mode [@opaque];
  t_vis : typedef_visibility;
}

and gconst = {
  cst_annotation : env_annotation;
  cst_mode: FileInfo.mode [@opaque];
  cst_name: sid;
  cst_type: hint option;
  cst_value: expr option;
  cst_is_define: bool;
}

and fun_def = fun_

and def =
  | Fun of fun_def
  | Class of class_
  | Typedef of typedef
  | Constant of gconst

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
  | Dollar _ -> "Dollar"
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
  | Efun _  -> "Efun"
  | Xml _  -> "Xml"
  | Callconv _ -> "Callconv"
  | Assert _  -> "Assert"
  | Clone _  -> "Clone"
  | Typename _  -> "Typename"

(*****************************************************************************)
(* This module defines a visitor class on the Nast data structure.
 * To use it you must inherit the generic object and redefine the appropriate
 * methods.
 *)
(*****************************************************************************)

module Visitor = struct

(*****************************************************************************)
(* The signature of the visitor. *)
(*****************************************************************************)

class type ['a] visitor_type = object
  method on_block : 'a -> block -> 'a
  method on_break : 'a -> Pos.t -> 'a
  method on_case : 'a -> case -> 'a
  method on_catch : 'a -> catch -> 'a
  method on_continue : 'a -> Pos.t -> 'a
  method on_do : 'a -> block -> expr -> 'a
  method on_expr : 'a -> expr -> 'a
  method on_expr_ : 'a -> expr_ -> 'a
  method on_for :
      'a -> expr -> expr -> expr -> block -> 'a
  method on_foreach :
      'a -> expr -> as_expr -> block -> 'a
  method on_if : 'a -> expr -> block -> block -> 'a
  method on_noop : 'a -> 'a
  method on_fallthrough : 'a -> 'a
  method on_return : 'a -> Pos.t -> expr option -> 'a
  method on_goto_label : 'a -> pstring -> 'a
  method on_goto : 'a -> pstring -> 'a
  method on_static_var : 'a -> expr list -> 'a
  method on_global_var : 'a -> expr list -> 'a
  method on_stmt : 'a -> stmt -> 'a
  method on_switch : 'a -> expr -> case list -> 'a
  method on_throw : 'a -> is_terminal -> expr -> 'a
  method on_try : 'a -> block -> catch list -> block -> 'a
  method on_let : 'a -> id -> hint option -> expr -> 'a
  method on_while : 'a -> expr -> block -> 'a
  method on_using : 'a -> bool -> expr -> block -> 'a
  method on_as_expr : 'a -> as_expr -> 'a
  method on_array : 'a -> afield list -> 'a
  method on_shape : 'a -> expr ShapeMap.t -> 'a
  method on_valCollection : 'a -> vc_kind -> expr list -> 'a
  method on_keyValCollection : 'a -> kvc_kind -> field list -> 'a
  method on_this : 'a -> 'a
  method on_id : 'a -> sid -> 'a
  method on_lvar : 'a -> id -> 'a
  method on_immutablevar : 'a -> id -> 'a
  method on_dollar : 'a -> expr -> 'a
  method on_dollardollar : 'a -> id -> 'a
  method on_fun_id : 'a -> sid -> 'a
  method on_method_id : 'a -> expr -> pstring -> 'a
  method on_smethod_id : 'a -> sid -> pstring -> 'a
  method on_method_caller : 'a -> sid -> pstring -> 'a
  method on_obj_get : 'a -> expr -> expr -> 'a
  method on_array_get : 'a -> expr -> expr option -> 'a
  method on_class_get : 'a -> class_id -> pstring -> 'a
  method on_class_const : 'a -> class_id -> pstring -> 'a
  method on_call : 'a -> call_type -> expr -> expr list -> expr list -> 'a
  method on_true : 'a -> 'a
  method on_false : 'a -> 'a
  method on_int : 'a -> string -> 'a
  method on_float : 'a -> string -> 'a
  method on_null : 'a -> 'a
  method on_string : 'a -> string -> 'a
  method on_string2 : 'a -> expr list -> 'a
  method on_special_func : 'a -> special_func -> 'a
  method on_yield_break : 'a -> 'a
  method on_yield : 'a -> afield -> 'a
  method on_yield_from : 'a -> expr -> 'a
  method on_await : 'a -> expr -> 'a
  method on_suspend : 'a -> expr -> 'a
  method on_list : 'a -> expr list -> 'a
  method on_pair : 'a -> expr -> expr -> 'a
  method on_expr_list : 'a -> expr list -> 'a
  method on_cast : 'a -> hint -> expr -> 'a
  method on_unop : 'a -> Ast.uop -> expr -> 'a
  method on_binop : 'a -> Ast.bop -> expr -> expr -> 'a
  method on_pipe : 'a -> id -> expr -> expr -> 'a
  method on_eif : 'a -> expr -> expr option -> expr -> 'a
  method on_typename : 'a -> sid -> 'a
  method on_instanceOf : 'a -> expr -> class_id -> 'a
  method on_is : 'a -> expr -> hint -> 'a
  method on_as : 'a -> expr -> hint -> bool -> 'a
  method on_class_id : 'a -> class_id -> 'a
  method on_class_id_ : 'a -> class_id_ -> 'a
  method on_new : 'a -> class_id -> expr list -> expr list -> 'a
  method on_efun : 'a -> fun_ -> id list -> 'a
  method on_xml : 'a -> sid -> xhp_attribute list -> expr list -> 'a
  method on_param_kind : 'a -> Ast.param_kind -> 'a
  method on_callconv : 'a -> Ast.param_kind -> expr -> 'a
  method on_assert : 'a -> assert_expr -> 'a
  method on_clone : 'a -> expr -> 'a
  method on_field: 'a -> field -> 'a
  method on_afield: 'a -> afield -> 'a

  method on_func_named_body: 'a -> func_named_body -> 'a
  method on_func_unnamed_body: 'a -> func_unnamed_body -> 'a
  method on_func_body: 'a -> func_body -> 'a
  method on_method_: 'a -> method_ -> 'a

  method on_fun_: 'a -> fun_ -> 'a
  method on_class_: 'a -> class_ -> 'a
  method on_gconst: 'a -> gconst -> 'a
  method on_typedef: 'a -> typedef -> 'a

  method on_hint: 'a -> hint -> 'a

  method on_def: 'a -> def -> 'a
  method on_program: 'a -> program -> 'a
end

(*****************************************************************************)
(* The generic visitor ('a is the type of the accumulator). *)
(*****************************************************************************)

class virtual ['a] visitor: ['a] visitor_type = object(this)

  method on_break acc _ = acc
  method on_continue acc _ = acc
  method on_noop acc = acc
  method on_fallthrough acc = acc
  method on_goto_label acc _ = acc
  method on_goto acc _ = acc

  method on_throw acc _ e =
    let acc = this#on_expr acc e in
    acc

  method on_return acc _ eopt =
    match eopt with
    | None -> acc
    | Some e -> this#on_expr acc e

  method on_static_var acc el = List.fold_left this#on_expr acc el

  method on_global_var acc el = List.fold_left this#on_expr acc el

  method on_if acc e b1 b2 =
    let acc = this#on_expr acc e in
    let acc = this#on_block acc b1 in
    let acc = this#on_block acc b2 in
    acc

  method on_do acc b e =
    let acc = this#on_block acc b in
    let acc = this#on_expr acc e in
    acc

  method on_while acc e b =
    let acc = this#on_expr acc e in
    let acc = this#on_block acc b in
    acc

  method on_using acc _has_await e b =
    let acc = this#on_expr acc e in
    let acc = this#on_block acc b in
    acc

  method on_for acc e1 e2 e3 b =
    let acc = this#on_expr acc e1 in
    let acc = this#on_expr acc e2 in
    let acc = this#on_expr acc e3 in
    let acc = this#on_block acc b in
    acc

  method on_switch acc e cl =
    let acc = this#on_expr acc e in
    let acc = List.fold_left this#on_case acc cl in
    acc

  method on_foreach acc e ae b =
    let acc = this#on_expr acc e in
    let acc = this#on_as_expr acc ae in
    let acc = this#on_block acc b in
    acc

  method on_try acc b cl fb =
    let acc = this#on_block acc b in
    let acc = List.fold_left this#on_catch acc cl in
    let acc = this#on_block acc fb in
    acc

  method on_let acc x h e =
    let acc = this#on_lvar acc x in
    let acc = match h with
      | Some h -> this#on_hint acc h
      | None -> acc in
    let acc = this#on_expr acc e in
    acc

  method on_block acc b =
    List.fold_left this#on_stmt acc b

  method on_case acc = function
    | Default b ->
        let acc = this#on_block acc b in
        acc
    | Case (e, b) ->
        let acc = this#on_expr acc e in
        let acc = this#on_block acc b in
        acc

  method on_as_expr acc = function
   | As_v e
   | Await_as_v (_, e) ->
       let acc = this#on_expr acc e in
       acc
   | As_kv (e1, e2)
   | Await_as_kv (_, e1, e2) ->
       let acc = this#on_expr acc e1 in
       let acc = this#on_expr acc e2 in
       acc

  method on_catch acc (_, _, b) = this#on_block acc b

  method on_stmt acc = function
    | Expr e                  -> this#on_expr acc e
    | Break p                 -> this#on_break acc p
    | Continue p              -> this#on_continue acc p
    | Throw   (is_term, e)    -> this#on_throw acc is_term e
    | Return  (p, eopt)       -> this#on_return acc p eopt
    | GotoLabel label         -> this#on_goto_label acc label
    | Goto label              -> this#on_goto acc label
    | If      (e, b1, b2)     -> this#on_if acc e b1 b2
    | Do      (b, e)          -> this#on_do acc b e
    | While   (e, b)          -> this#on_while acc e b
    | Using   (has_await, e, b) -> this#on_using acc has_await e b
    | For     (e1, e2, e3, b) -> this#on_for acc e1 e2 e3 b
    | Switch  (e, cl)         -> this#on_switch acc e cl
    | Foreach (e, ae, b)      -> this#on_foreach acc e ae b
    | Try     (b, cl, fb)     -> this#on_try acc b cl fb
    | Noop                    -> this#on_noop acc
    | Fallthrough             -> this#on_fallthrough acc
    | Static_var el           -> this#on_static_var acc el
    | Global_var el           -> this#on_global_var acc el
    | Let     (x, h, e)       -> this#on_let acc x h e

  method on_expr acc (_, e) =
    this#on_expr_ acc e

  method on_expr_ acc e =
    match e with
   | Any         -> acc
   | Array afl   -> this#on_array acc afl
   | Darray fieldl -> List.fold_left this#on_field acc fieldl
   | Varray el   -> List.fold_left this#on_expr acc el
   | Shape sh    -> this#on_shape acc sh
   | True        -> this#on_true acc
   | False       -> this#on_false acc
   | Int n       -> this#on_int acc n
   | Float n     -> this#on_float acc n
   | Null        -> this#on_null acc
   | String s    -> this#on_string acc s
   | This        -> this#on_this acc
   | Id sid      -> this#on_id acc sid
   | Lplaceholder _pos -> acc
   | Dollardollar id -> this#on_dollardollar acc id
   | Lvar id     -> this#on_lvar acc id
   | ImmutableVar id -> this#on_immutablevar acc id
   | Dollar e    -> this#on_dollar acc e
   | Fun_id sid  -> this#on_fun_id acc sid
   | Method_id (expr, pstr) -> this#on_method_id acc expr pstr
   | Method_caller (sid, pstr) -> this#on_method_caller acc sid pstr
   | Smethod_id (sid, pstr) -> this#on_smethod_id acc sid pstr
   | Yield_break -> this#on_yield_break acc
   | Yield e     -> this#on_yield acc e
   | Yield_from e -> this#on_yield_from acc e
   | Await e     -> this#on_await acc e
   | Suspend e   -> this#on_suspend acc e
   | List el     -> this#on_list acc el
   | Assert ae   -> this#on_assert acc ae
   | Clone e     -> this#on_clone acc e
   | Expr_list el    -> this#on_expr_list acc el
   | Special_func sf -> this#on_special_func acc sf
   | Obj_get     (e1, e2, _) -> this#on_obj_get acc e1 e2
   | Array_get   (e1, e2)    -> this#on_array_get acc e1 e2
   | Class_get   (cid, id)   -> this#on_class_get acc cid id
   | Class_const (cid, id)   -> this#on_class_const acc cid id
   | Call        (ct, e, _, el, uel) -> this#on_call acc ct e el uel
   | String2     el          -> this#on_string2 acc el
   | PrefixedString (_, e)   -> this#on_expr acc e
   | Pair        (e1, e2)    -> this#on_pair acc e1 e2
   | Cast        (hint, e)   -> this#on_cast acc hint e
   | Unop        (uop, e)         -> this#on_unop acc uop e
   | Binop       (bop, e1, e2)    -> this#on_binop acc bop e1 e2
   | Pipe        (id, e1, e2)         -> this#on_pipe acc id e1 e2
   | Eif         (e1, e2, e3)     -> this#on_eif acc e1 e2 e3
   | InstanceOf  (e1, e2)         -> this#on_instanceOf acc e1 e2
   | Is          (e, h)           -> this#on_is acc e h
   | As          (e, h, b)           -> this#on_as acc e h b
   | Typename n -> this#on_typename acc n
   | New         (cid, el, uel)   -> this#on_new acc cid el uel
   | Efun        (f, idl)         -> this#on_efun acc f idl
   | Xml         (sid, attrl, el) -> this#on_xml acc sid attrl el
   | Callconv    (kind, e)        -> this#on_callconv acc kind e
   | ValCollection    (s, el)     ->
       this#on_valCollection acc s el
   | KeyValCollection (s, fl)     ->
       this#on_keyValCollection acc s fl

  method on_array acc afl =
    List.fold_left this#on_afield acc afl

  method on_shape acc sm =
    ShapeMap.fold begin fun _ e acc ->
      let acc = this#on_expr acc e in
      acc
    end sm acc

  method on_valCollection acc _ el =
    List.fold_left this#on_expr acc el

  method on_keyValCollection acc _ fieldl =
    List.fold_left this#on_field acc fieldl

  method on_this acc = acc
  method on_id acc _ = acc
  method on_lvar acc _ = acc
  method on_immutablevar acc _ = acc
  method on_dollardollar acc id =
    this#on_lvar acc id

  method on_fun_id acc _ = acc
  method on_method_id acc _ _ = acc
  method on_smethod_id acc _ _ = acc
  method on_method_caller acc _ _ = acc
  method on_typename acc _ = acc

  method on_obj_get acc e1 e2 =
    let acc = this#on_expr acc e1 in
    let acc = this#on_expr acc e2 in
    acc

  method on_array_get acc e e_opt =
    let acc = this#on_expr acc e in
    let acc =
      match e_opt with
      | None -> acc
      | Some e -> this#on_expr acc e
    in
    acc

  method on_class_get acc cid _ = this#on_class_id acc cid

  method on_class_const acc cid _ = this#on_class_id acc cid

  method on_call acc _ e el uel =
    let acc = this#on_expr acc e in
    let acc = List.fold_left this#on_expr acc el in
    let acc = List.fold_left this#on_expr acc uel in
    acc

  method on_true acc = acc
  method on_false acc = acc
  method on_int acc _ = acc
  method on_float acc _ = acc
  method on_null acc = acc
  method on_string acc _ = acc

  method on_string2 acc el =
    let acc = List.fold_left this#on_expr acc el in
    acc

  method on_special_func acc = function
    | Gena e
    | Gen_array_rec e -> this#on_expr acc e
    | Genva el -> List.fold_left this#on_expr acc el

  method on_yield_break acc = acc
  method on_yield acc e = this#on_afield acc e
  method on_yield_from acc e = this#on_expr acc e
  method on_await acc e = this#on_expr acc e
  method on_dollar acc e = this#on_expr acc e
  method on_suspend acc e = this#on_expr acc e
  method on_list acc el = List.fold_left this#on_expr acc el

  method on_pair acc e1 e2 =
    let acc = this#on_expr acc e1 in
    let acc = this#on_expr acc e2 in
    acc

  method on_expr_list acc el =
    let acc = List.fold_left this#on_expr acc el in
    acc

  method on_cast acc _ e = this#on_expr acc e
  method on_unop acc _ e = this#on_expr acc e

  method on_binop acc _ e1 e2 =
    let acc = this#on_expr acc e1 in
    let acc = this#on_expr acc e2 in
    acc

  method on_pipe acc _id e1 e2 =
    let acc = this#on_expr acc e1 in
    let acc = this#on_expr acc e2 in
    acc

  method on_eif acc e1 e2 e3 =
    let acc = this#on_expr acc e1 in
    let acc =
      match e2 with
      | None -> acc
      | Some e -> this#on_expr acc e
    in
    let acc = this#on_expr acc e3 in
    acc

  method on_instanceOf acc e1 e2 =
    let acc = this#on_expr acc e1 in
    let acc = this#on_class_id acc e2 in
    acc

  method on_is acc e _ = this#on_expr acc e

  method on_as acc e _ _ = this#on_expr acc e

  method on_class_id acc (_, cid) = this#on_class_id_ acc cid

  method on_class_id_ acc = function
    | CIexpr e -> this#on_expr acc e
    | _ -> acc

  method on_new acc cid el uel =
    let acc = this#on_class_id acc cid in
    let acc = List.fold_left this#on_expr acc el in
    let acc = List.fold_left this#on_expr acc uel in
    acc

  method on_efun acc f _ = match f.f_body with
    | UnnamedBody _ ->
      failwith "lambdas expected to be named in the context of the surrounding function"
    | NamedBody { fnb_nast = n; _ } -> this#on_block acc n

  method on_xml acc _ attrl el =
    let acc = List.fold_left begin fun acc attr -> match attr with
      | Xhp_simple (_, e)
      | Xhp_spread e -> this#on_expr acc e
    end acc attrl in
    let acc = List.fold_left this#on_expr acc el in
    acc

  method on_param_kind acc _ = acc

  method on_callconv acc kind e =
    let acc = this#on_param_kind acc kind in
    let acc = this#on_expr acc e in
    acc

  method on_assert acc = function
    | AE_assert e -> this#on_expr acc e

  method on_clone acc e = this#on_expr acc e

  method on_field acc (e1, e2) =
    let acc = this#on_expr acc e1 in
    let acc = this#on_expr acc e2 in
    acc

  method on_afield acc = function
    | AFvalue e -> this#on_expr acc e
    | AFkvalue (e1, e2) ->
        let acc = this#on_expr acc e1 in
        let acc = this#on_expr acc e2 in
        acc

  method on_hint acc _ = acc

  method on_fun_ acc f =
    let acc = this#on_id acc f.f_name in
    let acc = this#on_func_body acc f.f_body in
    let acc = match f.f_ret with
      | Some h -> this#on_hint acc h
      | None -> acc in
    acc

  method on_func_named_body acc fnb =
    this#on_block acc fnb.fnb_nast

  method on_func_unnamed_body acc _ = acc

  method on_func_body acc = function
    | UnnamedBody unb -> this#on_func_unnamed_body acc unb
    | NamedBody nb -> this#on_func_named_body acc nb

  method on_method_ acc m =
    let acc = this#on_id acc m.m_name in
    let acc = this#on_func_body acc m.m_body in
    acc

  method on_class_ acc c =
    let acc = this#on_id acc c.c_name in
    let acc = List.fold_left this#on_hint acc c.c_extends in
    let acc = List.fold_left this#on_hint acc c.c_uses in
    let acc = List.fold_left this#on_hint acc c.c_implements in

    let acc = match c.c_constructor with
      | Some ctor -> this#on_method_ acc ctor
      | None -> acc in
    let acc = List.fold_left this#on_method_ acc c.c_methods in
    let acc = List.fold_left this#on_method_ acc c.c_static_methods in
    acc

  method on_gconst acc g =
    let acc = this#on_id acc g.cst_name in
    let acc = match g.cst_value with
      | Some e -> this#on_expr acc e
      | None -> acc in
    let acc = match g.cst_type with
      | Some h -> this#on_hint acc h
      | None -> acc in
    acc

  method on_typedef acc t =
    let acc = this#on_id acc t.t_name in
    let acc = this#on_hint acc t.t_kind in
    let acc = match t.t_constraint with
      | Some c -> this#on_hint acc c
      | None -> acc in
    acc

  method on_def acc = function
    | Fun f -> this#on_fun_ acc f
    | Class c -> this#on_class_ acc c
    | Typedef t -> this#on_typedef acc t
    | Constant g -> this#on_gconst acc g

  method on_program acc p =
    let acc = List.fold_left begin fun acc d ->
      this#on_def acc d end acc p in
    acc
end

(*****************************************************************************)
(* Returns true if a block has a return statement. *)
(*****************************************************************************)

module HasReturn: sig
  val block: block -> bool
end = struct

  let visitor =
    object
      inherit [bool] visitor
      method! on_expr acc _ = acc
      method! on_return _ _ _ = true
    end

  let block b = visitor#on_block false b

end

(* Used by HasBreak and HasContinue. Does not traverse nested loops, since the
 * breaks / continues in those loops do not affect the control flow of the
 * outermost loop. *)

class loop_visitor =
  object
    inherit [bool] visitor
    method! on_expr acc _ = acc
    method! on_for acc _ _ _ _ = acc
    method! on_foreach acc _ _ _ = acc
    method! on_do acc _ _ = acc
    method! on_while acc _ _ = acc
    method! on_switch acc _ _ = acc
  end

(*****************************************************************************)
(* Returns true if a block has a continue statement.
 * It is necessary to properly handle the type of locals.
 * When a block statement has a continue statement, the control flow graph
 * could be interrupted. When that is the case, the types of locals has to
 * be more conservative. Locals can have different types depending on their
 * position in a block. In the presence of constructions that can interrupt
 * the control flow (exceptions, continue), the type of the local becomes:
 * "any type that the local had, regardless of its position".
 *)
(*****************************************************************************)

module HasContinue: sig
  val block: block -> bool
end = struct

  let visitor =
    object
      inherit loop_visitor
      method! on_continue _ _ = true
    end

  let block b = visitor#on_block false b

end

(*****************************************************************************)
(* Returns true if a block has a continue statement.
 * Useful for checking if a while(true) {...} loop is non-terminating.
 *)
(*****************************************************************************)

module HasBreak: sig
  val block: block -> bool
end = struct

  let visitor =
    object
      inherit loop_visitor
      method! on_break _ _ = true
    end

  let block b = visitor#on_block false b

end

end (* of Visitor *)

end (* of AnnotatedAST functor *)
