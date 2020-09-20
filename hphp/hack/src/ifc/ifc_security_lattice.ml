(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ifc_types

(* This file contains code related to the security lattice we use to
 * check our constraint results against.
 *)

exception Invalid_security_lattice

let parse_policy pos purpose_str =
  match String.uppercase purpose_str with
  | "PUBLIC" -> Pbot pos
  | "PRIVATE" -> Ptop pos
  | purpose -> Ppurpose (pos, purpose)

(* Parses a Hasse diagram written in a ';' separated format,
 * e.g., "A < B; B < C; A < D"
 *)
let parse_exn str =
  let pos = PosSet.empty in
  String.filter ~f:(fun chr -> not @@ Char.equal ' ' chr) str
  |> String.split ~on:';'
  |> (fun xs ->
       if List.equal xs [""] ~equal:String.equal then
         []
       else
         xs)
  |> List.map ~f:(fun str ->
         match String.lsplit2 ~on:'<' str with
         | Some (l, r) -> (parse_policy pos l, parse_policy pos r)
         | None -> raise Invalid_security_lattice)
  |> FlowSet.of_list

(* A naive implementation of transitive closure *)
let rec transitive_closure set =
  let immediate_consequence (x, y) set =
    let add (y', z) set =
      if equal_policy y y' then
        FlowSet.add (x, z) set
      else
        set
    in
    FlowSet.fold add set set
  in
  let new_set = FlowSet.fold immediate_consequence set set in
  if FlowSet.cardinal new_set = FlowSet.cardinal set then
    set
  else
    transitive_closure new_set

let mk_exn str = parse_exn str |> transitive_closure
