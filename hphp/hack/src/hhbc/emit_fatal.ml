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

exception IncludeTimeFatalException of FatalOp.t * string

let raise_fatal_runtime message =
  raise (IncludeTimeFatalException (FatalOp.Runtime, message))

let raise_fatal_parse message =
  raise (IncludeTimeFatalException (FatalOp.Parse, message))

let emit_fatal op message =
  gather
  [
    instr_string message;
    instr (IOp (Fatal op))
  ]

let emit_fatal_runtime message = emit_fatal FatalOp.Runtime message
let emit_fatal_runtimeomitframe message =
  emit_fatal FatalOp.RuntimeOmitFrame message

let emit_fatal_for_break_continue level =
  let suffix = if level = 1 then "" else "s" in
  let message =
    Printf.sprintf "Cannot break/continue %d level%s" level suffix
  in
  emit_fatal_runtime message
