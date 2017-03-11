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
let memoize_guard = "static$memoize_cache$guard"
let memoize_shared_cache = "$shared$multi$memoize_cache"
let memoize_single_cache = "$shared$guarded_single$memoize_cache"
let memoized_single_guard = "$shared$guarded_single$memoize_cache$guard"

let param_code_sets params local =
  let rec aux params local block =
  match params with
  | [] -> block
  | h :: t ->
    let code = gather [
      instr_getmemokeyl (Local.Named (Hhas_param.name h));
      instr_setl (Local.Unnamed local);
      instr_popc ] in
    aux t (local + 1) (gather [block; code]) in
  aux params local empty

let param_code_gets params =
  let rec aux params index block =
    match params with
    | [] -> block
    | h :: t ->
      let block = gather [
        block;
        instr_fpassl index (Local.Named (Hhas_param.name h)) ] in
      aux t (index + 1) block in
  aux params 0 empty

let memoize_function_no_params renamed_name =
  (* TODO: A lot of this codegen doesn't make a lot of sense to me.
  Try to understand it and see if it can be improved. *)
  let local_cache = Local.Unnamed 0 in
  let local_guard = Local.Unnamed 1 in
  let label_0 = Label.Regular 0 in
  let label_1 = Label.Regular 1 in
  let label_2 = Label.Regular 2 in
  let label_3 = Label.Regular 3 in
  let label_4 = Label.Regular 4 in
  let label_5 = Label.Regular 5 in
  gather [
    instr_entrynop;
    instr_false;
    instr_staticlocinit local_guard memoize_guard;
    instr_null;
    instr_staticlocinit local_cache memoize_cache;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_0;
    instr_cgetl local_cache;
    instr_dup;
    instr_istypec Hhbc_ast.OpNull;
    instr_jmpnz label_1;
    instr_retc;
    instr_label label_1;
    instr_popc;
    instr_label label_0;
    instr_null;
    instr_maybememotype;
    instr_jmpz label_2;
    instr_cgetl local_guard;
    instr_jmpz label_2;
    instr_null;
    instr_retc;
    instr_label label_2;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_3;
    instr_fpushfuncd 0 renamed_name;
    instr_fcall 0;
    instr_unboxr;
    instr_setl local_cache;
    instr_jmp label_4;
    instr_label label_3;
    instr_fpushfuncd 0 renamed_name;
    instr_fcall 0;
    instr_unboxr;
    instr_label label_4;
    instr_null;
    instr_maybememotype;
    instr_jmpz label_5;
    instr_true;
    instr_setl local_guard;
    instr_popc;
    instr_label label_5;
    instr_retc ]

let memoize_function_with_params params renamed_name =
  let param_count = List.length params in
  let static_local = Local.Unnamed param_count in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed (param_count + 1) in
  gather [
    (* Why do we emit a no-op that cannot be removed here?
    The alleged reason for this no-op is because the runtime handles a branch
    to the first intruction poorly, but we know there will never be any such
    branch in this method. *)
    instr_entrynop;
    Emit_body.emit_method_prolog params;
    instr_dict 0 [];
    instr_staticlocinit static_local memoize_cache;
    param_code_sets params (param_count + 1);
    instr_basel static_local Warn;
    instr_memoget 0 first_local param_count;
    instr_isuninit;
    instr_jmpnz label;
    instr_cgetcunop;
    instr_retc;
    instr_label label;
    instr_ugetcunop;
    instr_popu;
    instr_fpushfuncd param_count renamed_name;
    param_code_gets params;
    instr_fcall param_count;
    instr_unboxr;
    instr_basel static_local Define;
    instr_memoset 0 first_local param_count;
    instr_retc
  ]

let memoized_function_body params renamed_name =
  if params = [] then
    memoize_function_no_params renamed_name
  else
    memoize_function_with_params params renamed_name

let memoize_function compiled =
  let original_name = Hhas_function.name compiled in
  let renamed_name = original_name ^ memoize_suffix in
  let renamed = Hhas_function.with_name compiled renamed_name in
  let params = Hhas_function.params compiled in
  let body = memoized_function_body params renamed_name in
  let body = instr_seq_to_list body in
  let memoized = Hhas_function.with_body compiled body in
  (renamed, memoized)

