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

(* Literal expressions can be converted into values *)
let expr_to_typed_value (_, expr_) =
  match expr_ with
  | A.Int (_, s) -> Some (TV.Int (Int64.of_string s))
  | A.True -> Some (TV.Bool true)
  | A.False -> Some (TV.Bool false)
  | A.Null -> Some TV.null
  | A.String (_, s) -> Some (TV.String s)
  | A.Float (_, s) -> Some (TV.Float (float_of_string s))
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

  (* Unary operations. `this` is the parent expression. We don't use `env` *)
  method! on_Unop env _this unop e =
  let e = self#on_expr env e in
  let default () = super#on_Unop env (A.Unop(unop, e)) unop e in
  match expr_to_typed_value e with
  | None -> default ()
  | Some v ->
    match unop_on_value unop v with
    | None -> default ()
    | Some result -> value_to_expr (fst e) result

  (* Binary operations. *)
  method! on_Binop env _this binop e1 e2 =
  let e1 = self#on_expr env e1 in
  let e2 = self#on_expr env e2 in
  let default () = super#on_Binop env (A.Binop(binop, e1, e2)) binop e1 e2 in
  match expr_to_typed_value e1, expr_to_typed_value e2 with
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
