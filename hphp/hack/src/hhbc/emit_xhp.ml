(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Emit_expression

module SU = Hhbc_string_utils

let p = Pos.none

let get_array3 i0 i1 i2 =
  let index0 = p, i0 in
  let index1 = p, i1 in
  let index2 = p, i2 in
  A.Array
  [A.AFkvalue ((p, A.Int (p, "0")), index0);
   A.AFkvalue ((p, A.Int (p, "1")), index1);
   A.AFkvalue ((p, A.Int (p, "2")), index2)]

let xhp_attribute_declaration_method name kind body =
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
    A.m_span = Pos.none;
    A.m_doc_comment = None;
  }

let emit_xhp_attribute_array ~ns xal =
  (* Taken from hphp/parser/hphp.y *)
  let hint_to_num = function
    | "HH\\string" -> 1
    | "HH\\bool" -> 2
    | "HH\\int" -> 3
    | "array" -> 4
    | "HH\\mixed" -> 6
    | "enum" -> 7
    | "HH\\float" -> 8
    | "callable" -> 9
    (* Regular class names is type 5 *)
    | _ -> 5
  in
  let get_enum_attributes = function
    | None ->
      failwith "Xhp attribute that's supposed to be an enum but not really"
    | Some (_, es) ->
      let turn_to_kv i e = A.AFkvalue ((p, A.Int (p, string_of_int i)), e) in
      p, A.Array (List.mapi ~f:turn_to_kv es)
  in
  let get_attribute_array_values id enumo =
    let id, _ = Hhbc_id.Class.elaborate_id ns (p, id) in
    let id = Hhbc_id.Class.to_raw_string id in
    let type_ = hint_to_num id in
    let type_ident = (p, A.Int (p, string_of_int type_)) in
    let class_name = match type_ with
      (* regular class names is type 5 *)
      | 5 -> (p, A.String (p, id))
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
    let is_required = (p, A.Int (p, if is_req then "1" else "0")) in
    [A.AFkvalue ((p, A.Int (p, "0")), hint);
     A.AFkvalue ((p, A.Int (p, "1")), class_name);
     A.AFkvalue ((p, A.Int (p, "2")), e);
     A.AFkvalue ((p, A.Int (p, "3")), is_required)]
  in
  let aux xa =
    let ho = Hhas_xhp_attribute.type_ xa in
    let _, (_, name), expo = Hhas_xhp_attribute.class_var xa in
    let enumo = Hhas_xhp_attribute.maybe_enum xa in
    let is_req = Hhas_xhp_attribute.is_required xa in
    let k = p, A.String (p, SU.Xhp.clean name) in
    let v = p, A.Array (inner_array ho expo enumo is_req) in
    A.AFkvalue (k, v)
  in
  p, A.Array (List.map ~f:aux xal)

let emit_xhp_use_attributes xual =
  let aux = function
    | _, A.Happly ((_, s), []) ->
      let s = SU.Xhp.mangle @@ Utils.strip_ns s in
      let e =
        p, A.Class_const ((p, s), (p, "__xhpAttributeDeclaration"))
      in
      p, A.Call (e, [], [], [])
    | _ -> failwith "Xhp use attribute - unexpected attribute"
  in
  List.map ~f:aux xual

(* AST transformations taken from hphp/parser/hphp.y *)
let from_attribute_declaration ~ns ast_class xal xual =
  let var_dollar_ = p, A.Lvar (p, "$_") in
  let neg_one = p, A.Unop(A.Uminus, (p, A.Int (p, "1"))) in
  (* static $_ = -1; *)
  let token1 = A.Static_var [p, A.Binop (A.Eq None, var_dollar_, neg_one)] in
  (* if ($_ === -1) {
   *   $_ = array_merge(parent::__xhpAttributeDeclaration(),
   *                    use_attributes
   *                    attributes);
   * }
   *)
  let cond = p, A.Binop (A.EQeqeq, var_dollar_, neg_one) in
  let arg1 =
    p, A.Call (
      (p, A.Class_const ((p, "parent"), (p, "__xhpAttributeDeclaration"))),
      [],
      [],
      [])
  in
  let args =
    arg1 :: emit_xhp_use_attributes xual @ [emit_xhp_attribute_array ~ns xal]
  in
  let array_merge_call = p, A.Call ((p, A.Id (p, "array_merge")), [], args, []) in
  let true_branch =
    [A.Expr (p, A.Binop (A.Eq None, var_dollar_, array_merge_call))]
  in
  let token2 = A.If (cond, true_branch, []) in
  (* return $_; *)
  let token3 = A.Return (p, Some var_dollar_) in
  let body = [token1; token2; token3] in
  let m =
    xhp_attribute_declaration_method
      "__xhpAttributeDeclaration"
      [A.Protected; A.Static]
      body
  in
  [Emit_method.from_ast ast_class m]

