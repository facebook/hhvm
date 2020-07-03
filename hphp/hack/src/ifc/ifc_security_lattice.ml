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

exception Checking_error

let parse_policy = function
  | "PUBLIC" -> Pbot
  | "PRIVATE" -> Ptop
  | pur -> Ppurpose (String.uppercase pur)

(* Parses a Hasse diagram written in a ';' separated format,
 * e.g., "A < B; B < C; A < D"
 *)
let parse_exn str =
  String.filter ~f:(fun chr -> not @@ Char.equal ' ' chr) str
  |> String.split ~on:';'
  |> (fun xs ->
       if List.equal xs [""] ~equal:String.equal then
         []
       else
         xs)
  |> List.map ~f:(fun str ->
         match String.lsplit2 ~on:'<' str with
         | Some (l, r) -> (parse_policy l, parse_policy r)
         | None -> raise Invalid_security_lattice)
  |> FlowSet.of_list

let bounded_extension_of lattice =
  let add_top_bottom (l, r) acc =
    let bounds = FlowSet.of_list [(Pbot, l); (l, Ptop); (Pbot, r); (r, Ptop)] in
    FlowSet.union bounds acc
  in
  FlowSet.fold add_top_bottom lattice lattice |> FlowSet.add (Pbot, Ptop)

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

let mk_exn str = parse_exn str |> bounded_extension_of |> transitive_closure

let rec check_exn lattice = function
  | Ctrue -> []
  | Cflow flow ->
    if FlowSet.mem flow lattice then
      []
    else
      [flow]
  | Cconj (prop1, prop2) -> check_exn lattice prop1 @ check_exn lattice prop2
  | _ -> raise Checking_error
