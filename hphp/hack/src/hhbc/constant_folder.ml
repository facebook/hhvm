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
open Hhbc_ast

let literal_from_bool b =
  if b then True else False

let int64_to_bool i =
  not (Int64.equal i Int64.zero)

let lit_to_int lit =
  match lit with
  | Int _ -> lit
  | True -> Int Int64.one
  | Null
  | False -> Int Int64.zero
  | _ -> failwith "conversion from literal to integer not yet implemented"

let handle_integer_overflow i =
  (* TODO: Deal with integer overflow *)
  Int i

let fold_addition left right =
  match (left, right) with
  | (Int left, Int right) -> handle_integer_overflow (Int64.add left right)
  | _ -> failwith "Folding + not yet implemented"

let fold_subtraction left right =
  match (left, right) with
  | (Int left, Int right) -> handle_integer_overflow (Int64.sub left right)
  | _ -> failwith "Folding - not yet implemented"

let fold_multiplication left right =
  match (left, right) with
  | (Int left, Int right) -> handle_integer_overflow (Int64.mul left right)
  | _ -> failwith "Folding * not yet implemented"

let fold_exponentiation left right =
  match (left, right) with
  | _ -> failwith "Folding ** not yet implemented"

let fold_division left right =
  (* TODO: Deal with div by zero *)
  (* TODO: Deal with int.minval / -1 *)
  match (left, right) with
  | (Int left, Int right) ->
    if (Int64.rem left right) = Int64.zero then
      Int (Int64.div left right)
    else
      let left = Int64.to_float left in
      let right = Int64.to_float right in
      let quotient = left /. right in
      (* TODO: Consider making double take a float, not a string. *)
      (* TODO: The original HHVM emitter produces considerably more precision
      when printing out floats *)
      Double (string_of_float quotient)
  | _ -> failwith "Folding / not yet implemented"

let fold_remainder left right =
  (* TODO: Deal with div by zero *)
  match (left, right) with
  | (Int left, Int right) ->
    Int (Int64.rem left right)
  | _ -> failwith "Folding % not yet implemented"

let fold_concatenation left right =
  match (left, right) with
  | (Int left, Int right) ->
    String ((Int64.to_string left) ^ (Int64.to_string right))
  | _ -> failwith "Folding . not yet implemented"

let fold_right_shift left right =
  match (left, right) with
  | (Int left, Int right) ->
    (* TODO: Deal with overflow of shifter *)
    let right = Int64.to_int right in
    Int (Int64.shift_right left right)
  | _ -> failwith "Folding >> not yet implemented"

let fold_left_shift left right =
  match (left, right) with
  | (Int left, Int right) ->
    (* TODO: Deal with overflow of shifter *)
    let right = Int64.to_int right in
    Int (Int64.shift_left left right)
  | _ -> failwith "Folding << not yet implemented"

let fold_logical_and left right =
  match (left, right) with
  | (Int left, Int right) ->
    if int64_to_bool left then
      literal_from_bool (int64_to_bool right)
    else
      False
  | _ -> failwith "Folding && not yet implemented"

let fold_logical_or left right =
  match (left, right) with
  | (Int left, Int right) ->
    if int64_to_bool left then
      True
    else
      literal_from_bool (int64_to_bool right)
  | _ -> failwith "Folding || not yet implemented"

let rec fold_and left right =
  match (left, right) with
  | (Int left, Int right) -> Int (Int64.logand left right)
  | (Null, _)
  | (False, _)
  | (True, _) -> fold_and (lit_to_int left) right
  | (_, Null)
  | (_ , False)
  | (_, True) -> fold_and left (lit_to_int right)
  | _ -> failwith "Folding & not yet implemented"

let fold_or left right =
  match (left, right) with
  | (Int left, Int right) -> Int (Int64.logor left right)
  | _ -> failwith "Folding | not yet implemented"

let fold_xor left right =
  match (left, right) with
  | (Int left, Int right) -> Int (Int64.logxor left right)
  | _ -> failwith "Folding ^ not yet implemented"

let fold_loose_equality left right =
  match (left, right) with
  | (Int left, Int right) -> literal_from_bool (Int64.equal left right)
  | _ -> failwith "Folding == not yet implemented"

let fold_strict_equality left right =
  match (left, right) with
  | (Int left, Int right) -> literal_from_bool (Int64.equal left right)
  | _ -> failwith "Folding === not yet implemented"

let fold_loose_inequality left right =
  match (left, right) with
  | (Int left, Int right) -> literal_from_bool (not (Int64.equal left right))
  | _ -> failwith "Folding != not yet implemented"

let fold_strict_inequality left right =
  match (left, right) with
  | (Int left, Int right) -> literal_from_bool (not (Int64.equal left right))
  | _ -> failwith "Folding !== not yet implemented"

let fold_less_than left right =
  match (left, right) with
  | (Int left, Int right) -> literal_from_bool ((Int64.compare left right) < 0)
  | _ -> failwith "Folding < not yet implemented"

