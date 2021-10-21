(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

let parse = function
  | "flag" -> Some { mode = FlagTargets }
  | "dump" -> Some { mode = DumpConstraints }
  | "simplify" -> Some { mode = SimplifyConstraints }
  | "solve" -> Some { mode = SolveConstraints }
  | _ -> None
