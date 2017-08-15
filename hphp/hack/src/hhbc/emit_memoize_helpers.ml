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
open Core

let memoize_suffix = "$memoize_impl"
let static_memoize_cache = "static$memoize_cache"
let static_memoize_cache_guard = "static$memoize_cache$guard"
let multi_memoize_cache class_prefix =
  class_prefix ^ "$multi$memoize_cache"
let shared_multi_memoize_cache class_prefix =
  Hhbc_id.Prop.from_raw_string
    ("$shared" ^ multi_memoize_cache class_prefix)
let guarded_single_memoize_cache class_prefix =
  class_prefix ^ "$guarded_single$memoize_cache"
let guarded_single_memoize_cache_guard class_prefix =
  guarded_single_memoize_cache class_prefix ^ "$guard"
let single_memoize_cache class_prefix =
  class_prefix ^ "$single$memoize_cache"
let shared_single_memoize_cache class_prefix =
  Hhbc_id.Prop.from_raw_string
    ("$shared" ^ single_memoize_cache class_prefix)
let guarded_shared_single_memoize_cache class_prefix =
  Hhbc_id.Prop.from_raw_string
    ("$shared" ^ guarded_single_memoize_cache class_prefix)
let guarded_shared_single_memoize_cache_guard class_prefix =
  Hhbc_id.Prop.from_raw_string
    ("$shared" ^ guarded_single_memoize_cache_guard class_prefix)

let param_code_sets params local =
  gather @@ List.concat_mapi params (fun index param ->
      [ instr_getmemokeyl (Local.Named (Hhas_param.name param));
        instr_setl (Local.Unnamed (local + index));
        instr_popc ])

let param_code_gets params =
  gather @@ List.mapi params (fun index param ->
      instr_fpassl index (Local.Named (Hhas_param.name param)))

(* Return true if method or function with this kind and return type
 * definitely cannot return null *)
let cannot_return_null fun_kind _ret =
  fun_kind = Ast_defs.FAsync
  (* Also in HHVM:
  || (m_curFunc->retTypeConstraint.hasConstraint() &&
   !m_curFunc->retTypeConstraint.isSoft() &&
   !m_curFunc->retTypeConstraint.isNullable() &&
   RuntimeOption::EvalCheckReturnTypeHints >= 3) *)

let check_memoize_possible pos ~ret_by_ref ~params =
  if ret_by_ref
  then Emit_fatal.raise_fatal_runtime pos
    "<<__Memoize>> cannot be used on functions that return by reference";
  if List.exists params (fun p -> p.Ast.param_is_reference)
  then Emit_fatal.raise_fatal_runtime pos
    "<<__Memoize>> cannot be used on functions with args passed by reference";
  if List.exists params (fun p -> p.Ast.param_is_variadic)
  then Emit_fatal.raise_fatal_runtime pos
    "<<__Memoize>> cannot be used on functions with variable arguments"
