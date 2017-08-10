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

exception IncludeTimeFatalException of FatalOp.t * Pos.t * string

let raise_fatal_runtime p message =
  raise (IncludeTimeFatalException (FatalOp.Runtime, p, message))

let raise_fatal_parse p message =
  raise (IncludeTimeFatalException (FatalOp.Parse, p, message))

let emit_fatal op pos message =
  gather
  [
    Emit_pos.emit_pos pos;
    instr_string message;
    instr (IOp (Fatal op))
  ]

let emit_fatal_runtime pos message =
  emit_fatal FatalOp.Runtime pos message
let emit_fatal_runtimeomitframe pos message =
  emit_fatal FatalOp.RuntimeOmitFrame pos message

let emit_fatal_for_break_continue pos level =
  let suffix = if level = 1 then "" else "s" in
  let message =
    Printf.sprintf "Cannot break/continue %d level%s" level suffix
  in
  emit_fatal_runtime pos message
