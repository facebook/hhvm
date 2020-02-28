(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Longident
open Utils

type flattened_longident =
  | FLident of string list
  | FLdot of flattened_longident * string list
  | FLapply of flattened_longident list

let rec flatten_longident = function
  | Lident str -> FLident [str]
  | Lapply (id1, id2) ->
    let ids =
      match flatten_longident id1 with
      | FLapply ids -> ids
      | id -> [id]
    in
    FLapply (ids @ [flatten_longident id2])
  | Ldot (id, str) ->
    (match flatten_longident id with
    | FLident strs -> FLident (str :: strs)
    | FLdot (id, strs) -> FLdot (id, str :: strs)
    | FLapply _ as id -> FLdot (id, [str]))

let to_string for_open id =
  let rec to_string id =
    match id with
    | FLident []
    | FLdot (_, [])
    | FLapply [] ->
      assert false
    | FLident (ty :: modules) ->
      let ty =
        match (ty, modules) with
        | _ when for_open -> convert_module_name ty
        | ("t", m :: _) -> convert_type_name m
        | _ -> convert_type_name ty
      in
      let modules = List.map modules convert_module_name in
      ty :: modules |> List.rev |> String.concat ~sep:"::"
    | FLdot (id, assoc_tys) ->
      let id = to_string id in
      let assoc_tys = List.map assoc_tys convert_type_name in
      assoc_tys |> List.rev |> List.cons id |> String.concat ~sep:"::"
    | FLapply (ftor :: args) ->
      let ftor = to_string ftor in
      let args = args |> List.map ~f:to_string |> String.concat ~sep:", " in
      sprintf "%s<%s>" ftor args
  in
  to_string id

let longident_to_string ?(for_open = false) id =
  flatten_longident id |> to_string for_open
