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
open Hhbc_ast.MemberOpMode

let memoize_suffix = "$memoize_impl"
let memoize_cache = "static$memoize_cache"

let memoized_body params =
  (* TODO: The unnamed local count should start fresh here, at the number
  of parameters. So if there are two params, it should start at _2. *)
  Label.reset_label ();
  Local.reset_local ();
  let static_local = Local.get_unnamed_local () in
  let label = Label.next_regular () in
  gather [
    (* TODO: Why do we emit a no-op that cannot be removed here? *)
    instr_entrynop;
    Emit_body.emit_method_prolog params;
    instr_dict 0 [];
    instr_staticlocinit static_local memoize_cache;
    (*  TODO: one of these sequences per param
        instr_getmemokeyl param_name
        SetL new_unnamed_local
        PopC
    *)
    instr_basel static_local Warn;
    (* TODO: instr_memoget 0 lowest_unnamed highest_unnamed *)
    instr_isuninit;
    instr_jmpnz label;
    instr_cgetcunop;
    instr_retc;
    instr_label label;
    instr_ugetcunop;
    instr_popu;
    (* TODO: instr_fpushfuncd param_count new_name *)
    (* TODO: one of these for each parameter:
    instr_fpassl param_index param_name *)
    (* TODO: instr_fcall param_count *)
    instr_unboxr;
    instr_basel static_local Define;
    (* TODO: instr_memoset 0 lowest_unnamed highest_unnamed *)
    instr_retc
  ]

let memoize_function compiled =
  let original_name = Hhas_function.name compiled in
  let renamed_name = original_name ^ memoize_suffix in
  let renamed = Hhas_function.with_name compiled renamed_name in
  let params = Hhas_function.params compiled in
  let body = memoized_body params in
  let body = instr_seq_to_list body in
  let memoized = Hhas_function.with_body compiled body in (* TODO *)
  (renamed, memoized)
