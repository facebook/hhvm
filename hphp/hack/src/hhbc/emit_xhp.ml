(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module A = Ast_defs
module SU = Hhbc_string_utils
module T = Aast

let p = Pos.none

let get_array3 i0 i1 i2 =
  let index0 = Tast_annotate.make i0 in
  let index1 = Tast_annotate.make i1 in
  let index2 = Tast_annotate.make i2 in
  T.Varray (None, [index0; index1; index2])

let xhp_attribute_declaration_method
    ?p name final abstract static visibility stmtl =
  let m_body = T.{ fb_ast = stmtl; fb_annotation = Tast.NoUnsafeBlocks } in
  let p =
    match p with
    | Some p -> p
    | None -> Pos.none
  in
  T.
    {
      m_span = p;
      m_annotation = Tast.dummy_saved_env;
      (* Dummy env *)
      m_final = final;
      m_abstract = abstract;
      m_static = static;
      m_visibility = visibility;
      m_name = (Pos.none, name);
      m_tparams = [];
      m_where_constraints = [];
      m_variadic = T.FVnonVariadic;
      m_params = [];
      m_body;
      m_fun_kind = A.FSync;
      m_user_attributes = [];
      m_ret = Tast.dummy_type_hint None;
      m_external = false;
      m_doc_comment = None;
    }

let emit_xhp_attribute_array xal =
  (* Taken from hphp/parser/hphp.y *)
  let hint_to_num id =
    match id with
    | "HH\\string" -> 1
    | "HH\\bool" -> 2
    | "HH\\int" -> 3
    | "array" -> 4
    | "var"
    | "HH\\mixed" ->
      6
    | "enum" -> 7
    | "HH\\float" -> 8
    | "callable" -> 9
    (* Regular class names are type 5 *)
    | _ -> 5
  in
  let get_enum_attributes enumo =
    match enumo with
    | None ->
      failwith "Xhp attribute that's supposed to be an enum but not really"
    | Some (_, es) -> Tast_annotate.make (T.Varray (None, es))
  in
  let get_attribute_array_values id enumo =
    let id = Hhbc_id.Class.(from_ast_name id |> to_raw_string) in
    let type_ = hint_to_num id in
    let type_ident = Tast_annotate.make (T.Int (string_of_int type_)) in
    let class_name =
      match type_ with
      (* regular class names is type 5 *)
      | 5 -> Tast_annotate.make (T.String id)
      (* enums are type 7 *)
      | 7 -> get_enum_attributes enumo
      | _ -> Tast_annotate.make T.Null
    in
    (class_name, type_ident)
  in
  let inner_array ho expo enumo is_req =
    let e =
      match expo with
      | None -> Tast_annotate.make T.Null
      | Some e -> e
    in
    let rec extract_from_hint h =
      match h with
      | (_, Aast.Happly ((_, inc), [h]))
        when inc = Naming_special_names.FB.cIncorrectType ->
        extract_from_hint h
      | (_, Aast.Hlike h)
      | (_, Aast.Hoption h) ->
        extract_from_hint h
      | (_, Aast.Happly ((_, id), _)) -> get_attribute_array_values id enumo
      | _ -> failwith "There are no other possible xhp attribute hints"
    in
    let (class_name, hint) =
      match ho with
      | None when enumo = None ->
        (* attribute declared with the var identifier - we treat it as mixed *)
        get_attribute_array_values "\\HH\\mixed" enumo
      | None -> get_attribute_array_values "enum" enumo
      (* As it turns out, if there is a type list, HHVM discards it *)
      | Some h -> extract_from_hint h
    in
    let is_required =
      Tast_annotate.make
        (T.Int
           ( if is_req then
             "1"
           else
             "0" ))
    in
    [hint; class_name; e; is_required]
  in
  let aux xa =
    let ho = Hhas_xhp_attribute.type_ xa in
    let cv = Hhas_xhp_attribute.class_var xa in
    let (_, name) = cv.T.cv_id in
    let expo = cv.T.cv_expr in
    let enumo =
      (* TODO(T23734724): Properly deal with codegen for optional enums. *)
      Option.map ~f:(fun (p, _, e) -> (p, e)) (Hhas_xhp_attribute.maybe_enum xa)
    in
    let is_req = Hhas_xhp_attribute.is_required xa in
    let k = Tast_annotate.make (T.String (SU.Xhp.clean name)) in
    let v =
      Tast_annotate.make (T.Varray (None, inner_array ho expo enumo is_req))
    in
    (k, v)
  in
  Tast_annotate.make (T.Darray (None, List.map ~f:aux xal))

let emit_xhp_use_attributes xual =
  let aux xua (* A cute little palindrome :P *) =
    match xua with
    | (_, Aast.Happly ((_, s), [])) ->
      let s = SU.Xhp.mangle @@ Utils.strip_ns s in
      let e =
        Tast_annotate.make
          (T.Class_const
             ( Tast_annotate.make (T.CIexpr (Tast_annotate.make (T.Id (p, s)))),
               (p, "__xhpAttributeDeclaration") ))
      in
      Tast_annotate.make (T.Call (Aast.Cnormal, e, [], [], None))
    | _ -> failwith "Xhp use attribute - unexpected attribute"
  in
  List.map ~f:aux xual

let properties_for_cache ~ns class_ class_is_const =
  let init_value = Tast_annotate.make T.Null in
  let prop =
    Emit_property.from_ast
      class_
      []
      false (* is_not_abstract *)
      true (* is_static *)
      Aast.Private
      class_is_const
      None
      []
      ns
      None
      (p, (p, "__xhpAttributeDeclarationCache"), Some init_value)
  in
  [prop]

(* AST transformations taken from hphp/parser/hphp.y *)
let from_attribute_declaration class_ xal xual =
  (* $r = self::$__xhpAttributeDeclarationCache; *)
  let var_r = Tast_annotate.make (T.Lvar (p, Local_id.make_unscoped "$r")) in
  let self = Tast_annotate.make (T.Id (p, "self")) in
  let cache =
    Tast_annotate.make
      (T.Class_get
         ( Tast_annotate.make (T.CIexpr self),
           T.CGstring (p, "$__xhpAttributeDeclarationCache") ))
  in
  let token1 =
    (p, T.Expr (Tast_annotate.make (T.Binop (A.Eq None, var_r, cache))))
  in
  (* if ($r === null) {
   *   self::$__xhpAttributeDeclarationCache =
   *       __SystemLib\\merge_xhp_attr_declarations(
   *          parent::__xhpAttributeDeclaration(),
   *          attributes
   *        );
   *   $r = self::$__xhpAttributeDeclarationCache;
   * }
   *)
  let null_ = Tast_annotate.make T.Null in
  let cond = Tast_annotate.make (T.Binop (A.Eqeqeq, var_r, null_)) in
  let arg1 =
    Tast_annotate.make
      (T.Call
         ( Aast.Cnormal,
           Tast_annotate.make
             (T.Class_const
                ( Tast_annotate.make
                    (T.CIexpr (Tast_annotate.make (T.Id (p, "parent")))),
                  (p, "__xhpAttributeDeclaration") )),
           [],
           [],
           None ))
  in
  let args =
    (arg1 :: emit_xhp_use_attributes xual) @ [emit_xhp_attribute_array xal]
  in
  let array_merge_call =
    Tast_annotate.make
      (T.Call
         ( Aast.Cnormal,
           Tast_annotate.make
             (T.Id (p, "__SystemLib\\merge_xhp_attr_declarations")),
           [],
           args,
           None ))
  in
  let set_cache =
    ( p,
      T.Expr (Tast_annotate.make (T.Binop (A.Eq None, cache, array_merge_call)))
    )
  in
  let set_r =
    (p, T.Expr (Tast_annotate.make (T.Binop (A.Eq None, var_r, cache))))
  in
  let true_branch = [set_cache; set_r] in
  let token2 = (p, T.If (cond, true_branch, [])) in
  (* return $r; *)
  let token3 = (p, T.Return (Some var_r)) in
  let body = [token1; token2; token3] in
  let m =
    xhp_attribute_declaration_method
      "__xhpAttributeDeclaration"
      false
      false
      true (* static *)
      Aast.Protected
      body
  in
  Emit_method.from_ast class_ m

let rec xhp_child_op_to_int op =
  match op with
  | None -> 0
  | Some Aast.ChildStar -> 1
  | Some Aast.ChildQuestion -> 2
  | Some Aast.ChildPlus -> 3

and emit_xhp_child_decl ~unary child =
  match child with
  | Aast.ChildList l ->
    get_array3
      (T.Int unary)
      (T.Int "5")
      (emit_xhp_children_decl_expr ~unary:"0" l)
  | Aast.ChildName (_, name) when String.lowercase name = "any" ->
    get_array3 (T.Int unary) (T.Int "1") T.Null
  | Aast.ChildName (_, name) when String.lowercase name = "pcdata" ->
    get_array3 (T.Int unary) (T.Int "2") T.Null
  | Aast.ChildName (_, s) when s.[0] = '%' ->
    get_array3
      (T.Int unary)
      (T.Int "4")
      (T.String (SU.Xhp.mangle (String.sub s 1 (String.length s - 1))))
  | Aast.ChildName (_, s) ->
    get_array3 (T.Int unary) (T.Int "3") (T.String (SU.Xhp.mangle s))
  | Aast.ChildUnary (c, op) ->
    emit_xhp_children_decl_expr
      [c]
      ~unary:(string_of_int @@ xhp_child_op_to_int @@ Some op)
  | Aast.ChildBinary (c1, c2) ->
    get_array3
      (T.Int "5")
      (emit_xhp_children_decl_expr ~unary [c1])
      (emit_xhp_children_decl_expr ~unary [c2])

and emit_xhp_children_decl_expr ~unary l =
  match l with
  | [] -> failwith "xhp children: unexpected empty list"
  | [c] -> emit_xhp_child_decl ~unary c
  | c1 :: c2 :: cs ->
    let first_two =
      get_array3
        (T.Int "4")
        (emit_xhp_child_decl ~unary c1)
        (emit_xhp_child_decl ~unary c2)
    in
    List.fold_left cs ~init:first_two ~f:(fun acc n ->
        get_array3 (T.Int "4") acc (emit_xhp_child_decl ~unary n))

let emit_xhp_children_paren_expr c =
  let (l, op_num) =
    match c with
    | Aast.ChildList l -> (l, xhp_child_op_to_int None)
    | Aast.ChildUnary (Aast.ChildList l, op) ->
      (l, xhp_child_op_to_int @@ Some op)
    | _ ->
      failwith
      @@ "Xhp children declarations cannot be plain id, "
      ^ "plain binary or unary without an inside list"
  in
  let arr = emit_xhp_children_decl_expr ~unary:"0" l in
  get_array3 (T.Int (string_of_int op_num)) (T.Int "5") arr

let emit_xhp_children_array children =
  match children with
  | [] -> T.Int "0"
  | [(Aast.ChildName (_, n) as c)] ->
    begin
      match String.lowercase n with
      | "empty" -> T.Int "0"
      | "any" -> T.Int "1"
      | _ -> emit_xhp_children_paren_expr c
    end
  | [c] -> emit_xhp_children_paren_expr c
  | _ -> failwith "HHVM does not support multiple children declarations"

(* AST transformations taken from hphp/parser/hphp.y *)
let from_children_declaration (ast_class : Tast.class_) (child_pos, children) =
  (* return children; *)
  let annot = Tast_annotate.null_annotation p in
  let children_arr = (annot, emit_xhp_children_array children) in
  let token1 = (child_pos, T.Return (Some children_arr)) in
  let body = [token1] in
  let m =
    xhp_attribute_declaration_method
      ~p:child_pos
      "__xhpChildrenDeclaration"
      false
      false
      false
      Aast.Protected
      body
  in
  Emit_method.from_ast ast_class m

let get_category_array categories =
  (* TODO: is this always 1? *)
  List.map
    ~f:(fun s ->
      (Tast_annotate.make (T.String s), Tast_annotate.make (T.Int "1")))
    categories

(* AST transformations taken from hphp/parser/hphp.y *)
let from_category_declaration ast_class (cat_pos, categories) =
  let annot = Tast_annotate.null_annotation p in
  let category_arr = (annot, T.Darray (None, get_category_array categories)) in
  let token1 = (p, T.Return (Some category_arr)) in
  let body = [token1] in
  let m =
    xhp_attribute_declaration_method
      ~p:cat_pos
      "__xhpCategoryDeclaration"
      false
      false
      false
      Aast.Protected
      body
  in
  Emit_method.from_ast ast_class m
