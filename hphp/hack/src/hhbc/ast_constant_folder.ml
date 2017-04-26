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

(* Literal expressions can be converted into values *)
let expr_to_typed_value (_, expr_) =
  match expr_ with
  | A.Int (_, s) -> Some (TV.Int (Int64.of_string s))
  | A.True -> Some (TV.Bool true)
  | A.False -> Some (TV.Bool false)
  | A.Null -> Some TV.null
  | A.String (_, s) -> Some (TV.String s)
  | A.Float (_, s) -> Some (TV.Float (float_of_string s))
  | A.Id (_, id) when id = "NAN" -> Some (TV.Float nan)
  | A.Id (_, id) when id = "INF" -> Some (TV.Float infinity)
  | _ -> None

(* Any value can be converted into a literal expression *)
let value_to_expr p v =
  match v with
  | TV.Null -> A.Null
  | TV.Int i -> A.Int (p, Int64.to_string i)
  | TV.Bool false -> A.False
  | TV.Bool true -> A.True
  | TV.String s -> A.String (p, s)
  | TV.Float f -> A.Float (p, string_of_float f)

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
  | A.Amp -> TV.bitwise_and v1 v2
  | A.Bar -> TV.bitwise_or v1 v2
  | A.Xor -> TV.bitwise_xor v1 v2
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
    if id = SN.Typehints.int || id = SN.Typehints.integer then
      TV.cast_to_int v
    else if id = SN.Typehints.bool || id = SN.Typehints.boolean then
      TV.cast_to_bool v
    else if id = SN.Typehints.string then
      TV.cast_to_string v
    else if id = SN.Typehints.real ||
            id = SN.Typehints.double ||
            id = SN.Typehints.float then
      TV.cast_to_float v
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
  inherit [_] Ast_visitors.endo as _super

  (* Type casts. cast_expr is A.Cast(hint, e) *)
  method! on_Cast env cast_expr hint e =
    let enew = self#on_expr env e in
    let default () =
      if enew == e
      then cast_expr
      else A.Cast(hint, enew) in
    match expr_to_typed_value enew with
    | None -> default ()
    | Some v ->
      match cast_value (snd hint) v with
      | None -> default ()
      | Some v -> value_to_expr (fst e) v

  (* Unary operations. unop_expr is A.Unop(unop, e) *)
  method! on_Unop env unop_expr unop e =
    let enew = self#on_expr env e in
    let default () =
      if enew == e
      then unop_expr
      else A.Unop(unop, enew) in
    match expr_to_typed_value enew with
    | None -> default ()
    | Some v ->
      match unop_on_value unop v with
      | None -> default ()
      | Some result -> value_to_expr (fst e) result

  (* Binary operations. binop_expr is A.Binop(binop, e1, e2) *)
  method! on_Binop env binop_expr binop e1 e2 =
    let e1new = self#on_expr env e1 in
    let e2new = self#on_expr env e2 in
    let default () =
      if e1new == e1 && e2new == e2
      then binop_expr
      else (A.Binop(binop, e1new, e2new)) in
    match expr_to_typed_value e1new, expr_to_typed_value e2new with
    | Some v1, Some v2 ->
      begin match binop_on_values binop v1 v2 with
      | None -> default ()
      | Some result -> value_to_expr (fst e1) result
      end
    | _, _ -> default ()

  method on_Goto _ parent _ = parent
  method on_GotoLabel _ parent _ = parent
end

let fold_expr e =
  folder_visitor#on_expr () e
let fold_function fd =
  folder_visitor#on_fun_ () fd
let fold_method md =
  folder_visitor#on_method_ () md
let fold_stmt s =
  folder_visitor#on_stmt () s
let fold_gconst c =
  folder_visitor#on_gconst () c
