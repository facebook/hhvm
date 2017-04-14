(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* We introduce a type for Hack/PHP values, mimicking what happens at runtime.
 * Currently this is used for constant folding. By defining a special type, we
 * ensure independence from usage: for example, it can be used for optimization
 * on ASTs, or on bytecode, or (in future) on a compiler intermediate language.
 * HHVM takes a similar approach: see runtime/base/typed-value.h
 *)
type t =
  (* Hack/PHP integers are 64-bit *)
  | Int of Int64.t
  | Bool of bool
  (* Both Hack/PHP and Caml floats are IEEE754 64-bit *)
  | Float of float
  | String of string
  | Null

(* Some useful constants *)
let zero = Int Int64.zero
let null = Null
let one = Int Int64.one

(* Cast to a boolean: the (bool) operator in PHP *)
let to_bool v =
  match v with
  | Bool b -> b
  | Null -> false
  | String "" -> false
  | String "0" -> false
  | String _ -> true
  | Int i -> i <> Int64.zero
  | Float f -> f <> 0.0

(* Cast to an integer: the (int) operator in PHP. Return None if we can't
 * or won't produce the correct value *)
let to_int v =
  match v with
  | Bool false -> Some Int64.zero
  | Bool true -> Some Int64.one
  | Null -> Some Int64.zero
  | String s ->
    (* TODO: deal with strings whose initial prefix is digits *)
    (try Some (Int64.of_string s) with Failure _ -> None)
  | Int i -> Some i
  | Float f ->
    (* TODO: get this right. It's unlikely that Caml and PHP semantics match up *)
    (try Some (Int64.of_float f) with Failure _ -> None)

(* Cast to a float: the (float) operator in PHP. Return None if we can't
 * or won't produce the correct value *)
let to_float v =
  match v with
  | Bool false -> Some 0.0
  | Bool true -> Some 1.0
  | Null -> Some 0.0
  | String s ->
    (try Some (float_of_string s) with Failure _ -> None)
  | Int i ->
    (try Some (Int64.to_float i) with Failure _ -> None)
  | Float f -> Some f

(* Cast to a string: the (string) operator in PHP. Return None if we can't
 * or won't produce the correct value *)
let to_string v =
  match v with
  | Bool false -> ""
  | Bool true -> "1"
  | Null -> ""
  | Int i -> Int64.to_string i
  | String s -> s
  (* TODO: get this right. It's unlikely that Caml and PHP semantics match up *)
  | Float f -> string_of_float f

let ints_overflow_to_ints () =
  Hhbc_options.ints_overflow_to_ints !Emit_expression.compiler_options

(* Integer operations. For now, we don't attempt to implement the
 * overflow-to-float semantics *)
let add_int i1 i2 =
  if ints_overflow_to_ints ()
  then Some (Int (Int64.add i1 i2))
  else None

let sub_int i1 i2 =
  if ints_overflow_to_ints ()
  then Some (Int (Int64.sub i1 i2))
  else None

let mul_int i1 i2 =
  if ints_overflow_to_ints ()
  then Some (Int (Int64.mul i1 i2))
  else None

(* Arithmetic. For now, only on pure integer or float operands *)
let add v1 v2 =
  match v1, v2 with
  | Float f1, Float f2 -> Some (Float (f1 +. f2))
  | Int i1, Int i2 -> add_int i1 i2
  | _, _ -> None

let sub v1 v2 =
  match v1, v2 with
  | Float f1, Float f2 -> Some (Float (f1 -. f2))
  | Int i1, Int i2 -> sub_int i1 i2
  | _, _ -> None

let mul v1 v2 =
  match v1, v2 with
  | Float f1, Float f2 -> Some (Float (f1 *. f2))
  | Int i1, Int i2 -> mul_int i1 i2
  | _, _ -> None

let div v1 v2 =
  match v1, v2 with
  | Int left, Int right ->
    if Int64.rem left right = Int64.zero then
      Some (Int (Int64.div left right))
    else
      let left = Int64.to_float left in
      let right = Int64.to_float right in
      let quotient = left /. right in
      Some (Float quotient)
  | Float f1, Float f2 -> Some (Float (f1 /. f2))
  | _, _ -> None

(* String concatenation *)
let concat v1 v2 =
  Some (String (to_string v1 ^ to_string v2))

(* Boolean operations *)
let not v =
  Some (Bool (not (to_bool v)))

(* Bitwise operations. For now, only on integers *)
let bitwise_not v =
  match v with
  | Int i -> Some (Int (Int64.lognot i))
  | _ -> None

let bitwise_and v1 v2 =
  match v1, v2 with
  | Int i1, Int i2 -> Some (Int (Int64.logand i1 i2))
  | _ -> None

let bitwise_or v1 v2 =
  match v1, v2 with
  | Int i1, Int i2 -> Some (Int (Int64.logor i1 i2))
  | _ -> None

let bitwise_xor v1 v2 =
  match v1, v2 with
  | Int i1, Int i2 -> Some (Int (Int64.logxor i1 i2))
  | _ -> None
