(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type def = string

let show_def def = def

type primitive =
  | PNull
  | PInt
  | PString
  | PFloat
  | PBool
  | PArraykey
  | PNum
[@@deriving enum, ord]

type kind =
  | KMixed
  | KPrimitive
  | KOption
  | KClass
  | KAlias
  | KNewtype
  | KCase
[@@deriving enum]

type ty =
  | TMixed
  | TPrimitive of primitive
  | TOption of ty
  | TClass of { name: string }
  | TAlias of {
      name: string;
      aliased: ty;
    }
  | TNewtype of {
      name: string;
      producer: string;
    }
  | TCase of {
      name: string;
      disjuncts: ty list;
    }

let name_ctr = ref 0

let fresh prefix =
  let n = !name_ctr in
  name_ctr := !name_ctr + 1;
  prefix ^ string_of_int n

let pick l = List.length l - 1 |> Random.int_incl 0 |> List.nth_exn l

let rec show_ty ty =
  match ty with
  | TMixed -> "mixed"
  | TPrimitive prim -> begin
    match prim with
    | PNull -> "null"
    | PInt -> "int"
    | PString -> "string"
    | PFloat -> "float"
    | PBool -> "bool"
    | PArraykey -> "arraykey"
    | PNum -> "num"
  end
  | TOption ty -> "?" ^ show_ty ty
  | TClass info -> info.name
  | TAlias info -> info.name
  | TNewtype info -> info.name
  | TCase info -> info.name

let are_disjoint_prims prim prim' =
  let expand_prim = function
    | PArraykey -> [PInt; PString]
    | PNum -> [PInt; PFloat]
    | prim -> [prim]
  in
  let prims = expand_prim prim @ expand_prim prim' in
  let raw_length = List.length prims in
  let dedup_length =
    List.dedup_and_sort ~compare:compare_primitive prims |> List.length
  in
  raw_length = dedup_length

let rec are_disjoint_tys ty ty' =
  match (ty, ty') with
  | (TMixed, _)
  | (_, TMixed) ->
    false
  | (TNewtype _, _)
  | (_, TNewtype _) ->
    (* Opaque newtypes (which is all we represent at the moment) cannot be
       statically checked to be disjoint since the typechecker cannot see the
       underlying type *)
    false
  | (TAlias info, ty') -> are_disjoint_tys info.aliased ty'
  | (ty, TAlias info) -> are_disjoint_tys ty info.aliased
  | (TCase info, ty)
  | (ty, TCase info) ->
    List.for_all info.disjuncts ~f:(are_disjoint_tys ty)
  | (TPrimitive prim, TPrimitive prim') -> are_disjoint_prims prim prim'
  | (TOption ty, ty')
  | (ty', TOption ty) ->
    are_disjoint_tys (TPrimitive PNull) ty' && are_disjoint_tys ty ty'
  | (TClass _, _)
  | (_, TClass _) ->
    (* Each class is distinct from any other type because we generate a fresh
       class definition each time we generate a new type. *)
    true

let rec expr_for = function
  | TMixed ->
    let (ty, defs) = ty () in
    let (str, defs') = expr_for ty in
    (str, defs @ defs')
  | TPrimitive prim -> begin
    match prim with
    | PNull -> ("null", [])
    | PInt -> ("42", [])
    | PString -> ("'apple'", [])
    | PFloat -> ("42.0", [])
    | PBool -> ("true", [])
    | PArraykey ->
      let prim = pick [PInt; PString] in
      expr_for (TPrimitive prim)
    | PNum ->
      let prim = pick [PInt; PFloat] in
      expr_for (TPrimitive prim)
  end
  | TOption ty -> expr_for @@ pick [TPrimitive PNull; ty]
  | TClass info -> ("new " ^ info.name ^ "()", [])
  | TAlias info -> expr_for info.aliased
  | TNewtype info -> (info.producer ^ "()", [])
  | TCase info -> expr_for (pick info.disjuncts)

and ty () =
  let kind =
    Random.int_incl min_kind max_kind |> kind_of_enum |> Option.value_exn
  in
  match kind with
  | KMixed -> (TMixed, [])
  | KPrimitive ->
    let prim =
      Random.int_incl min_primitive max_primitive
      |> primitive_of_enum
      |> Option.value_exn
    in
    (TPrimitive prim, [])
  | KOption ->
    let rec candidate () =
      match ty () with
      | (TMixed, _)
      | (TOption _, _) ->
        (* Due to some misguided checks the parser and the typechecker has. We
           need to eliminate these cases. *)
        candidate ()
      | res -> res
    in
    let (ty, defs) = candidate () in
    (TOption ty, defs)
  | KClass ->
    let name = fresh "C" in
    let def_class = "class " ^ name ^ " {}" in
    (TClass { name }, [def_class])
  | KAlias ->
    let name = fresh "A" in
    let (aliased, defs) = ty () in
    let def_alias = "type " ^ name ^ " = " ^ show_ty aliased ^ ";" in
    (TAlias { name; aliased }, def_alias :: defs)
  | KNewtype ->
    let name = fresh "N" in
    let (aliased, defs) = ty () in
    let def_newtype = "newtype " ^ name ^ " = " ^ show_ty aliased ^ ";" in
    let producer = fresh "mkNewtype" in
    let (aliased_expr, expr_defs) = expr_for aliased in
    let def_maker =
      "function "
      ^ producer
      ^ "(): "
      ^ name
      ^ " { return "
      ^ aliased_expr
      ^ "; }"
    in
    (TNewtype { name; producer }, def_newtype :: def_maker :: (expr_defs @ defs))
  | KCase ->
    let name = fresh "CT" in
    let rec add_disjuncts (disjuncts, defs) =
      if Random.bool () then
        let (disjunct, defs') = ty () in
        if List.for_all disjuncts ~f:(are_disjoint_tys disjunct) then
          add_disjuncts @@ (disjunct :: disjuncts, defs' @ defs)
        else
          add_disjuncts (disjuncts, defs)
      else
        (disjuncts, defs)
    in
    let (ty, defs) = ty () in
    let (disjuncts, defs) = add_disjuncts ([ty], defs) in
    let def_case_type =
      "case type "
      ^ name
      ^ " = "
      ^ String.concat ~sep:" | " (List.map ~f:show_ty disjuncts)
      ^ ";"
    in
    (TCase { name; disjuncts }, def_case_type :: defs)
