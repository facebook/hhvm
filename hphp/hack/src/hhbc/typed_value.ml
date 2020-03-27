(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* We introduce a type for Hack/PHP values, mimicking what happens at runtime.
 * Currently this is used for constant folding. By defining a special type, we
 * ensure independence from usage: for example, it can be used for optimization
 * on ASTs, or on bytecode, or (in future) on a compiler intermediate language.
 * HHVM takes a similar approach: see runtime/base/typed-value.h
 *)
type t =
  (* Used for fields that are initialized in the 86pinit method *)
  | Uninit
  (* Hack/PHP integers are 64-bit *)
  | Int of Int64.t
  | Bool of bool
  (* Both Hack/PHP and Caml floats are IEEE754 64-bit *)
  | Float of float
  | String of string
  | Null
  (* Classic PHP arrays with explicit (key,value) entries *)
  | HhasAdata of string
  | Array of (t * t) list
  | VArray of t list * prov_tag
  | DArray of (t * t) list * prov_tag
  (* Hack arrays: vectors, keysets, and dictionaries *)
  | Vec of t list * prov_tag
  | Keyset of t list
  | Dict of (t * t) list * prov_tag

and prov_tag = Pos.t option [@@deriving ord]

let compare : t -> t -> int = compare

let compare_prov_tag : prov_tag -> prov_tag -> int = compare_prov_tag

module TVMap : WrappedMap.S with type key = t = WrappedMap.Make (struct
  type key = t

  type t = key

  let compare : t -> t -> int = compare
end)

(* Some useful constants *)
let zero = Int Int64.zero

let null = Null

module StringOps = struct
  let bitwise_not s =
    let result = Bytes.create (String.length s) in
    Caml.String.iteri
      (fun i c ->
        (* keep only last byte *)
        let b = lnot (int_of_char c) land 0xFF in
        Bytes.set result i (char_of_int b))
      s;
    Bytes.to_string result
end

(* Cast to a boolean: the (bool) operator in PHP *)
let to_bool v =
  match v with
  | Uninit -> false (* Should not happen *)
  | Bool b -> b
  | Null -> false
  | String "" -> false
  | String "0" -> false
  | String _ -> true
  | Int i -> i <> Int64.zero
  | Float f -> f <> 0.0
  (* Empty collections cast to false *)
  | Dict ([], _)
  | Array []
  | VArray ([], _)
  | DArray ([], _)
  | Keyset []
  | Vec ([], _) ->
    false
  (* Non-empty collections cast to true *)
  | HhasAdata _
  | Dict (_, _)
  | Array _
  | VArray _
  | DArray _
  | Keyset _
  | Vec (_, _) ->
    true

(* try to convert numeric *)
let string_to_int_opt ~allow_inf s =
  let int_opt = (try Some (Int64.of_string s) with _ -> None) in
  match int_opt with
  | None ->
    (try
       let s = float_of_string s in
       if (not allow_inf) && (s = Float.infinity || s = Float.neg_infinity) then
         None
       else
         Some (Int64.of_float s)
     with _ -> None)
  | x -> x

