(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
module A = Ast
module TV = Typed_value
module SN = Naming_special_names
module SU = Hhbc_string_utils
module TVL = Unique_list_typed_value
open Ast_class_expr
open Core

(* Literal expressions can be converted into values *)
let rec expr_to_typed_value ns (_, expr_) =
  match expr_ with
  | A.Int (_, s) -> TV.Int (Int64.of_string s)
  | A.True -> TV.Bool true
  | A.False -> TV.Bool false
  | A.Null -> TV.null
  | A.String (_, s) -> TV.String s
  | A.Float (_, s) -> TV.Float (float_of_string s)
  | A.Id (_, id) when id = "NAN" -> TV.Float nan
  | A.Id (_, id) when id = "INF" -> TV.Float infinity
  | A.Array fields -> array_to_typed_value ns fields
  | A.Collection ((_, "vec"), fields) ->
    TV.Vec (List.map fields (value_afield_to_typed_value ns))
  | A.Collection ((_, "keyset"), fields) ->
    let l = List.fold_left fields
      ~f:(fun l x -> TVL.add l (value_afield_to_typed_value ns x))
      ~init:TVL.empty in
    TV.Keyset (List.rev (TVL.items l))
  | A.Collection ((_, ("dict" | "ImmMap" | "Map")), fields) ->
    (* TODO: avoid quadratic-time behaviour *)
    let d = List.fold_left fields
      ~f:(fun d x ->
        let k, v = afield_to_typed_value_pair ns x in
        Typed_value.add_to_dict d k v) ~init:[] in
    TV.Dict d
  | A.Class_const(cid, (_, id)) when id = SN.Members.mClass ->
    let cexpr, _ = expr_to_class_expr [] (id_to_expr cid) in
    begin match cexpr with
    | Class_id cid ->
      let fq_id, _ = Hhbc_id.Class.elaborate_id ns cid in
      TV.String (Hhbc_id.Class.to_raw_string fq_id)
    | _ -> failwith "expr_to_typed_value: not a literal"
    end
  | _ ->
    failwith "expr_to_typed_value: not a literal"

and array_to_typed_value ns fields =
  let pairs, _ =
    List.fold_left fields ~init:([], Int64.zero)
      ~f:(fun (pairs, maxindex) afield ->
        match afield with
          (* Special treatment for explicit integer key *)
        | A.AFkvalue ((_, A.Int (_, s)), value) ->
          let newindex = Int64.of_string s in
          (TV.Int newindex, expr_to_typed_value ns value) :: pairs,
            Int64.add (if Int64.compare newindex maxindex > 0
            then newindex else maxindex) Int64.one
        | A.AFkvalue (key, value) ->
          (key_expr_to_typed_value ns key, expr_to_typed_value ns value)
            :: pairs,
          maxindex
        | A.AFvalue value ->
          (TV.Int maxindex, expr_to_typed_value ns value) :: pairs,
            Int64.add maxindex Int64.one)
  in TV.Array (List.rev pairs)

and key_expr_to_typed_value ns expr =
  let tv = expr_to_typed_value ns expr in
  match TV.cast_to_arraykey tv with
  | Some tv -> tv
  | None -> failwith "key_expr_to_typed_value: invalid key type"

and array_afield_to_typed_value_pair ns index afield =
  match afield with
  | A.AFvalue e ->
    (TV.Int (Int64.of_int index), expr_to_typed_value ns e)
  | A.AFkvalue (key, value) ->
    (key_expr_to_typed_value ns key, expr_to_typed_value ns value)

and afield_to_typed_value_pair ns afield =
  match afield with
  | A.AFvalue (_key, _value) ->
    failwith "afield_to_typed_value_pair: unexpected value"
  | A.AFkvalue (key, value) ->
    (key_expr_to_typed_value ns key, expr_to_typed_value ns value)

and value_afield_to_typed_value ns afield =
  match afield with
  | A.AFvalue e -> expr_to_typed_value ns e
  | A.AFkvalue (_key, _value) ->
    failwith "value_afield_to_typed_value: unexpected key=>value"

let literal_from_expr ns expr = expr_to_typed_value ns expr

let literals_from_exprs_with_index ns exprs =
  List.concat @@ List.mapi exprs (fun index e ->
    [TV.Int (Int64.of_int index); literal_from_expr ns e])

let expr_to_opt_typed_value ns e =
  try Some (expr_to_typed_value ns e) with Failure _ -> None

(* Any value can be converted into a literal expression *)
let rec value_to_expr_ p v =
  match v with
  | TV.Uninit -> failwith "value_to_expr: uninit value"
  | TV.Null -> A.Null
  | TV.Int i -> A.Int (p, Int64.to_string i)
  | TV.Bool false -> A.False
  | TV.Bool true -> A.True
  | TV.String s -> A.String (p, s)
  | TV.Float f -> A.Float (p, SU.Float.to_string f)
  | TV.Vec values -> A.Varray (List.map values (value_to_expr p))
  | TV.Keyset _values -> failwith "value_to_expr: keyset NYI"
  | TV.Array pairs -> A.Array (List.map pairs (value_pair_to_afield p))
  | TV.Dict pairs ->
    A.Darray (List.map pairs
      (fun (v1, v2) -> (value_to_expr p v1, value_to_expr p v2)))
and value_to_expr p v =
  (p, value_to_expr_ p v)
and value_pair_to_afield p (v1, v2) =
  A.AFkvalue (value_to_expr p v1, value_to_expr p v2)

(* Apply a unary operation on a typed value v.
 * Return None if we can't or won't determine the result *)
let unop_on_value unop v =
  match unop with
  | A.Unot -> TV.not v
  | A.Uplus -> TV.add TV.zero v
  | A.Uminus -> TV.sub TV.zero v
  | A.Utild -> TV.bitwise_not v
  | _ -> None

(* Likewise for binary operations *)
let binop_on_values binop v1 v2 =
  match binop with
  | A.Dot -> TV.concat v1 v2
  | A.Plus -> TV.add v1 v2
  | A.Minus -> TV.sub v1 v2
  | A.Star -> TV.mul v1 v2
  | A.Slash -> TV.div v1 v2
  | A.Percent -> TV.rem v1 v2
  | A.Amp -> TV.bitwise_and v1 v2
  | A.Bar -> TV.bitwise_or v1 v2
  | A.Xor -> TV.bitwise_xor v1 v2
  | A.AMpamp -> TV.logical_and v1 v2
  | A.BArbar -> TV.logical_or v1 v2
  | A.Eqeq -> TV.eqeq v1 v2
  | A.EQeqeq -> TV.eqeqeq v1 v2
  | A.Diff -> TV.diff v1 v2
  | A.Diff2 -> TV.diff2 v1 v2
  | A.Gtgt -> TV.shift_right v1 v2
  | A.Ltlt -> TV.shift_left v1 v2
  | A.Gt -> TV.greater_than v1 v2
  | A.Gte -> TV.greater_than_equals v1 v2
  | A.Lt -> TV.less_than v1 v2
  | A.Lte -> TV.less_than_equals v1 v2
  | _ -> None

(* try to apply type cast to a value *)
let cast_value hint v =
  match hint with
  | A.Happly((_, id), []) ->
    if id = SN.Typehints.int || id = SN.Typehints.integer
    then TV.cast_to_int v
    else
    if id = SN.Typehints.bool || id = SN.Typehints.boolean
    then TV.cast_to_bool v
    else
    if id = SN.Typehints.string
    then TV.cast_to_string v
    else if id = SN.Typehints.real ||
            id = SN.Typehints.double ||
            id = SN.Typehints.float
    then TV.cast_to_float v
    else None
  | _ -> None

(* We build a visitor over the syntax tree that recursively transforms unary and
 * binary operations on literal expressions.
 * NOTE: although it will exhaustively transform something like 2+(3*4), it does
 * so by converting 3*4 into a Typed_value.Int 12, then back to a literal 12, before
 * transforming this back into a Typed_value.t in order to compute the addition.
 * In future we might try and maintain typed values and avoid going back to
 * expressions. *)
let folder_visitor =
object (self)
  inherit [_] Ast_visitors.endo as super

  method! on_class_ _env cd =
    super#on_class_ cd.Ast.c_namespace cd

  method! on_fun_ _env fd =
    super#on_fun_ fd.Ast.f_namespace fd

  (* Type casts. cast_expr is A.Cast(hint, e) *)
  method! on_Cast env cast_expr hint e =
    let enew = self#on_expr env e in
    let default () =
      if enew == e
      then cast_expr
      else A.Cast(hint, enew) in
    match expr_to_opt_typed_value env enew with
    | None -> default ()
    | Some v ->
      match cast_value (snd hint) v with
      | None -> default ()
      | Some v -> value_to_expr_ (fst e) v

  (* Unary operations. unop_expr is A.Unop(unop, e) *)
  method! on_Unop env unop_expr unop e =
    let enew = self#on_expr env e in
    let default () =
      if enew == e
      then unop_expr
      else A.Unop(unop, enew) in
    match expr_to_opt_typed_value env enew with
    | None -> default ()
    | Some v ->
      match unop_on_value unop v with
      | None -> default ()
      | Some result -> value_to_expr_ (fst e) result

  (* Binary operations. binop_expr is A.Binop(binop, e1, e2) *)
  method! on_Binop env binop_expr binop e1 e2 =
    let e1new = self#on_expr env e1 in
    let e2new = self#on_expr env e2 in
    let default () =
      if e1new == e1 && e2new == e2
      then binop_expr
      else (A.Binop(binop, e1new, e2new)) in
    match expr_to_opt_typed_value env e1new,
          expr_to_opt_typed_value env e2new with
    | Some v1, Some v2 ->
      begin match binop_on_values binop v1 v2 with
      | None -> default ()
      | Some result -> value_to_expr_ (fst e1) result
      end
    | _, _ -> default ()

  method on_Goto _ parent _ = parent
  method on_GotoLabel _ parent _ = parent
end

let fold_expr ns e =
  folder_visitor#on_expr ns e
let fold_function fd =
  folder_visitor#on_fun_ fd.Ast.f_namespace fd
let fold_stmt ns s =
  folder_visitor#on_stmt ns s
let fold_gconst c =
  folder_visitor#on_gconst c.Ast.cst_namespace c
let fold_class_elt ns ce =
  folder_visitor#on_class_elt ns ce
