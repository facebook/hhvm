(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ifc_types
module Lattice = Ifc_security_lattice

exception Invalid_ifc_mode of string

let parse_mode_exn mode_str =
  match String.uppercase mode_str with
  | "LATTICE" -> Mlattice
  | "DECL" -> Mdecl
  | "ANALYSE" -> Manalyse
  | "SOLVE" -> Msolve
  | "CHECK" -> Mcheck
  | "DEBUG" -> Mdebug
  | _ -> raise @@ Invalid_ifc_mode mode_str

let parse ~mode ~lattice =
  try
    let opt_mode = parse_mode_exn mode in
    let opt_security_lattice = Lattice.mk_exn lattice in
    Ok { opt_mode; opt_security_lattice }
  with
  | Lattice.Invalid_security_lattice ->
    Error
      ( "option error: lattice specification should be basic flux "
      ^ "constraints, e.g., `A < B` separated by `;`" )
  | Invalid_ifc_mode mode ->
    Error (Printf.sprintf "option error: %s is not a recognised mode" mode)
