(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Instruction_sequence
open Hhbc_ast

let emit_pos pos =
  if pos <> Pos.none then
    let (line_begin, line_end, col_begin, col_end) =
      Pos.info_pos_extended pos
    in
    instr (ISrcLoc { line_begin; line_end; col_begin; col_end })
  else
    empty

(* Emit srcloc prior to these instructions *)
let emit_pos_then pos instrs = gather [emit_pos pos; instrs]
