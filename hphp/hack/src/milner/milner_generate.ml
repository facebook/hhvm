(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let defs = ref []

type primitive =
  | PInt
  | PString
  | PFloat
  | PBool
  | PArraykey
  | PNum
[@@deriving enum]

type kind =
  | KPrimitive
  | KClass
  | KAlias
  | KNewtype
[@@deriving enum]

type ty =
  | TPrimitive of primitive
  | TClass of { name: string }
  | TAlias of {
      name: string;
      aliased: ty;
    }
  | TNewtype of {
      name: string;
      producer: string;
    }

let name_ctr = ref 0

let fresh prefix =
  let n = !name_ctr in
  name_ctr := !name_ctr + 1;
  prefix ^ string_of_int n

let show_ty ty =
  match ty with
  | TPrimitive prim -> begin
    match prim with
    | PInt -> "int"
    | PString -> "string"
    | PFloat -> "float"
    | PBool -> "bool"
    | PArraykey -> "arraykey"
    | PNum -> "num"
  end
  | TClass info -> info.name
  | TAlias info -> info.name
  | TNewtype info -> info.name

let rec expr_for = function
  | TPrimitive prim -> begin
    match prim with
    | PInt -> "42"
    | PString -> "'apple'"
    | PFloat -> "42.0"
    | PBool -> "true"
    | PArraykey ->
      let prim = List.nth_exn [PInt; PString] (Random.int_incl 0 1) in
      expr_for (TPrimitive prim)
    | PNum ->
      let prim = List.nth_exn [PInt; PFloat] (Random.int_incl 0 1) in
      expr_for (TPrimitive prim)
  end
  | TClass info -> "new " ^ info.name ^ "()"
  | TAlias info -> expr_for info.aliased
  | TNewtype info -> info.producer ^ "()"

let rec ty () =
  let kind =
    Random.int_incl min_kind max_kind |> kind_of_enum |> Option.value_exn
  in
  match kind with
  | KPrimitive ->
    let prim =
      Random.int_incl min_primitive max_primitive
      |> primitive_of_enum
      |> Option.value_exn
    in
    TPrimitive prim
  | KClass ->
    let name = fresh "C" in
    let def = "class " ^ name ^ " {}" in
    defs := def :: !defs;
    TClass { name }
  | KAlias ->
    let name = fresh "A" in
    let aliased = ty () in
    let def = "type " ^ name ^ " = " ^ show_ty aliased ^ ";" in
    defs := def :: !defs;
    TAlias { name; aliased }
  | KNewtype ->
    let name = fresh "N" in
    let aliased = ty () in
    let def = "newtype " ^ name ^ " = " ^ show_ty aliased ^ ";" in
    defs := def :: !defs;
    let producer = fresh "mkNewtype" in
    let def =
      "function "
      ^ producer
      ^ "(): "
      ^ name
      ^ " { return "
      ^ expr_for aliased
      ^ "; }"
    in
    defs := def :: !defs;
    TNewtype { name; producer }
