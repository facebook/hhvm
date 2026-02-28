(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Refactor_sd_types

let parse_analysis_mode = function
  | "flag" -> Some FlagTargets
  | "dump" -> Some DumpConstraints
  | "simplify" -> Some SimplifyConstraints
  | "solve" -> Some SolveConstraints
  | _ -> None

let parse_refactor_mode = function
  | "Class" -> Some Class
  | "Function" -> Some Function
  | _ -> None

let mk ~analysis_mode ~refactor_mode = { analysis_mode; refactor_mode }