let memoize_method_no_params renamed_name =
  (* TODO: A lot of this codegen doesn't make a lot of sense to me.
  Try to understand it and see if it can be improved. *)
  let label_0 = Label.Regular 0 in
  let label_1 = Label.Regular 1 in
  let label_2 = Label.Regular 2 in
  let label_3 = Label.Regular 3 in
  let label_4 = Label.Regular 4 in
  let label_5 = Label.Regular 5 in
  gather [
    instr_entrynop;
    instr_checkthis;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_0;
    instr_baseh;
    instr_querym_cget_pt 0 memoize_single_cache;
    instr_dup;
    instr_istypec Hhbc_ast.OpNull;
    instr_jmpnz label_1;
    instr_retc;
    instr_label label_1;
    instr_popc;
    instr_label label_0;
    instr_null;
    instr_maybememotype;
    instr_jmpz label_2;
    instr_baseh;
    instr_querym_cget_pt 0 memoized_single_guard;
    instr_jmpz label_2;
    instr_null;
    instr_retc;
    instr_label label_2;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_3;
    instr_this;
    instr_fpushobjmethodd_nullthrows 0 renamed_name;
    instr_fcall 0;
    instr_unboxr;
    instr_baseh;
    instr_setm_pt 0 memoize_single_cache;
    instr_jmp label_4;
    instr_label label_3;
    instr_this;
    instr_fpushobjmethodd_nullthrows 0 renamed_name;
    instr_fcall 0;
    instr_unboxr;
    instr_label label_4;
    instr_null;
    instr_maybememotype;
    instr_jmpz label_5;
    instr_true;
    instr_baseh;
    instr_setm_pt 0 memoized_single_guard;
    instr_popc;
    instr_label label_5;
    instr_retc ]

let memoize_method_with_params params renamed_name total_count index =
  let param_count = List.length params in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed param_count in
  (* All memoized methods in the same class share a cache. We distinguish the
  methods from each other by adding a unique integer indexing the method itself
  to the set of indices for the cache. *)
  let index_block =
    if total_count = 1 then
      empty
    else
      gather [
        instr_int index;
        instr_setl first_local;
        instr_popc ] in
  (* The total number of unnamed locals is one for the optional index, and
  one for each formal parameter. *)
  let local_count = if total_count = 1 then param_count else param_count + 1 in
  (* The index of the first local that represents a formal is the number of
  parameters, plus one for the optional index. This is equal to the count
  of locals, so we'll just use that. *)
  let first_parameter_local = local_count in
  gather [
    (* Why do we emit a no-op that cannot be removed here? *)
    instr_entrynop;
    Emit_body.emit_method_prolog params;
    instr_checkthis;
    index_block;
    param_code_sets params first_parameter_local;
    instr_baseh;
    instr_dim_warn_pt memoize_shared_cache;
    instr_memoget 0 first_local local_count;
    instr_isuninit;
    instr_jmpnz label;
    instr_cgetcunop;
    instr_retc;
    instr_label label;
    instr_ugetcunop;
    instr_popu;
    instr_this;
    instr_fpushobjmethodd_nullthrows param_count renamed_name;
    param_code_gets params;
    instr_fcall param_count;
    instr_unboxr;
    instr_baseh;
    instr_dim_define_pt memoize_shared_cache;
    instr_memoset 0 first_local local_count;
    instr_retc ]

let memoized_method_body params renamed_name total_count index =
  if params = [] && total_count = 1 then
    memoize_method_no_params renamed_name
  else
    memoize_method_with_params params renamed_name total_count index

let memoize_method compiled total_count index =
  let original_name = Hhas_method.name compiled in
  let renamed_name = original_name ^ memoize_suffix in
  let renamed = Hhas_method.with_name compiled renamed_name in
  let renamed = Hhas_method.make_private renamed in
  let params = Hhas_method.params compiled in
  let body = memoized_method_body params renamed_name total_count index in
  let body = instr_seq_to_list body in
  let memoized = Hhas_method.with_body compiled body in
  (renamed, memoized)
