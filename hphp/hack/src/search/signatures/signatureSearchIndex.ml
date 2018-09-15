(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

type query =
  | And of query list
  | Or of query list
  | Term of string

type t = (string, SSet.t) Hashtbl.t

let make () = Hashtbl.create 100

let rec query_helper index query =
  match query with
  | Term query ->
    begin match Hashtbl.find_opt index query with
    | Some set -> set
    | None -> SSet.empty
    end

  | And query_list ->
    begin match List.map query_list (query_helper index) with
    | hd :: tl -> List.fold tl ~init:hd ~f:SSet.inter
    | [] -> SSet.empty
    end

  | Or query_list ->
    query_list
    |> List.map ~f:(query_helper index)
    |> List.fold ~init:SSet.empty ~f:SSet.union

let get index query =
  SSet.elements (query_helper index query)

let update index name terms =
  List.iter terms ~f:(fun term ->
    let functions =
      match Hashtbl.find_opt index term with
      | Some set -> set
      | None -> SSet.empty
    in
    let functions = SSet.add name functions in
    Hashtbl.replace index term functions
  )
