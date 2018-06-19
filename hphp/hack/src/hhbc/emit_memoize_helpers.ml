(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Instruction_sequence
open Hh_core

let memoize_suffix = "$memoize_impl"

let param_code_sets params local =
  gather @@ List.concat_mapi params (fun index param ->
      [ instr_getmemokeyl (Local.Named (Hhas_param.name param));
        instr_setl (Local.Unnamed (local + index));
        instr_popc ])

let param_code_gets params =
  gather @@ List.concat_map params (fun param ->
      [ instr_cgetl (Local.Named (Hhas_param.name param));
        instr_fpasscnop ])

let check_memoize_possible pos ~ret_by_ref ~params ~is_method =
  if ret_by_ref
  then Emit_fatal.raise_fatal_runtime pos
    "<<__Memoize>> cannot be used on functions that return by reference";
  if List.exists params (fun p -> p.Ast.param_is_reference)
  then Emit_fatal.raise_fatal_runtime pos
    "<<__Memoize>> cannot be used on functions with args passed by reference";
  if not is_method && List.exists params (fun p -> p.Ast.param_is_variadic)
  then Emit_fatal.raise_fatal_runtime pos
    "<<__Memoize>> cannot be used on functions with variable arguments"
