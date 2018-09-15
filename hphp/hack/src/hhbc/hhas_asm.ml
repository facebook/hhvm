(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* Everything needed to describe a function, method, or main asm *)
type t = {
  asm_instrs                : Instruction_sequence.t;
  asm_decl_vars             : string list; (* Actually local_id list *)
  asm_num_iters             : int;
  asm_num_cls_ref_slots     : int;
  asm_is_memoize_wrapper    : bool;
  asm_is_memoize_wrapper_lsb: bool;
  asm_static_inits          : string list;
}

let make
  instrs
  decl_vars
  num_iters
  num_cls_ref_slots
  is_memoize_wrapper
  is_memoize_wrapper_lsb
  static_inits =
  {
    asm_instrs = instrs;
    asm_decl_vars = decl_vars;
    asm_num_iters = num_iters;
    asm_num_cls_ref_slots = num_cls_ref_slots;
    asm_is_memoize_wrapper = is_memoize_wrapper;
    asm_is_memoize_wrapper_lsb = is_memoize_wrapper_lsb;
    asm_static_inits = static_inits;
  }

let instrs a = a.asm_instrs
let decl_vars a = a.asm_decl_vars
let num_iters a = a.asm_num_iters
let num_cls_ref_slots a = a.asm_num_cls_ref_slots
let is_memoize_wrapper a = a.asm_is_memoize_wrapper
let is_memoize_wrapper_lsb a = a.asm_is_memoize_wrapper_lsb
let static_inits a = a.asm_static_inits
