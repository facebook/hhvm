(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Refactor_sd_types

let show_entity = function
  | Literal pos -> Format.asprintf "%a" Pos.pp pos
  | Variable var -> Format.sprintf "?%d" var

let show_constraint_ _env = function
  | Introduction pos -> Format.asprintf "Introduction at %a" Pos.pp pos
  | Upcast (ent, _) -> Format.asprintf "Upcast at %s" (show_entity ent)
  | Subset (sub, sup) -> show_entity sub ^ " ⊆ " ^ show_entity sup
  | Called pos -> Format.asprintf "Function call at %a" Pos.pp pos

let show_refactor_sd_result _env = function
  | Exists_Upcast pos -> Format.asprintf "Upcast exists at %a" Pos.pp pos
  | No_Upcast -> Format.asprintf "No upcast"