let xhp_child_op_to_int = function
  | None -> 0
  | Some A.ChildStar -> 1
  | Some A.ChildQuestion -> 2
  | Some A.ChildPlus -> 3

let rec emit_xhp_children_decl_expr ~unary = function
  | [] -> failwith "Empty xhp child declaration"
  | [A.ChildList l] ->
    get_array3
      (A.Int (p, unary))
      (A.Int (p, "5"))
      (emit_xhp_children_decl_expr ~unary:"0" l)
  | [A.ChildName (_, "any")] ->
    get_array3
      (A.Int (p, unary))
      (A.Int (p, "1"))
      (A.Null)
  | [A.ChildName (_, "pcdata")] ->
    get_array3
      (A.Int (p, unary))
      (A.Int (p, "2"))
      (A.Null)
  | [A.ChildName (_, s)] when s.[0] = '%' ->
    get_array3
      (A.Int (p, unary))
      (A.Int (p, "4"))
      (A.String (p, SU.Xhp.mangle (String.sub s 1 ((String.length s) - 1))))
  | [A.ChildName (_, s)] ->
    get_array3
      (A.Int (p, unary))
      (A.Int (p, "3"))
      (A.String (p, SU.Xhp.mangle s))
  | [A.ChildUnary (c, op)] ->
    emit_xhp_children_decl_expr [c]
      ~unary:(string_of_int @@ xhp_child_op_to_int @@ Some op)
  | [A.ChildBinary (c1, c2)] ->
    get_array3
      (A.Int (p, "5"))
      (emit_xhp_children_decl_expr ~unary [c1])
      (emit_xhp_children_decl_expr ~unary [c2])
  | [c1; c2] ->
    get_array3
      (A.Int (p, "4"))
      (emit_xhp_children_decl_expr ~unary [c1])
      (emit_xhp_children_decl_expr ~unary [c2])
  | c1 :: c2 :: cs ->
    get_array3
      (A.Int (p, "4"))
      (emit_xhp_children_decl_expr ~unary [c1; c2])
      (emit_xhp_children_decl_expr ~unary cs)

let emit_xhp_children_paren_expr c =
  let l, op_num = match c with
    | A.ChildList l -> l, xhp_child_op_to_int None
    | A.ChildUnary (A.ChildList l, op) -> l, xhp_child_op_to_int @@ Some op
    | _ ->
      failwith @@ "Xhp children declarations cannot be plain id, " ^
                  "plain binary or unary without an inside list"
  in
  let arr = emit_xhp_children_decl_expr ~unary:"0" l in
  get_array3 (A.Int (p, string_of_int op_num)) (A.Int (p, "5")) arr

let emit_xhp_children_array = function
  | [] | [A.ChildName (_, "empty")] -> A.Int (Pos.none, "0")
  | [A.ChildName (_, "any")] -> A.Int (Pos.none, "1")
  | [c] -> emit_xhp_children_paren_expr c
  | _ -> failwith "HHVM does not support multiple children declarations"

(* AST transformations taken from hphp/parser/hphp.y *)
let from_children_declaration ast_class children =
  let var_dollar_ = p, A.Lvar (p, "$_") in
  (* static $_ = children; *)
  let children_arr = p, emit_xhp_children_array children in
  let token1 =
    A.Static_var [p, A.Binop (A.Eq None, var_dollar_, children_arr)]
  in
  (* return $_; *)
  let token2 = A.Return (p, Some var_dollar_) in
  let body = [token1; token2] in
  let m =
    xhp_attribute_declaration_method
      "__xhpChildrenDeclaration"
      [A.Protected]
      body
  in
  [Emit_method.from_ast ast_class m]

let get_category_array categories =
  (* TODO: is this always 1? *)
  List.map categories
    ~f:(fun s -> A.AFkvalue ((p, A.String s), (p, A.Int (p, "1"))))

(* AST transformations taken from hphp/parser/hphp.y *)
let from_category_declaration ast_class categories =
  let var_dollar_ = p, A.Lvar (p, "$_") in
  (* static $_ = categories; *)
  let category_arr = p, A.Array (get_category_array categories) in
  let token1 =
    A.Static_var [p, A.Binop (A.Eq None, var_dollar_, category_arr)]
  in
  (* return $_; *)
  let token2 = A.Return (p, Some var_dollar_) in
  let body = [token1; token2] in
  let m =
    xhp_attribute_declaration_method
      "__xhpCategoryDeclaration"
      [A.Protected]
      body
  in
  [Emit_method.from_ast ast_class m]
