(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Hh_core
open Emit_expression

module SU = Hhbc_string_utils

let p = Pos.none

let get_array3 i0 i1 i2 =
  let index0 = p, i0 in
  let index1 = p, i1 in
  let index2 = p, i2 in
  A.Varray [index0; index1; index2]

let xhp_attribute_declaration_method ?p name kind body =
  let p = match p with
  | Some p -> p
  | None -> Pos.none in
  {
    A.m_kind = kind;
    A.m_tparams = [];
    A.m_constrs = [];
    A.m_name = Pos.none, name;
    A.m_params = [];
    A.m_body = body;
    A.m_user_attributes = [];
    A.m_ret = None;
    A.m_ret_by_ref = false;
    A.m_fun_kind = A.FSync;
    A.m_span = p;
    A.m_doc_comment = None;
  }

let emit_xhp_attribute_array ~ns xal =
  (* Taken from hphp/parser/hphp.y *)
  let hint_to_num id =
    if Emit_env.is_hh_syntax_enabled ()
    then
      begin match id with
      | "HH\\string" -> 1
      | "HH\\bool" -> 2
      | "HH\\int" -> 3
      | "array" -> 4
      | "var" | "HH\\mixed" -> 6
      | "enum" -> 7
      | "HH\\float" -> 8
      | "callable" -> 9
      (* Regular class names is type 5 *)
      | _ -> 5
      end
    else
      begin match id with
      | "string" -> 1
      | "bool" | "boolean" -> 2
      | "int" | "integer" -> 3
      | "array" -> 4
      | "var" | "mixed" -> 6
      | "enum" -> 7
      | "float" | "real" | "double" -> 8
      | "callable" -> 9
      (* Regular class names is type 5 *)
      | _ -> 5
      end
  in
  let get_enum_attributes = function
    | None ->
      failwith "Xhp attribute that's supposed to be an enum but not really"
    | Some (_, es) -> p, A.Varray es
  in
  let get_attribute_array_values id enumo =
    let id, _ = Hhbc_id.Class.elaborate_id ns (p, id) in
    let id = Hhbc_id.Class.to_raw_string id in
    let type_ = hint_to_num id in
    let type_ident = (p, A.Int (string_of_int type_)) in
    let class_name = match type_ with
      (* regular class names is type 5 *)
      | 5 -> (p, A.String id)
      (* enums are type 7 *)
      | 7 -> get_enum_attributes enumo
      | _ -> (p, A.Null)
    in
    class_name, type_ident
  in
  let inner_array ho expo enumo is_req =
    let e = match expo with None -> (p, A.Null) | Some e -> e in
    let class_name, hint = match ho with
      | None when enumo = None
      (* attribute declared with the var identifier - we treat it as mixed *)
        -> get_attribute_array_values "mixed" enumo
      | None -> get_attribute_array_values "enum" enumo
      (* As it turns out, if there is a type list, HHVM discards it *)
      | Some (_, A.Happly ((_, id), _))
      | Some (_, A.Hoption (_, A.Happly ((_, id), _)))
        -> get_attribute_array_values id enumo
      | _ -> failwith "There are no other possible xhp attribute hints"
    in
    let is_required = (p, A.Int (if is_req then "1" else "0")) in
    [hint; class_name; e; is_required]
  in
  let aux xa =
    let ho = Hhas_xhp_attribute.type_ xa in
    let _, (_, name), expo = Hhas_xhp_attribute.class_var xa in
    let enumo =
      (* TODO(T23734724): Properly deal with codegen for optional enums. *)
      Option.map ~f:(fun (p, _, e) -> p, e) (Hhas_xhp_attribute.maybe_enum xa)
    in
    let is_req = Hhas_xhp_attribute.is_required xa in
    let k = p, A.String (SU.Xhp.clean name) in
    let v = p, A.Varray (inner_array ho expo enumo is_req) in
    (k, v)
  in
  p, A.Darray (List.map ~f:aux xal)

let emit_xhp_use_attributes xual =
  let aux = function
    | _, A.Happly ((_, s), []) ->
      let s = SU.Xhp.mangle @@ Utils.strip_ns s in
      let e =
        p, A.Class_const ((p, A.Id (p, s)), (p, "__xhpAttributeDeclaration"))
      in
      p, A.Call (e, [], [], [])
    | _ -> failwith "Xhp use attribute - unexpected attribute"
  in
  List.map ~f:aux xual

let properties_for_cache ~ns ast_class class_is_immutable =
  let prop = Emit_property.from_ast
    ast_class
    []
    [Ast.Private; A.Static]
    class_is_immutable
    None
    []
    ns
    None
    (p, (p, "__xhpAttributeDeclarationCache"), Some (p, A.Null))
  in
  [prop]

(* AST transformations taken from hphp/parser/hphp.y *)
let from_attribute_declaration ~ns ast_class xal xual =
  (* $r = self::$__xhpAttributeDeclarationCache; *)
  let var_r = p, A.Lvar (p, "$r") in
  let self = p, A.Id (p, "self") in
  let cache = p, A.Class_get (self, (p, A.Lvar (p, "$__xhpAttributeDeclarationCache"))) in
  let token1 = p, A.Expr (p, A.Binop (A.Eq None, var_r, cache)) in
  (* if ($r === null) {
   *   self::$__xhpAttributeDeclarationCache =
   *       __SystemLib\\merge_xhp_attr_declarations(
   *          parent::__xhpAttributeDeclaration(),
   *          attributes
   *        );
   *   $r = self::$__xhpAttributeDeclarationCache;
   * }
   *)
  let null_ = p, A.Null in
  let cond = p, A.Binop (A.EQeqeq, var_r, null_) in
  let arg1 =
    p, A.Call (
      (p, A.Class_const ((p, A.Id (p, "parent")), (p, "__xhpAttributeDeclaration"))),
      [],
      [],
      [])
  in
  let args =
    arg1 :: emit_xhp_use_attributes xual @ [emit_xhp_attribute_array ~ns xal]
  in
  let array_merge_call =
    p, A.Call ((p, A.Id (p, "__SystemLib\\merge_xhp_attr_declarations")), [], args, []) in
  let set_cache = p, A.Expr (p, A.Binop (A.Eq None, cache, array_merge_call)) in
  let set_r = p, A.Expr (p, A.Binop (A.Eq None, var_r, cache)) in
  let true_branch = [set_cache; set_r] in
  let token2 = p, A.If (cond, true_branch, []) in
  (* return $r; *)
  let token3 = p, A.Return (Some var_r) in
  let body = [token1; token2; token3] in
  let m =
    xhp_attribute_declaration_method
      "__xhpAttributeDeclaration"
      [A.Protected; A.Static]
      body
  in
  Emit_method.from_ast ast_class m

let xhp_child_op_to_int = function
  | None -> 0
  | Some A.ChildStar -> 1
  | Some A.ChildQuestion -> 2
  | Some A.ChildPlus -> 3

let rec emit_xhp_child_decl ~unary = function
  | A.ChildList l ->
    get_array3
      (A.Int unary)
      (A.Int "5")
      (emit_xhp_children_decl_expr ~unary:"0" l)
  | A.ChildName (_, name) when String.lowercase_ascii name = "any" ->
    get_array3
      (A.Int unary)
      (A.Int "1")
      (A.Null)
  | A.ChildName (_, name) when String.lowercase_ascii name = "pcdata"->
    get_array3
      (A.Int unary)
      (A.Int "2")
      (A.Null)
  | A.ChildName (_, s) when s.[0] = '%' ->
    get_array3
      (A.Int unary)
      (A.Int "4")
      (A.String (SU.Xhp.mangle (String.sub s 1 ((String.length s) - 1))))
  | A.ChildName (_, s) ->
    get_array3
      (A.Int unary)
      (A.Int "3")
      (A.String (SU.Xhp.mangle s))
  | A.ChildUnary (c, op) ->
    emit_xhp_children_decl_expr [c]
      ~unary:(string_of_int @@ xhp_child_op_to_int @@ Some op)
  | A.ChildBinary (c1, c2) ->
    get_array3
      (A.Int "5")
      (emit_xhp_children_decl_expr ~unary [c1])
      (emit_xhp_children_decl_expr ~unary [c2])

and emit_xhp_children_decl_expr ~unary l =
  match l with
  | [] -> failwith "xhp children: unexpected empty list"
  | [c] -> emit_xhp_child_decl ~unary c
  | c1 :: c2 :: cs ->
    let first_two =
      get_array3
        (A.Int "4")
        (emit_xhp_child_decl ~unary c1)
        (emit_xhp_child_decl ~unary c2) in
    Core_list.fold_left cs ~init:first_two ~f:(fun acc n ->
      get_array3
        (A.Int "4")
        acc
        (emit_xhp_child_decl ~unary n))

let emit_xhp_children_paren_expr c =
  let l, op_num = match c with
    | A.ChildList l -> l, xhp_child_op_to_int None
    | A.ChildUnary (A.ChildList l, op) -> l, xhp_child_op_to_int @@ Some op
    | _ ->
      failwith @@ "Xhp children declarations cannot be plain id, " ^
                  "plain binary or unary without an inside list"
  in
  let arr = emit_xhp_children_decl_expr ~unary:"0" l in
  get_array3 (A.Int (string_of_int op_num)) (A.Int "5") arr

let emit_xhp_children_array = function
  | [] -> A.Int "0"
  | [A.ChildName (_, n) as c] ->
    begin match String.lowercase_ascii n with
    | "empty" -> A.Int "0"
    | "any" -> A.Int "1"
    | _ -> emit_xhp_children_paren_expr c
    end
  | [c] -> emit_xhp_children_paren_expr c
  | _ -> failwith "HHVM does not support multiple children declarations"

(* AST transformations taken from hphp/parser/hphp.y *)
let from_children_declaration ast_class (child_pos, children) =
  (* return children; *)
  let children_arr = p, emit_xhp_children_array children in
  let token1 = child_pos, A.Return (Some children_arr) in
  let body = [token1] in
  let m =
    xhp_attribute_declaration_method
      ~p:child_pos
      "__xhpChildrenDeclaration"
      [A.Protected]
      body
  in
  Emit_method.from_ast ast_class m

let get_category_array categories =
  (* TODO: is this always 1? *)
  List.map categories
    ~f:(fun s -> ((p, A.String s), (p, A.Int "1")))

(* AST transformations taken from hphp/parser/hphp.y *)
let from_category_declaration ast_class (cat_pos, categories) =
  (* return categories; *)
  let category_arr = p, A.Darray (get_category_array categories) in
  let token1 = p, A.Return (Some category_arr) in
  let body = [token1] in
  let m =
    xhp_attribute_declaration_method
      ~p:cat_pos
      "__xhpCategoryDeclaration"
      [A.Protected]
      body
  in
  Emit_method.from_ast ast_class m
