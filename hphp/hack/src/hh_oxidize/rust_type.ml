(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Utils

type lifetime = string

let lifetime x = x

type t =
  | Var of string
  | Ref of (lifetime * t)
  | Type of {
      name: string;
      lifetimes: lifetime list;
      params: t list;
    }

let rust_type_var (v : string) : t = Var v

let rust_ref (lt : lifetime) (inner : t) : t = Ref (lt, inner)

let rust_type (name : string) (lifetimes : lifetime list) (params : t list) : t
    =
  Type { name; lifetimes; params }

let rust_simple_type (name : string) : t = rust_type name [] []

let is_ref (ty : t) : bool =
  match ty with
  | Ref _ -> true
  | _ -> false

let rec contains_ref (ty : t) : bool =
  match ty with
  | Var _ -> false
  | Ref _ -> true
  | Type { name = _; lifetimes = []; params } ->
    List.fold ~f:(fun a p -> a || is_ref p || contains_ref p) ~init:false params
  | Type _ -> true

let rec rust_type_to_string (ty : t) : string =
  match ty with
  | Var v -> v
  | Ref (lt, t) -> sprintf "&'%s %s" lt (rust_type_to_string t)
  | Type { name = "[]"; lifetimes = _; params } ->
    sprintf "[%s]" (map_and_concat ~f:rust_type_to_string ~sep:"," params)
  | Type { name = "()"; lifetimes = _; params } ->
    sprintf "(%s)" (map_and_concat ~f:rust_type_to_string ~sep:"," params)
  | Type { name; lifetimes = []; params = [] } -> name
  | Type { name; lifetimes; params } ->
    sprintf "%s%s" name (type_params_to_string lifetimes params)

and type_params_to_string
    ?(bound : string = "") (lts : lifetime list) (tys : t list) : string =
  let bound =
    if String.is_empty bound then
      bound
    else
      ":" ^ bound
  in
  let lts = map_and_concat ~f:(sprintf "'%s") ~sep:"," lts in
  let ps =
    map_and_concat ~f:(fun ty -> rust_type_to_string ty ^ bound) ~sep:"," tys
  in
  sprintf
    "<%s%s%s>"
    lts
    ( if String.is_empty lts || String.is_empty ps then
      ""
    else
      "," )
    ps

let deref (ty : t) : t =
  match ty with
  | Ref (_, t) -> t
  | _ -> ty

let rec type_name_and_params (ty : t) : string * t list =
  match ty with
  | Var n -> (n, [])
  | Ref (_, r) -> type_name_and_params r
  | Type { name; lifetimes = _; params } -> (name, params)

let is_var (ty : t) : bool =
  match ty with
  | Var _ -> true
  | _ -> false