let fold_less_than_equal left right =
  match (left, right) with
  | (Int left, Int right) -> literal_from_bool ((Int64.compare left right) <= 0)
  | _ -> failwith "Folding <= not yet implemented"

let fold_greater_than left right =
  match (left, right) with
  | (Int left, Int right) -> literal_from_bool ((Int64.compare left right) > 0)
  | _ -> failwith "Folding > not yet implemented"

let fold_greater_than_equal left right =
  match (left, right) with
  | (Int left, Int right) -> literal_from_bool ((Int64.compare left right) >= 0)
  | _ -> failwith "Folding >= not yet implemented"

let literal_from_binop op left right =
  (* TODO: HHVM does not allow 2+2 in an attribute, but does allow it in
  a constant initializer. It seems reasonable to allow this in attributes
  as well. Make sure this decision is documented in the specification. *)
  match op with
  | Ast.Eqeq -> fold_loose_equality left right
  | Ast.EQeqeq -> fold_strict_equality left right
  | Ast.Diff -> fold_loose_inequality left right
  | Ast.Diff2 -> fold_strict_inequality left right
  | Ast.Lt -> fold_less_than left right
  | Ast.Lte -> fold_less_than_equal left right
  | Ast.Gt -> fold_greater_than left right
  | Ast.Gte -> fold_greater_than_equal left right
  | Ast.Amp -> fold_and left right
  | Ast.Bar -> fold_or left right
  | Ast.Xor -> fold_xor left right
  | Ast.BArbar -> fold_logical_or left right
  | Ast.AMpamp -> fold_logical_and left right
  | Ast.Ltlt -> fold_left_shift left right
  | Ast.Gtgt -> fold_right_shift left right
  | Ast.Dot -> fold_concatenation left right
  | Ast.Plus -> fold_addition left right
  | Ast.Minus -> fold_subtraction left right
  | Ast.Star -> fold_multiplication left right
  | Ast.Starstar -> fold_exponentiation left right
  | Ast.Slash -> fold_division left right
  | Ast.Percent -> fold_remainder left right
  | Ast.Eq _ -> failwith "Unexpected assignment on constant"

let fold_not operand =
  match operand with
  | Int operand ->
    literal_from_bool (not (int64_to_bool operand))
  | _ -> failwith "Folding of ! not yet implemented"

let fold_binary_not operand =
  match operand with
  | Int operand -> Int (Int64.lognot operand)
  | _ -> failwith "Folding of ~ not yet implemented"

let fold_unary_plus operand =
  match operand with
  | Int _ -> operand
  | _ -> failwith "Folding of unary + not yet implemented"

let fold_unary_minus operand =
  match operand with
  | Int operand -> handle_integer_overflow (Int64.neg operand)
  | _ -> failwith "Folding of unary - not yet implemented"

let literal_from_unop op operand =
  match op with
  | Ast.Unot  -> fold_not operand
  | Ast.Utild -> fold_binary_not operand
  | Ast.Uplus -> fold_unary_plus operand
  | Ast.Uminus -> fold_unary_minus operand
  | Ast.Upincr
  | Ast.Uincr -> failwith "unexpected increment on constant"
  | Ast.Udecr
  | Ast.Updecr -> failwith "unexpected decrement on constant"
  | Ast.Uref -> failwith "unexpected reference on constant"

let rec collection_literal_fields fields =
  let folder (index, consts) field =
    match field with
    | Ast.AFvalue v ->
      (index + 1, literal_from_expr v :: Int (Int64.of_int index) :: consts)
    | Ast.AFkvalue (k, v) ->
      (index, literal_from_expr k :: literal_from_expr v :: consts )
  in
  List.rev @@ snd @@ List.fold_left fields ~init:(0, []) ~f:folder

and dictionary_literal fields =
  let num = List.length fields in
  let fields = collection_literal_fields fields in
  Dict (num, fields)

and array_literal fields =
  let num = List.length fields in
  let fields = collection_literal_fields fields in
  Array (num, fields)

and literal_from_expr expr =
  match snd expr with
  | Ast.Float (_, litstr) -> Double litstr
  | Ast.String (_, litstr) -> String litstr
  | Ast.Int (_, litstr) -> Int (Int64.of_string litstr)
  | Ast.Null -> Null
  | Ast.False -> False
  | Ast.True -> True
  | Ast.Array fields -> array_literal fields
  | Ast.Collection ((_, "dict"), fields) -> dictionary_literal fields
  (* TODO: Vec, Keyset, etc. *)
  | Ast.Binop (op, left, right) ->
    let left = literal_from_expr left in
    let right = literal_from_expr right in
    literal_from_binop op left right
  | Ast.Unop (op, operand) ->
    let operand = literal_from_expr operand in
    literal_from_unop op operand
  (* TODO: ??, ?:, others? *)
  | _ -> failwith "Expected a literal expression"

let literals_from_exprs_with_index exprs =
 List.rev @@ snd @@
 List.fold_left
   exprs
   ~init:(0, [])
   ~f:(fun (index, l) e ->
     (index + 1, literal_from_expr e :: Int (Int64.of_int index) :: l))