(* Cast to an integer: the (int) operator in PHP. Return None if we can't
 * or won't produce the correct value *)
let to_int v =
  match v with
  | Uninit -> None (* Should not happen *)
  (* Unreachable - the only callsite of to_int is cast_to_arraykey, which never
   * calls it with String *)
  | String _ -> None
  | Int i -> Some i
  | Float f ->
    let fpClass = Float.classify f in
    begin
      match fpClass with
      (* Here's a handy dandy chart of all possible values based on language
       * | float | PHP 5   | HHVM    | PHP 7
       * ----------------------------------------
       * |  NaN  | int_min | int_min | 0
       * |  INF  | int_min |  0      | 0
       * | -INF  | int_min | int_min | 0
       * For NaN, the value is min_int in HHVM
       * For positive infinity, the value is 0 in HHVM
       * For negative infinity the value is min_int in HHVM
       * For PHP7, the value is always 0
       * Thus if the float is infinity OR we're in PHP7, set it to 0
       *)
      | Float.Class.Nan
      | Float.Class.Infinite ->
        if f = Float.infinity then
          Some Int64.zero
        else
          Some Caml.Int64.min_int
      | _ ->
        (* mimic double-to-int64.h *)
        let cast v = (try Some (Int64.of_float v) with Failure _ -> None) in
        if f >= 0.0 then
          if f < Int64.to_float Caml.Int64.max_int then
            cast f
          else
            Some 0L
        else
          cast f
    end
  | _ ->
    Some
      ( if to_bool v then
        Int64.one
      else
        Int64.zero )

(* Cast to a float: the (float) operator in PHP. Return None if we can't
 * or won't produce the correct value *)
let to_float v =
  match v with
  | Uninit -> None (* Should not happen *)
  | String _ ->
    None (* Not worth trying to replicate float printing sematics here *)
  | Int i -> (try Some (Int64.to_float i) with Failure _ -> None)
  | Float f -> Some f
  | _ ->
    Some
      ( if to_bool v then
        1.0
      else
        0.0 )

(* Cast to a string: the (string) operator in PHP. Return None if we can't
 * or won't produce the correct value *)
let to_string v =
  match v with
  | Uninit -> None (* Should not happen *)
  | Bool false -> Some ""
  | Bool true -> Some "1"
  | Null -> Some ""
  | Int i -> Some (Int64.to_string i)
  | String s -> Some s
  | Float _ ->
    None (* Not worth trying to replicate float printing sematics here *)
  | _ -> None

(* Integer operations. For now, we don't attempt to implement the
 * overflow-to-float semantics *)
let add_int i1 i2 = Some (Int (Int64.( + ) i1 i2))

let neg i =
  match i with
  | Int i -> Some (Int (Int64.neg i))
  | Float f -> Some (Float (0.0 -. f))
  | _ -> None

let sub_int i1 i2 = Some (Int (Int64.( - ) i1 i2))

(* Arithmetic. For now, only on pure integer or float operands *)
let sub v1 v2 =
  match (v1, v2) with
  | (Int i1, Int i2) -> sub_int i1 i2
  | (Float f1, Float f2) -> Some (Float (f1 -. f2))
  | _ -> None

let mul_int i1 i2 = Some (Int (Int64.( * ) i1 i2))

(* Arithmetic. For now, only on pure integer or float operands *)
let mul v1 v2 =
  match (v1, v2) with
  | (Int i1, Int i2) -> mul_int i1 i2
  | (Float f1, Float f2) -> Some (Float (f1 *. f2))
  | (Int i1, Float f2) -> Some (Float (Int64.to_float i1 *. f2))
  | (Float f1, Int i2) -> Some (Float (f1 *. Int64.to_float i2))
  | _ -> None

(* Arithmetic. For now, only on pure integer or float operands *)
let div v1 v2 =
  match (v1, v2) with
  | (Int i1, Int i2) when i2 <> 0L ->
    if Int64.rem i1 i2 = 0L then
      Some (Int (Int64.( / ) i1 i2))
    else
      Some (Float (Int64.to_float i1 /. Int64.to_float i2))
  | (Float f1, Float f2) when f2 <> 0.0 -> Some (Float (f1 /. f2))
  | (Int i1, Float f2) when f2 <> 0.0 -> Some (Float (Int64.to_float i1 /. f2))
  | (Float f1, Int i2) when i2 <> 0L -> Some (Float (f1 /. Int64.to_float i2))
  | _ -> None

(* Arithmetic. For now, only on pure integer or float operands *)
let add v1 v2 =
  match (v1, v2) with
  | (Float f1, Float f2) -> Some (Float (f1 +. f2))
  | (Int i1, Int i2) -> add_int i1 i2
  | (Int i1, Float f2) -> Some (Float (Int64.to_float i1 +. f2))
  | (Float f1, Int i2) -> Some (Float (f1 +. Int64.to_float i2))
  | (_, _) -> None

let shift_left v1 v2 =
  match (v1, v2) with
  | (Int i1, Int i2) when i2 >= 0L ->
    begin
      try
        let v = Int64.to_int_exn i2 in
        Some (Int (Int64.shift_left i1 v))
      with _ -> None
    end
  | _ -> None

(* Arithmetic. For now, only on pure integer operands *)
let bitwise_or v1 v2 =
  match (v1, v2) with
  | (Int i1, Int i2) -> Some (Int (Int64.( lor ) i1 i2))
  | _ -> None

(* String concatenation *)
let concat v1 v2 =
  match Option.both (to_string v1) (to_string v2) with
  | Some (l, r) -> Some (String (l ^ r))
  | None -> None

(* Bitwise operations. *)
let bitwise_not v =
  match v with
  | Int i -> Some (Int (Int64.lnot i))
  | String s -> Some (String (StringOps.bitwise_not s))
  | _ -> None

(* Logical operators *)
let not v = Some (Bool (not (to_bool v)))

let cast_to_string v = Option.map (to_string v) (fun x -> String x)

let cast_to_int v = Option.map (to_int v) (fun x -> Int x)

let cast_to_bool v = Some (Bool (to_bool v))

let cast_to_float v = Option.map (to_float v) (fun x -> Float x)

let cast_to_arraykey v =
  match v with
  | String s -> Some (String s)
  | Null -> Some (String "")
  | Uninit
  | Array _
  | VArray _
  | DArray _
  | Vec _
  | Keyset _
  | Dict _ ->
    None
  | _ -> cast_to_int v
