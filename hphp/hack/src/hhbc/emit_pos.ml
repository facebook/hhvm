(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Instruction_sequence
open Hhbc_ast

let emit_pos pos =
  if Hhbc_options.source_mapping !Hhbc_options.compiler_options
  then instr (ISrcLoc pos)
  else empty

(* Emit srcloc prior to these instructions *)
let emit_pos_then pos instrs =
  gather [emit_pos pos; instrs]
