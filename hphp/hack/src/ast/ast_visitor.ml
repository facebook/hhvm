(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* This module defines a visitor class on the Ast data structure.
 * To use it you must inherit the generic object and redefine the appropriate
 * methods.
 *)
(*****************************************************************************)

open Ast

(** A {!reducer} is a AST visitor which is not in control of the iteration
 * (and thus cannot change the order of the iteration or choose not to visit
 * some subtrees).
 *
 * Intended to be used with {!Ast.reduce} to aggregate many checks into a
 * single pass over a AST.  The type parameters are as follows:
 * - 'a result being reduced (accumulated)
 * - 'b context type, which is updated by a {!Ast.reduce} visitor
 *)
class type ['a, 'b] reducer_type = object
  method at_Call : 'b -> expr -> targ list -> expr list -> expr list -> 'a
  method at_expr : 'b -> expr -> 'a
  method at_Lvar : 'b -> id -> 'a
end

class ['a, 'b] reducer
  (_zero : unit -> 'a)
  (_plus : 'a -> 'a -> 'a)
  : ['a, 'b] reducer_type
= object
  method at_Call _ctx _ _ _ _ = _zero ()
  method at_expr _ctx _e = _zero ()
  method at_Lvar _ctx _ = _zero ()
end

(*****************************************************************************)
(* The signature of the hand-rolled visitor. *)
(* DEPRECATED: use {!Ast.reduce} or {!Ast.iter} auto-generated visitors *)
(*****************************************************************************)

class type ['a] ast_visitor_type = object
  method on_afield: 'a -> afield -> 'a
  method on_array : 'a -> afield list -> 'a
  method on_array_get : 'a -> expr -> expr option -> 'a
  method on_as_expr : 'a -> as_expr -> 'a
  method on_await : 'a -> expr -> 'a
  method on_awaitall : 'a -> (id option * expr) list -> block -> 'a
  method on_binop : 'a -> bop -> expr -> expr -> 'a
  method on_pipe : 'a -> expr -> expr -> 'a
  method on_block : 'a -> block -> 'a
  method on_break : 'a -> expr option -> 'a
  method on_call : 'a -> expr -> targ list -> expr list -> expr list -> 'a
  method on_callconv : 'a -> param_kind -> expr -> 'a
  method on_case : 'a -> case -> 'a
  method on_cast : 'a -> hint -> expr -> 'a
  method on_catch : 'a -> catch -> 'a
  method on_markup: 'a -> pstring -> expr option -> 'a
  method on_class_const : 'a -> expr -> pstring -> 'a
  method on_class_get : 'a -> expr -> expr -> 'a
  method on_clone : 'a -> expr -> 'a
  method on_collection: 'a -> id -> collection_targ option -> afield list -> 'a
  method on_continue : 'a -> expr option -> 'a
  method on_darray : 'a -> (targ * targ) option -> (expr * expr) list -> 'a
  method on_def_inline : 'a -> def -> 'a
  method on_do : 'a -> block -> expr -> 'a
  method on_efun : 'a -> fun_ -> id list -> 'a
  method on_eif : 'a -> expr -> expr option -> expr -> 'a
  method on_expr : 'a -> expr -> 'a
  method on_omitted: 'a -> 'a
  method on_expr_ : 'a -> expr_ -> 'a
  method on_expr_list : 'a -> expr list -> 'a
  method on_fallthrough : 'a -> 'a
  method on_false : 'a -> 'a
  method on_field: 'a -> field -> 'a
  method on_float : 'a -> string -> 'a
  method on_for : 'a -> expr -> expr -> expr -> block -> 'a
  method on_foreach : 'a -> expr -> Pos.t option -> as_expr -> block -> 'a
  method on_goto_label : 'a -> pstring -> 'a
  method on_goto : 'a -> pstring -> 'a
  method on_hint: 'a -> hint -> 'a
  method on_id : 'a -> id -> 'a
  method on_if : 'a -> expr -> block -> block -> 'a
  method on_import: 'a -> import_flavor -> expr -> 'a
  method on_import_flavor: 'a -> import_flavor -> 'a
  method on_include: 'a -> 'a
  method on_includeOnce: 'a -> 'a
  method on_instanceOf : 'a -> expr -> expr -> 'a
  method on_int : 'a -> string -> 'a
  method on_is : 'a -> expr -> hint -> 'a
  method on_as : 'a -> expr -> hint -> bool -> 'a
  method on_let : 'a -> id -> hint option -> expr -> 'a
  method on_lfun: 'a -> fun_ -> 'a
  method on_list : 'a -> expr list -> 'a
  method on_lvar : 'a -> id -> 'a
  method on_new : 'a -> expr -> targ list -> expr list -> expr list -> 'a
  method on_record : 'a -> expr -> (expr * expr) list -> 'a
  method on_noop : 'a -> 'a
  method on_null : 'a -> 'a
  method on_obj_get : 'a -> expr -> expr -> 'a
  method on_param_kind : 'a -> param_kind -> 'a
  method on_pstring : 'a -> pstring -> 'a
  method on_require: 'a -> 'a
  method on_requireOnce: 'a -> 'a
  method on_return : 'a -> expr option -> 'a
  method on_sfclass_const: 'a -> id -> pstring -> 'a
  method on_sflit: 'a -> pstring -> 'a
  method on_shape : 'a -> (shape_field_name * expr) list -> 'a
  method on_shape_field_name: 'a -> shape_field_name -> 'a
  method on_stmt : 'a -> stmt -> 'a
  method on_stmt_ : 'a -> stmt_ -> 'a
  method on_string2 : 'a -> expr list -> 'a
  method on_string : 'a -> string -> 'a
  method on_suspend: 'a -> expr -> 'a
  method on_switch : 'a -> expr -> case list -> 'a
  method on_targ : 'a -> hint -> 'a
  method on_throw : 'a -> expr -> 'a
  method on_true : 'a -> 'a
  method on_try : 'a -> block -> catch list -> block -> 'a
  method on_unop : 'a -> uop -> expr -> 'a
  method on_unsafe: 'a -> 'a
  method on_using: 'a -> using_stmt -> 'a
  method on_varray : 'a -> targ option -> expr list -> 'a
  method on_while : 'a -> expr -> block -> 'a
  method on_declare : 'a -> bool -> expr -> block -> 'a
  method on_xml : 'a -> id -> xhp_attribute list -> expr list -> 'a
  method on_yield : 'a -> afield -> 'a
  method on_yield_from : 'a -> expr -> 'a
  method on_yield_break : 'a -> 'a


  (* traversal for top-level parts of the AST *)
  (* may not be exactly what you want for all implementations*)
  method on_absConst: 'a -> hint option -> id -> 'a
  method on_attributes: 'a -> class_attr list -> 'a
  method on_class_: 'a -> class_ -> 'a
  method on_class_elt: 'a -> class_elt -> 'a
  method on_classTraitRequire: 'a -> trait_req_kind -> hint -> 'a
  method on_classUse: 'a -> hint -> 'a
  method on_classUseAlias: 'a ->
                           id option -> pstring ->
                           id option -> kind list -> 'a
  method on_classUsePrecedence: 'a -> id -> pstring -> id list -> 'a
  method on_methodTraitResolution: 'a -> method_trait_resolution -> 'a
  method on_classVars:
    'a -> class_vars_ -> 'a
  method on_const: 'a -> hint option -> (id * expr) list -> 'a
  method on_constant: 'a -> gconst -> 'a
  method on_def: 'a -> def -> 'a
  method on_fun_: 'a -> fun_ -> 'a
  method on_fun_param: 'a -> fun_param -> 'a
  method on_gconst: 'a -> gconst -> 'a
  method on_method_: 'a -> method_ -> 'a
  method on_namespace: 'a -> id -> program -> 'a
  method on_namespaceUse: 'a -> (Ast.ns_kind * id * id) list -> 'a
  method on_file_attributes: 'a -> Ast.file_attributes -> 'a
  method on_program: 'a -> program -> 'a
  method on_tparam: 'a -> tparam -> 'a
  method on_typeConst: 'a -> typeconst -> 'a
  method on_typedef: 'a -> typedef -> 'a
  method on_user_attribute: 'a -> user_attribute -> 'a
  method on_xhpAttr: 'a -> hint option -> class_var -> bool ->
                     ((Pos.t * bool * expr list) option) -> 'a
  method on_xhpAttrUse: 'a -> hint -> 'a
  method on_xhpCategory: 'a -> pstring list -> 'a
  method on_xhp_child: 'a -> xhp_child -> 'a

  (* Pocket Universes *)
  method on_pu_atom : 'a -> id -> 'a
  method on_pu_identifier: 'a -> expr -> id -> id -> 'a
  method on_pumapping : 'a -> pumapping -> 'a
  method on_pufield : 'a -> pufield -> 'a
  method on_class_enum_ : 'a -> id -> pufield list -> 'a

end

(*****************************************************************************)
(* The generic but hand-rolled visitor ('a is the type of the accumulator). *)
(* DEPRECATED: use {!Ast.reduce} or {!Ast.iter} auto-generated visitors *)
(*****************************************************************************)

class virtual ['a] ast_visitor: ['a] ast_visitor_type = object(this)

  method on_break acc level_opt =
    match level_opt with
    | Some e -> this#on_expr acc e
    | None -> acc
  method on_continue acc _ = acc
  method on_noop acc = acc
  method on_fallthrough acc = acc
  method on_unsafe acc = acc
  method on_include acc = acc
  method on_require acc = acc
  method on_includeOnce acc = acc
  method on_requireOnce acc = acc

  method on_hint acc h =
    match (snd h) with
    | Hsoft h
    | Hlike h
    | Hoption h ->
      let acc = this#on_hint acc h in
      acc
    | Hfun (_, hl, kl, _, h) ->
      let acc = List.fold_left this#on_hint acc hl in
      let acc = List.fold_left (fun acc k ->
        match k with
        | Some kind -> this#on_param_kind acc kind
        | None -> acc
      ) acc kl in
      let acc = this#on_hint acc h in
      acc
    | Htuple hl ->
      let acc = List.fold_left this#on_hint acc hl in
      acc
    | Happly (id, hl) ->
      let acc = this#on_id acc id in
      let acc = List.fold_left this#on_hint acc hl in
      acc
    | Hshape shape_info ->
      let {si_shape_field_list; _} = shape_info in
      let acc = List.fold_left (fun acc sf ->
        let acc = this#on_shape_field_name acc sf.sf_name in
        let acc = this#on_hint acc sf.sf_hint in
        acc
      ) acc si_shape_field_list in
      acc
    | Haccess (id1, id2, idl) ->
      let acc = this#on_id acc id1 in
      let acc = this#on_id acc id2 in
      let acc = List.fold_left this#on_id acc idl in
      acc

  method on_targ acc h =
    let acc = this#on_hint acc h in
    acc

  method on_throw acc e =
    let acc = this#on_expr acc e in
    acc

  method on_return acc eopt =
    match eopt with
    | None -> acc
    | Some e -> this#on_expr acc e

  method on_awaitall acc el b =
    let acc = List.fold_left (fun acc (e1, e2) ->
      let acc = (match e1 with
      | None -> acc
      | Some e -> this#on_lvar acc e) in
      this#on_expr acc e2
    ) acc el in
    this#on_block acc b

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

  method on_declare acc _ e b =
    let acc = this#on_expr acc e in
    let acc = this#on_block acc b in
    acc

  method on_for acc e1 e2 e3 b =
    let acc = this#on_expr acc e1 in
    let acc = this#on_expr acc e2 in
    let acc = this#on_expr acc e3 in
    let acc = this#on_block acc b in
    acc

  method on_using acc s =
    let acc = this#on_expr acc s.us_expr in
    let acc = this#on_block acc s.us_block in
    acc

  method on_switch acc e cl =
    let acc = this#on_expr acc e in
    let acc = List.fold_left this#on_case acc cl in
    acc

  method on_foreach acc  e _ ae b =
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
   | As_v e ->
       let acc = this#on_expr acc e in
       acc
   | As_kv (e1, e2) ->
       let acc = this#on_expr acc e1 in
       let acc = this#on_expr acc e2 in
       acc

  method on_catch acc (i1, i2, b) =
    let acc = this#on_id acc i1 in
    let acc = this#on_id acc i2 in
    let acc = this#on_block acc b in
    acc

  method on_markup acc pstr e =
    let acc = this#on_pstring acc pstr in
    match e with
    | Some e -> this#on_expr acc e
    | None -> acc

  method on_let acc id hint_opt e =
    let acc = this#on_id acc id in
    let acc =
      match hint_opt with
        | None -> acc
        | Some h -> this#on_hint acc h
    in
    let acc = this#on_expr acc e in
    acc

  method on_stmt_ acc = function
    | Unsafe                  -> this#on_unsafe acc
    | Expr e                  -> this#on_expr acc e
    | Break level_opt         -> this#on_break acc level_opt
    | Block b                 -> this#on_block acc b
    | Continue level_opt      -> this#on_continue acc level_opt
    | Throw   (e)             -> this#on_throw acc e
    | Return  eopt            -> this#on_return acc eopt
    | GotoLabel label         -> this#on_goto_label acc label
    | Goto label              -> this#on_goto acc label
    | If      (e, b1, b2)     -> this#on_if acc e b1 b2
    | Do      (b, e)          -> this#on_do acc b e
    | While   (e, b)          -> this#on_while acc e b
    | For     (e1, e2, e3, b) -> this#on_for acc e1 e2 e3 b
    | Switch  (e, cl)         -> this#on_switch acc e cl
    | Foreach (e, popt, ae, b)-> this#on_foreach acc e popt ae b
    | Try     (b, cl, fb)     -> this#on_try acc b cl fb
    | Def_inline d ->
      this#on_def_inline acc d
    | Noop                    -> this#on_noop acc
    | Fallthrough             -> this#on_fallthrough acc
    | Awaitall (el, b)        -> this#on_awaitall acc el b
    | Markup (s, e)           -> this#on_markup acc s e
    | Using s                 -> this#on_using acc s
    | Declare (is_block, e, b) -> this#on_declare acc is_block e b
    | Let (id, h, e)          -> this#on_let acc id h e

  method on_def_inline acc d =
    this#on_def acc d

  method on_xhp_child acc e =
    match e with
   | ChildName id ->  this#on_id acc id
   | ChildList children -> List.fold_left this#on_xhp_child acc children
   | ChildUnary (child, _) -> this#on_xhp_child acc child
   | ChildBinary (c1, c2) ->
     let acc = this#on_xhp_child acc c1 in
     this#on_xhp_child acc c2

  method on_expr acc (_, e) =
    this#on_expr_ acc e

  method on_stmt acc (_, s) =
    this#on_stmt_ acc s

  method on_omitted acc = acc

  method on_expr_ acc e =
    match e with
   | Unsafeexpr e-> this#on_expr acc e
   | Collection (i, tal, afl) -> this#on_collection acc i tal afl
   | Lfun f          -> this#on_lfun acc f
   | Import (ifv, e) -> this#on_import acc ifv e
   | Array afl   -> this#on_array acc afl
   | Darray (tap, fl) -> this#on_darray acc tap fl
   | Varray (ta, el) -> this#on_varray acc ta el
   | Shape sh    -> this#on_shape acc sh
   | True        -> this#on_true acc
   | False       -> this#on_false acc
   | Int n       -> this#on_int acc n
   | Float n     -> this#on_float acc n
   | Null        -> this#on_null acc
   | String s    -> this#on_string acc s
   | Id id       -> this#on_id acc id
   | Lvar id     -> this#on_lvar acc id
   | Yield_break -> this#on_yield_break acc
   | Yield e     -> this#on_yield acc e
   | Yield_from e -> this#on_yield_from acc e
   | Await e     -> this#on_await acc e
   | List el     -> this#on_list acc el
   | Clone e     -> this#on_clone acc e
   | Expr_list el    -> this#on_expr_list acc el
   | Obj_get     (e1, e2, _) -> this#on_obj_get acc e1 e2
   | Array_get   (e1, e2)    -> this#on_array_get acc e1 e2
   | Class_get   (e1, p)   -> this#on_class_get acc e1 p
   | Class_const (e1, pstr)   -> this#on_class_const acc e1 pstr
   | Call        (e, hl, el, uel) -> this#on_call acc e hl el uel
   | String2     el           -> this#on_string2 acc el
   | PrefixedString (_, e)    -> this#on_expr acc e
   | Cast        (hint, e)   -> this#on_cast acc hint e
   | Unop        (uop, e)         -> this#on_unop acc uop e
   | Binop       (bop, e1, e2)    -> this#on_binop acc bop e1 e2
   | Pipe        (e1, e2)    -> this#on_pipe acc e1 e2
   | Eif         (e1, e2, e3)     -> this#on_eif acc e1 e2 e3
   | InstanceOf  (e1, e2)         -> this#on_instanceOf acc e1 e2
   | Is          (e, h) -> this#on_is acc e h
   | As          (e, h, b) -> this#on_as acc e h b
   | BracedExpr e
   | ParenthesizedExpr e -> this#on_expr acc e
   | New         (e, hl, el, uel) -> this#on_new acc e hl el uel
   | Record      (e, fl)          -> this#on_record acc e fl
   | Efun        (f, idl)         -> this#on_efun acc f idl
   | Xml         (id, attrl, el) -> this#on_xml acc id attrl el
   | Omitted                     -> this#on_omitted  acc
   | Suspend e  -> this#on_suspend acc e
   | Callconv    (kind, e)   -> this#on_callconv acc kind e
   | PU_atom id    -> this#on_pu_atom acc id
   | PU_identifier (e, id1, id2)   -> this#on_pu_identifier acc e id1 id2

  method on_array acc afl =
    List.fold_left this#on_afield acc afl

  method on_darray acc tap fl =
    let acc = match tap with
    | Some ta ->
      let acc = this#on_targ acc (fst ta) in
      let acc = this#on_targ acc (snd ta) in
      acc
    | None -> acc in

    let on_field acc (e1, e2) =
      let acc = this#on_expr acc e1 in
      this#on_expr acc e2 in
    List.fold_left on_field acc fl

  method on_varray acc ta el =
    let acc = match ta with
    | Some t -> this#on_targ acc t
    | None -> acc in
    List.fold_left this#on_expr acc el

  method on_shape acc sfnel =
    List.fold_left begin fun acc (sfn, e) ->
      let acc = this#on_shape_field_name acc sfn in
      let acc = this#on_expr acc e in
      acc
    end  acc sfnel

  method on_id acc _ = acc

  method on_lvar acc _ = acc

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

  method on_class_get acc e p =
    let acc = this#on_expr acc e in
    let acc = this#on_expr acc p in
    acc

  method on_class_const acc e pstr =
    let acc = this#on_expr acc e in
    let acc = this#on_pstring acc pstr in
    acc

  method on_call acc e hl el uel =
    let acc = this#on_expr acc e in
    let acc = List.fold_left this#on_targ acc hl in
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

  method on_yield_break acc = acc
  method on_yield acc e = this#on_afield acc e
  method on_yield_from acc e = this#on_expr acc e
  method on_await acc e = this#on_expr acc e
  method on_suspend acc e = this#on_expr acc e
  method on_list acc el = List.fold_left this#on_expr acc el

  method on_expr_list acc el =
    let acc = List.fold_left this#on_expr acc el in
    acc

  method on_cast acc h e =
    let acc = this#on_expr acc e in
    let acc = this#on_hint acc h in
    acc

  method on_unop acc _ e = this#on_expr acc e

  method on_binop acc _ e1 e2 =
    let acc = this#on_expr acc e1 in
    let acc = this#on_expr acc e2 in
    acc

  method on_pipe acc e1 e2 =
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
    let acc = this#on_expr acc e2 in
    acc

  method on_is acc e h =
    let acc = this#on_expr acc e in
    let acc = this#on_hint acc h in
    acc

  method on_as acc e h _b =
    let acc = this#on_expr acc e in
    let acc = this#on_hint acc h in
    acc

  method on_new acc e hl el uel =
    let acc = this#on_expr acc e in
    let acc = List.fold_left this#on_targ acc hl in
    let acc = List.fold_left this#on_expr acc el in
    let acc = List.fold_left this#on_expr acc uel in
    acc

  method on_record acc e fl =
    let acc = this#on_expr acc e in
    let on_field acc (e1, e2) =
      let acc = this#on_expr acc e1 in
      this#on_expr acc e2 in
    List.fold_left on_field acc fl

  method on_efun acc f _ = this#on_fun_ acc f

  method on_xml acc pstr attrl el =
    let acc = this#on_pstring acc pstr in
    let acc = List.fold_left begin fun acc attr ->
      match attr with
      | Xhp_simple (_, e) -> this#on_expr acc e
      | Xhp_spread e -> this#on_expr acc e
    end acc attrl in
    let acc = List.fold_left this#on_expr acc el in
    acc

  method on_goto_label = this#on_pstring

  method on_goto = this#on_pstring

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

  method on_shape_field_name acc = function
    | SFlit_int pstr
    | SFlit_str pstr -> this#on_sflit acc pstr
    | SFclass_const (id, pstr) -> this#on_sfclass_const acc id pstr

  method on_sflit acc pstr = this#on_pstring acc pstr
  method on_sfclass_const acc (p, _ as id) c =
    this#on_class_const acc (p, Id id) c

  method on_collection acc i ta afl =
    let acc = this#on_id acc i in
    let acc = match ta with
    | Some CollectionTV t ->
      this#on_targ acc t
    | Some CollectionTKV (tk, tv) ->
      let acc = this#on_targ acc tk in
      let acc = this#on_targ acc tv in
      acc
    | None -> acc in
    let acc = List.fold_left this#on_afield acc afl in
    acc

  method on_import acc ifv e =
    let acc = this#on_import_flavor acc ifv in
    let acc = this#on_expr acc e in
    acc

  method on_import_flavor acc = function
    | Include     -> this#on_include acc
    | Require     -> this#on_require acc
    | IncludeOnce -> this#on_includeOnce acc
    | RequireOnce -> this#on_requireOnce acc

  method on_lfun acc l = this#on_fun_ acc l

  method on_param_kind acc _ = acc

  method on_callconv acc kind e =
    let acc = this#on_param_kind acc kind in
    let acc = this#on_expr acc e in
    acc

  method on_fun_ acc f =
    let acc = List.fold_left this#on_user_attribute acc f.f_user_attributes in
    let acc = this#on_id acc f.f_name in
    let acc = List.fold_left this#on_tparam acc f.f_tparams in
    let acc = List.fold_left this#on_fun_param acc f.f_params in
    let acc = this#on_block acc f.f_body in
    let acc = match f.f_ret with
      | Some h -> this#on_hint acc h
      | None -> acc in
    acc

  method on_program acc p =
    let acc = List.fold_left begin fun acc d ->
      this#on_def acc d end acc p in
    acc

  method on_def acc = function
    | Fun f -> this#on_fun_ acc f
    | Class c -> this#on_class_ acc c
    | Stmt s -> this#on_stmt acc s
    | Typedef t -> this#on_typedef acc t
    | Constant g -> this#on_constant acc g
    | Namespace (i, p) -> this#on_namespace acc i p
    | NamespaceUse idl -> this#on_namespaceUse acc idl
    | SetNamespaceEnv _e -> acc
    | FileAttributes fa -> this#on_file_attributes acc fa

  method on_class_ acc c =
    let acc = List.fold_left this#on_user_attribute acc c.c_user_attributes in
    let acc = this#on_id acc c.c_name in
    let acc = List.fold_left this#on_tparam acc c.c_tparams in
    let acc = List.fold_left this#on_hint acc c.c_extends in
    let acc = List.fold_left this#on_hint acc c.c_implements in
    let acc = List.fold_left this#on_class_elt acc c.c_body in
    acc

  method on_typedef acc t =
    let acc = this#on_id acc t.t_id in
    let acc = match t.t_kind with
      | Alias h | NewType h -> this#on_hint acc h in
    let acc = List.fold_left this#on_tparam acc t.t_tparams in
    let acc = List.fold_left this#on_user_attribute acc t.t_user_attributes in
    acc

  method on_constant acc g =
    this#on_gconst acc g

  method on_namespace acc i p =
    let acc = this#on_id acc i in
    let acc = this#on_program acc p in
    acc

  method on_namespaceUse acc il =
    List.fold_left begin fun acc (_, i1, i2) ->
      let acc = this#on_id acc i1 in
      let acc = this#on_id acc i2 in
      acc end acc il

  method on_file_attributes acc fa =
    List.fold_left this#on_user_attribute acc fa.fa_user_attributes

  method on_tparam acc t =
    let acc = this#on_id acc t.tp_name in
    let on_tparam_constraint acc (_, h) = this#on_hint acc h in
    let acc = List.fold_left on_tparam_constraint acc t.tp_constraints in
    let acc = List.fold_left this#on_user_attribute acc t.tp_user_attributes in
    acc

  method on_fun_param acc f =
    let acc = this#on_id acc f.param_id in
    let acc = match f.param_expr with
      | None -> acc
      | Some expr -> this#on_expr acc expr in
    let acc = match f.param_hint with
      | Some h -> this#on_hint acc h
      | None -> acc in
    let acc = match f.param_callconv with
      | Some kind -> this#on_param_kind acc kind
      | None -> acc in
    acc

  method on_user_attribute acc u =
    let acc = this#on_id acc u.ua_name in
    let acc = List.fold_left this#on_expr acc u.ua_params in
    acc

  method on_gconst acc g =
    let acc = this#on_id acc g.cst_name in
    let acc = this#on_expr acc g.cst_value in
    let acc = match g.cst_type with
      | Some h -> this#on_hint acc h
      | None -> acc in
    acc

  method on_class_elt acc = function
    | Const (hopt, iel) -> this#on_const acc hopt iel
    | AbsConst (h, a) -> this#on_absConst acc h a
    | Attributes cl -> this#on_attributes acc cl
    | TypeConst t -> this#on_typeConst acc t
    | ClassUse h -> this#on_classUse acc h
    | ClassUseAlias (ido1, ps, ido2, ko) ->
      this#on_classUseAlias acc ido1 ps ido2 ko
    | ClassUsePrecedence (id, ps, ids) ->
      this#on_classUsePrecedence acc id ps ids
    | MethodTraitResolution i ->
      this#on_methodTraitResolution acc i
    | XhpAttrUse h -> this#on_xhpAttrUse acc h
    | XhpCategory (_, cs) -> this#on_xhpCategory acc cs
    | XhpChild (_, c) -> this#on_xhp_child acc c
    | ClassTraitRequire (t, h) -> this#on_classTraitRequire acc t h
    | ClassVars cv -> this#on_classVars acc cv
    | XhpAttr (t,h,i,n) -> this#on_xhpAttr acc t h i n
    | Method m -> this#on_method_ acc m
    | ClassEnum (_, id, fields) -> this#on_class_enum_ acc id fields

  method on_const acc h_opt consts =
    let acc = match h_opt with
      | Some h -> this#on_hint acc h
      | None -> acc in
    let acc = List.fold_left (fun acc (id, expr) ->
      let acc = this#on_id acc id in
      let acc = this#on_expr acc expr in
      acc
    ) acc consts in
    acc
  method on_absConst acc h_opt id =
    let acc = match h_opt with
      | Some h -> this#on_hint acc h
      | None -> acc in
    let acc = this#on_id acc id in
    acc
  method on_attributes acc _ = acc
  method on_typeConst acc t =
    let acc = this#on_id acc t.tconst_name in
    let acc = match t.tconst_constraint with
      | Some h -> this#on_hint acc h
      | None -> acc in
    let acc = match t.tconst_type with
      | Some h -> this#on_hint acc h
      | None -> acc in
    acc
  method on_classUse acc h =
    let acc = this#on_hint acc h in
    acc
  method on_classUseAlias acc ido1 ps ido2 _ =
    let acc = match ido1 with
      | Some id -> this#on_id acc id
      | None -> acc in
    let acc = this#on_pstring acc ps in
    let acc = match ido2 with
      | Some id -> this#on_id acc id
      | None -> acc in
    acc
  method on_classUsePrecedence acc id ps ids =
    let acc = this#on_id acc id in
    let acc = this#on_pstring acc ps in
    let acc = List.fold_left this#on_id acc ids in
    acc
  method on_methodTraitResolution acc mtr =
    let acc = this#on_id acc mtr.mt_name in
    let acc = List.fold_left this#on_tparam acc mtr.mt_tparams in
    let acc = List.fold_left this#on_fun_param acc mtr.mt_params in
    let acc = List.fold_left this#on_user_attribute acc mtr.mt_user_attributes in
    let acc = match mtr.mt_ret with
      | Some h -> this#on_hint acc h
      | None -> acc in
    acc

  method on_xhpAttrUse acc h =
    let acc = this#on_hint acc h in
    acc
  method on_classTraitRequire acc _ h =
    let acc = this#on_hint acc h in
    acc
  method on_classVars acc cv =
    let acc = match cv.cv_hint with
      | Some h -> this#on_hint acc h
      | None -> acc in
    let acc = List.fold_left (fun acc (_, id, opt_expr) ->
      let acc = this#on_id acc id in
      match opt_expr with
      | Some expr -> this#on_expr acc expr
      | None -> acc
    ) acc cv.cv_names in
    acc
  method on_xhpAttr acc h_opt _ _ _ =
    let acc = match h_opt with
      | Some h -> this#on_hint acc h
      | None -> acc in
    acc
  method on_xhpCategory acc cs =
    let acc = List.fold_left this#on_pstring acc cs in
    acc

  method on_method_ acc m =
    let acc = this#on_id acc m.m_name in
    let acc = List.fold_left this#on_tparam acc m.m_tparams in
    let acc = List.fold_left this#on_fun_param acc m.m_params in
    let acc = List.fold_left this#on_user_attribute acc m.m_user_attributes in
    let acc = match m.m_ret with
      | Some h -> this#on_hint acc h
      | None -> acc in
    let acc = this#on_block acc m.m_body in
    acc

  method on_pstring acc _ = acc

  method on_pu_atom acc _ = acc

  method on_pu_identifier acc e _ _ =
    this#on_expr acc e

  method on_pumapping acc = function
    | PUMappingID (id, e) ->
      let acc = this#on_id acc id in
      let acc = this#on_expr acc e in
      acc
    | PUMappingType (id, h) ->
      let acc = this#on_id acc id in
      let acc = this#on_hint acc h in
      acc

  method on_pufield acc = function
    | PUAtomDecl (id, mappings) ->
      let acc = this#on_id acc id in
      let acc = List.fold_left this#on_pumapping acc mappings in
      acc
    | PUCaseType id -> this#on_id acc id
    | PUCaseTypeExpr (h, id) ->
      let acc = this#on_hint acc h in
      let acc = this#on_id acc id in
      acc

  method on_class_enum_ acc id fields =
    let acc = this#on_id acc id in
    let acc = List.fold_left this#on_pufield acc fields in
    acc

end
