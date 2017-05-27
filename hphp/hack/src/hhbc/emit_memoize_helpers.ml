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
let single_memoize_cache class_prefix =
  class_prefix ^ "$guarded_single$memoize_cache"
let single_memoize_cache_guard class_prefix =
  single_memoize_cache class_prefix ^ "$guard"
let shared_single_memoize_cache class_prefix =
  Hhbc_id.Prop.from_raw_string
    ("$shared" ^ single_memoize_cache class_prefix)
let shared_single_memoize_cache_guard class_prefix =
  Hhbc_id.Prop.from_raw_string
    ("$shared" ^ single_memoize_cache_guard class_prefix)

let param_code_sets params local =
  gather @@ List.concat_mapi params (fun index param ->
      [ instr_getmemokeyl (Local.Named (Hhas_param.name param));
        instr_setl (Local.Unnamed (local + index));
        instr_popc ])

let param_code_gets params =
  gather @@ List.mapi params (fun index param ->
      instr_fpassl index (Local.Named (Hhas_param.name param)))
