(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ifc_types

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

let new_raw_options mode lattice =
  { ropt_mode = mode; ropt_security_lattice = lattice }

let new_options mode lattice =
  { opt_mode = mode; opt_security_lattice = lattice }
