(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Instruction_sequence

let memoize_suffix = "$memoize_impl"

let getmemokeyl local index name =
  [
    instr_getmemokeyl (Local.Named name);
    instr_setl (Local.Unnamed (local + index));
    instr_popc;
  ]

let param_code_sets params local =
  gather
  @@ List.concat_mapi params (fun index param ->
         getmemokeyl local index @@ Hhas_param.name param)

let param_code_gets params =
  gather
  @@ List.map params (fun param ->
         instr_cgetl (Local.Named (Hhas_param.name param)))

let check_memoize_possible pos ~params ~is_method =
  if (not is_method) && List.exists params (fun p -> p.Aast.param_is_variadic)
  then
    Emit_fatal.raise_fatal_runtime
      pos
      "<<__Memoize>> cannot be used on functions with variable arguments"
