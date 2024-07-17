(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let name_ctr = ref 0

let fresh prefix =
  let n = !name_ctr in
  name_ctr := !name_ctr + 1;
  prefix ^ string_of_int n

let select l = List.length l - 1 |> Random.int_incl 0 |> List.nth_exn l

module Definition = struct
  type t = string

  let show def = def
end

module Primitive = struct
  type t =
    | Null
    | Int
    | String
    | Float
    | Bool
    | Arraykey
    | Num
  [@@deriving enum, ord]

  let pick () = Random.int_incl min max |> of_enum |> Option.value_exn

  let are_disjoint prim prim' =
    let expand_prim = function
      | Arraykey -> [Int; String]
      | Num -> [Int; Float]
      | prim -> [prim]
    in
    let prims = expand_prim prim @ expand_prim prim' in
    let raw_length = List.length prims in
    let dedup_length = List.dedup_and_sort ~compare prims |> List.length in
    raw_length = dedup_length
end

module Kind = struct
  type t =
    | Mixed
    | Primitive
    | Option
    | Class
    | Alias
    | Newtype
    | Case
  [@@deriving enum]

  let pick () = Random.int_incl min max |> of_enum |> Option.value_exn
end

module Type = struct
  type t =
    | Mixed
    | Primitive of Primitive.t
    | Option of t
    | Class of { name: string }
    | Alias of {
        name: string;
        aliased: t;
      }
    | Newtype of {
        name: string;
        producer: string;
      }
    | Case of {
        name: string;
        disjuncts: t list;
      }

  let rec show ty =
    match ty with
    | Mixed -> "mixed"
    | Primitive prim -> begin
      let open Primitive in
      match prim with
      | Null -> "null"
      | Int -> "int"
      | String -> "string"
      | Float -> "float"
      | Bool -> "bool"
      | Arraykey -> "arraykey"
      | Num -> "num"
    end
    | Option ty -> "?" ^ show ty
    | Class info -> info.name
    | Alias info -> info.name
    | Newtype info -> info.name
    | Case info -> info.name

  let rec are_disjoint ty ty' =
    match (ty, ty') with
    | (Mixed, _)
    | (_, Mixed) ->
      false
    | (Newtype _, _)
    | (_, Newtype _) ->
      (* Opaque newtypes (which is all we represent at the moment) cannot be
         statically checked to be disjoint since the typechecker cannot see the
         underlying type *)
      false
    | (Alias info, ty') -> are_disjoint info.aliased ty'
    | (ty, Alias info) -> are_disjoint ty info.aliased
    | (Case info, ty)
    | (ty, Case info) ->
      List.for_all info.disjuncts ~f:(are_disjoint ty)
    | (Primitive prim, Primitive prim') -> Primitive.are_disjoint prim prim'
    | (Option ty, ty')
    | (ty', Option ty) ->
      are_disjoint (Primitive Primitive.Null) ty' && are_disjoint ty ty'
    | (Class _, _)
    | (_, Class _) ->
      (* Each class is distinct from any other type because we generate a fresh
         class definition each time we generate a new type. *)
      true

  let rec expr_of = function
    | Mixed ->
      let (ty, defs) = mk () in
      let (str, defs') = expr_of ty in
      (str, defs @ defs')
    | Primitive prim -> begin
      let open Primitive in
      match prim with
      | Null -> ("null", [])
      | Int -> ("42", [])
      | String -> ("'apple'", [])
      | Float -> ("42.0", [])
      | Bool -> ("true", [])
      | Arraykey ->
        let prim = select [Int; String] in
        expr_of (Primitive prim)
      | Num ->
        let prim = select [Int; Float] in
        expr_of (Primitive prim)
    end
    | Option ty -> expr_of @@ select [Primitive Primitive.Null; ty]
    | Class info -> ("new " ^ info.name ^ "()", [])
    | Alias info -> expr_of info.aliased
    | Newtype info -> (info.producer ^ "()", [])
    | Case info -> expr_of (select info.disjuncts)

  and mk () =
    match Kind.pick () with
    | Kind.Mixed -> (Mixed, [])
    | Kind.Primitive -> (Primitive (Primitive.pick ()), [])
    | Kind.Option ->
      let rec candidate () =
        match mk () with
        | (Mixed, _)
        | (Option _, _) ->
          (* Due to some misguided checks the parser and the typechecker has. We
             need to eliminate these cases. *)
          candidate ()
        | res -> res
      in
      let (ty, defs) = candidate () in
      (Option ty, defs)
    | Kind.Class ->
      let name = fresh "C" in
      let def_class = "class " ^ name ^ " {}" in
      (Class { name }, [def_class])
    | Kind.Alias ->
      let name = fresh "A" in
      let (aliased, defs) = mk () in
      let def_alias = "type " ^ name ^ " = " ^ show aliased ^ ";" in
      (Alias { name; aliased }, def_alias :: defs)
    | Kind.Newtype ->
      let name = fresh "N" in
      let (aliased, defs) = mk () in
      let def_newtype = "newtype " ^ name ^ " = " ^ show aliased ^ ";" in
      let producer = fresh "mkNewtype" in
      let (aliased_expr, expr_defs) = expr_of aliased in
      let def_maker =
        "function "
        ^ producer
        ^ "(): "
        ^ name
        ^ " { return "
        ^ aliased_expr
        ^ "; }"
      in
      ( Newtype { name; producer },
        def_newtype :: def_maker :: (expr_defs @ defs) )
    | Kind.Case ->
      let name = fresh "CT" in
      let rec add_disjuncts (disjuncts, defs) =
        if Random.bool () then
          let (disjunct, defs') = mk () in
          if List.for_all disjuncts ~f:(are_disjoint disjunct) then
            add_disjuncts @@ (disjunct :: disjuncts, defs' @ defs)
          else
            add_disjuncts (disjuncts, defs)
        else
          (disjuncts, defs)
      in
      let (ty, defs) = mk () in
      let (disjuncts, defs) = add_disjuncts ([ty], defs) in
      let def_case_type =
        "case type "
        ^ name
        ^ " = "
        ^ String.concat ~sep:" | " (List.map ~f:show disjuncts)
        ^ ";"
      in
      (Case { name; disjuncts }, def_case_type :: defs)
end
