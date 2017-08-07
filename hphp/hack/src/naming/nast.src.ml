(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SN = Naming_special_names

type id = Pos.t * Local_id.t [@@deriving show]
type sid = Ast.id [@@deriving show]
type pstring = Ast.pstring [@@deriving show]

type is_terminal = bool [@@deriving show]

type call_type =
  | Cnormal    (* when the call looks like f() *)
  | Cuser_func (* when the call looks like call_user_func(...) *)
  [@@deriving show]

type shape_field_name = Ast.shape_field_name

module ShapeMap = Ast.ShapeMap

type hint = Pos.t * hint_
and hint_ =
  | Hoption of hint
  | Hfun of hint list * bool * hint
  | Htuple of hint list
  | Happly of sid * hint list
  | Hshape of nast_shape_info

 (* This represents the use of a type const. Type consts are accessed like
  * regular consts in Hack, i.e.
  *
  * [self | static | Class]::TypeConst
  *
  * Class  => Happly "Class"
  * self   => Happly of the class of definition
  * static => Habstr ("static",
  *           Habstr ("this", (Constraint_as, Happly of class of definition)))
  * Type const access can be chained such as
  *
  * Class::TC1::TC2::TC3
  *
  * We resolve the root of the type access chain as a type as follows.
  *
  * This will result in the following representation
  *
  * Haccess (Happly "Class", ["TC1", "TC2", "TC3"])
  *)
  | Haccess of hint * sid list

  (* The following constructors don't exist in the AST hint type *)
  | Hany
  | Hmixed
  | Habstr of string
  | Harray of hint option * hint option
  | Hdarray of hint * hint
  | Hvarray of hint
  | Hdarray_or_varray of hint
  | Hprim of tprim
  | Hthis

(* AST types such as Happly("int", []) are resolved to Hprim values *)
and tprim =
  | Tvoid
  | Tint
  | Tbool
  | Tfloat
  | Tstring
  | Tresource
  | Tnum
  | Tarraykey
  | Tnoreturn

and shape_field_info = {
  sfi_optional: bool;
  sfi_hint : hint;
}

and nast_shape_info = {
  nsi_allows_unknown_fields : bool;
  nsi_field_map : shape_field_info ShapeMap.t;
}
[@@deriving show]

type og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe
  [@@deriving show]

type kvc_kind = [
  | `Map
  | `ImmMap
  | `Dict ]

  let pp_kvc_kind fmt _ = Format.pp_print_string fmt "<kvc_kind>"

type vc_kind = [
  | `Vector
  | `ImmVector
  | `Vec
  | `Set
  | `ImmSet
  | `Pair
  | `Keyset ]

let pp_vc_kind fmt _ = Format.pp_print_string fmt "<vc_kind>"

type tparam = Ast.variance * sid * (Ast.constraint_kind * hint) list [@@deriving show]

type visibility =
  | Private
  | Public
  | Protected
  [@@deriving show]

type typedef_visibility =
  | Transparent
  | Opaque
  [@@deriving show]

type enum_ = {
  e_base       : hint;
  e_constraint : hint option;
} [@@deriving show]


type instantiated_sid = sid * hint list [@@deriving show]

type where_constraint = hint * Ast.constraint_kind * hint [@@deriving show]

module type AnnotationType = sig
  type t
  val pp : Format.formatter -> t -> unit
end

module AnnotatedAST(Annotation: AnnotationType) = struct

type stmt =
  | Fallthrough
  | Expr of expr
  (* AST has Block of block *)
  | Break of Pos.t
  | Continue of Pos.t
  (* is_terminal is new *)
  | Throw of is_terminal * expr
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
  (* Dropped the Pos.t option *)
  | Foreach of expr * as_expr * block
  | Try of block * catch list * block
  | Noop

and as_expr =
  | As_v of expr
  | As_kv of expr * expr
  (* This is not in AST *)
  | Await_as_v of Pos.t * expr
  | Await_as_kv of Pos.t * expr * expr

and block = stmt list

(* This is not in AST *)
and class_id =
  | CIparent
  | CIself
  | CIstatic
  | CIexpr of expr
  | CI of instantiated_sid

and expr = Annotation.t * expr_
and expr_ =
  | Array of afield list
  | Darray of (expr * expr) list
  | Varray of expr list
  (* This is more abstract than the AST but forgets evaluation order *)
  | Shape of expr ShapeMap.t
  | ValCollection of vc_kind * expr list
  | KeyValCollection of kvc_kind * field list
  | Null
  | This
  | True
  | False
  (* TODO: to match AST we need Id_type_arguments as well *)
  | Id of sid
  | Lvar of id
  | Lvarvar of int * id
  | Dollardollar of id
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
  | Int of pstring
  | Float of pstring
  | String of pstring
  | String2 of expr list
  | Yield of afield
  | Yield_break
  | Await of expr
  | List of expr list
  | Expr_list of expr list
  | Cast of hint * expr
  | Unop of Ast.uop * expr
  | Binop of Ast.bop * expr * expr
  (** The ID of the $$ that is implicitly declared by this pipe. *)
  | Pipe of id * expr * expr
  | Eif of expr * expr option * expr
  | NullCoalesce of expr * expr
  | InstanceOf of expr * class_id
  | New of class_id * expr list * expr list
  | Efun of fun_ * id list
  | Xml of sid * (pstring * expr) list * expr list

  (* None of these constructors exist in the AST *)
  | Lplaceholder of Pos.t
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

and catch = sid * id * block

and field = expr * expr
and afield =
  | AFvalue of expr
  | AFkvalue of expr * expr

and special_func =
  | Gena of expr
  | Genva of expr list
  | Gen_array_rec of expr

and is_reference = bool
and is_variadic = bool
and fun_param = {
  param_hint : hint option;
  param_is_reference : is_reference;
  param_is_variadic : is_variadic;
  param_pos : Pos.t;
  param_name : string;
  param_expr : expr option;
}

and fun_variadicity = (* does function take varying number of args? *)
  | FVvariadicArg of fun_param (* PHP5.6 ...$args finishes the func declaration *)
  | FVellipsis    (* HH ... finishes the declaration; deprecate for ...$args? *)
  | FVnonVariadic (* standard non variadic function *)

and fun_ = {
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
} [@@deriving show]

type class_ = {
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
  c_static_vars    : class_var list   ;
  c_vars           : class_var list   ;
  c_constructor    : method_ option   ;
  c_static_methods : method_ list     ;
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
  cv_final      : bool        ;
  cv_is_xhp     : bool        ;
  cv_visibility : visibility  ;
  cv_type       : hint option ;
  cv_id         : sid         ;
  cv_expr       : expr option ;
}

and method_ = {
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
}

and typedef = {
  t_name : sid;
  t_tparams : tparam list;
  t_constraint : hint option;
  t_kind : hint;
  t_user_attributes : user_attribute list;
  t_mode : FileInfo.mode [@opaque];
  t_vis : typedef_visibility;
}

and gconst = {
  cst_mode: FileInfo.mode [@opaque];
  cst_name: sid;
  cst_type: hint option;
  cst_value: expr option;
}

[@@deriving show]

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
  | Lvarvar _ -> "Lvarvar"
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
  | Special_func _  -> "Special_func"
  | Yield_break -> "Yield_break"
  | Yield _  -> "Yield"
  | Await _  -> "Await"
  | List _  -> "List"
  | Pair _  -> "Pair"
  | Expr_list _  -> "Expr_list"
  | Cast _  -> "Cast"
  | Unop _  -> "Unop"
  | Binop _  -> "Binop"
  | Pipe _  -> "Pipe"
  | Eif _  -> "Eif"
  | NullCoalesce _  -> "NullCoalesce"
  | InstanceOf _  -> "InstanceOf"
  | New _  -> "New"
  | Efun _  -> "Efun"
  | Xml _  -> "Xml"
  | Assert _  -> "Assert"
  | Clone _  -> "Clone"
  | Typename _  -> "Typename"

type def =
  | Fun of fun_
  | Class of class_
  | Typedef of typedef
  | Constant of gconst
  [@@deriving show]

type program = def list [@@deriving show]

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
  method on_while : 'a -> expr -> block -> 'a
  method on_as_expr : 'a -> as_expr -> 'a
  method on_array : 'a -> afield list -> 'a
  method on_shape : 'a -> expr ShapeMap.t -> 'a
  method on_valCollection : 'a -> vc_kind -> expr list -> 'a
  method on_keyValCollection : 'a -> kvc_kind -> field list -> 'a
  method on_this : 'a -> 'a
  method on_id : 'a -> sid -> 'a
  method on_lvar : 'a -> id -> 'a
  method on_lvarvar : 'a -> int -> id -> 'a
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
  method on_int : 'a -> pstring -> 'a
  method on_float : 'a -> pstring -> 'a
  method on_null : 'a -> 'a
  method on_string : 'a -> pstring -> 'a
  method on_string2 : 'a -> expr list -> 'a
  method on_special_func : 'a -> special_func -> 'a
  method on_yield_break : 'a -> 'a
  method on_yield : 'a -> afield -> 'a
  method on_await : 'a -> expr -> 'a
  method on_list : 'a -> expr list -> 'a
  method on_pair : 'a -> expr -> expr -> 'a
  method on_expr_list : 'a -> expr list -> 'a
  method on_cast : 'a -> hint -> expr -> 'a
  method on_unop : 'a -> Ast.uop -> expr -> 'a
  method on_binop : 'a -> Ast.bop -> expr -> expr -> 'a
  method on_pipe : 'a -> id -> expr -> expr -> 'a
  method on_eif : 'a -> expr -> expr option -> expr -> 'a
  method on_nullCoalesce : 'a -> expr -> expr -> 'a
  method on_typename : 'a -> sid -> 'a
  method on_instanceOf : 'a -> expr -> class_id -> 'a
  method on_class_id : 'a -> class_id -> 'a
  method on_new : 'a -> class_id -> expr list -> expr list -> 'a
  method on_efun : 'a -> fun_ -> id list -> 'a
  method on_xml : 'a -> sid -> (pstring * expr) list -> expr list -> 'a
  method on_assert : 'a -> assert_expr -> 'a
  method on_clone : 'a -> expr -> 'a
  method on_field: 'a -> field -> 'a
  method on_afield: 'a -> afield -> 'a

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
    | For     (e1, e2, e3, b) -> this#on_for acc e1 e2 e3 b
    | Switch  (e, cl)         -> this#on_switch acc e cl
    | Foreach (e, ae, b)      -> this#on_foreach acc e ae b
    | Try     (b, cl, fb)     -> this#on_try acc b cl fb
    | Noop                    -> this#on_noop acc
    | Fallthrough             -> this#on_fallthrough acc
    | Static_var el           -> this#on_static_var acc el
    | Global_var el           -> this#on_global_var acc el

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
   | Lvarvar (n, id) -> this#on_lvarvar acc n id
   | Fun_id sid  -> this#on_fun_id acc sid
   | Method_id (expr, pstr) -> this#on_method_id acc expr pstr
   | Method_caller (sid, pstr) -> this#on_method_caller acc sid pstr
   | Smethod_id (sid, pstr) -> this#on_smethod_id acc sid pstr
   | Yield_break -> this#on_yield_break acc
   | Yield e     -> this#on_yield acc e
   | Await e     -> this#on_await acc e
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
   | Pair        (e1, e2)    -> this#on_pair acc e1 e2
   | Cast        (hint, e)   -> this#on_cast acc hint e
   | Unop        (uop, e)         -> this#on_unop acc uop e
   | Binop       (bop, e1, e2)    -> this#on_binop acc bop e1 e2
   | Pipe        (id, e1, e2)         -> this#on_pipe acc id e1 e2
   | Eif         (e1, e2, e3)     -> this#on_eif acc e1 e2 e3
   | NullCoalesce (e1, e2)     -> this#on_nullCoalesce acc e1 e2
   | InstanceOf  (e1, e2)         -> this#on_instanceOf acc e1 e2
   | Typename n -> this#on_typename acc n
   | New         (cid, el, uel)   -> this#on_new acc cid el uel
   | Efun        (f, idl)         -> this#on_efun acc f idl
   | Xml         (sid, attrl, el) -> this#on_xml acc sid attrl el
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
  method on_lvarvar acc _ _ = acc
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
  method on_await acc e = this#on_expr acc e
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

  method on_nullCoalesce acc e1 e2 =
    let acc = this#on_expr acc e1 in
    let acc = this#on_expr acc e2 in
    acc

  method on_instanceOf acc e1 e2 =
    let acc = this#on_expr acc e1 in
    let acc = this#on_class_id acc e2 in
    acc

  method on_class_id acc = function
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
    let acc = List.fold_left begin fun acc (_, e) ->
      this#on_expr acc e
    end acc attrl in
    let acc = List.fold_left this#on_expr acc el in
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

(* The NAST definitions, which we just include into this file *)
module PosAnnotation = struct type t = Pos.t [@@deriving show] end
module PosAnnotatedAST = AnnotatedAST(PosAnnotation)
include PosAnnotatedAST

(* Expecting that Naming.func_body / Naming.class_meth_bodies has been
 * allowed at the AST. Ideally this would be enforced by the compiler,
 * a la the typechecking decl vs local phases *)
let assert_named_body = function
  | NamedBody b -> b
  | UnnamedBody _ -> failwith "Expecting a named function body"

let get_instantiated_sid_name ((_, x), _) = x

let class_id_to_str = function
  | CIparent -> SN.Classes.cParent
  | CIself -> SN.Classes.cSelf
  | CIstatic -> SN.Classes.cStatic
  | CIexpr (_, This) -> SN.SpecialIdents.this
  | CIexpr (_, Lvar (_, x)) -> "$"^Local_id.to_string x
  | CIexpr _ -> assert false
  | CI x -> get_instantiated_sid_name x

let is_kvc_kind name =
  name = SN.Collections.cMap ||
  name = SN.Collections.cImmMap ||
  name = SN.Collections.cStableMap ||
  name = SN.Collections.cDict

let get_kvc_kind name = match name with
  | x when x = SN.Collections.cMap -> `Map
  | x when x = SN.Collections.cImmMap -> `ImmMap
  | x when x = SN.Collections.cDict -> `Dict
  | _ -> begin
    Errors.internal_error Pos.none ("Invalid KeyValueCollection name: "^name);
    `Map
  end

let kvc_kind_to_name kind = match kind with
  | `Map -> SN.Collections.cMap
  | `ImmMap -> SN.Collections.cImmMap
  | `Dict -> SN.Collections.cDict

let is_vc_kind name =
  name = SN.Collections.cVector ||
  name = SN.Collections.cImmVector ||
  name = SN.Collections.cSet ||
  name = SN.Collections.cImmSet ||
  name = SN.Collections.cKeyset ||
  name = SN.Collections.cVec

let get_vc_kind name = match name with
  | x when x = SN.Collections.cVector -> `Vector
  | x when x = SN.Collections.cImmVector -> `ImmVector
  | x when x = SN.Collections.cVec -> `Vec
  | x when x = SN.Collections.cSet -> `Set
  | x when x = SN.Collections.cImmSet -> `ImmSet
  | x when x = SN.Collections.cKeyset -> `Keyset
  | _ -> begin
    Errors.internal_error Pos.none ("Invalid ValueCollection name: "^name);
    `Set
  end

let vc_kind_to_name kind = match kind with
  | `Vector -> SN.Collections.cVector
  | `ImmVector -> SN.Collections.cImmVector
  | `Vec -> SN.Collections.cVec
  | `Set -> SN.Collections.cSet
  | `ImmSet -> SN.Collections.cImmSet
  | `Keyset -> SN.Collections.cKeyset
  | `Pair -> SN.Collections.cPair
