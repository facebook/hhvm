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

let memoize_suffix = "$memoize_impl"

let memoized_body = (* TODO: Generate based on formals *)
  gather [
    instr_null;
    instr_retc
  ]

let memoize_function compiled =
  let original_name = Hhas_function.name compiled in
  let renamed_name = original_name ^ memoize_suffix in
  let renamed = Hhas_function.with_name compiled renamed_name in
  let body = memoized_body in
  let body = instr_seq_to_list body in
  let memoized = Hhas_function.with_body compiled body in (* TODO *)
  (renamed, memoized)
