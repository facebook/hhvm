(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Hh_core

type t = {
  attribute_name          : string;
  attribute_arguments     : Typed_value.t list
}

let make attribute_name attribute_arguments =
  { attribute_name; attribute_arguments }

let name a = a.attribute_name
let arguments a = a.attribute_arguments

let is_ s attr = (name attr) = s
let has_ f attrs = List.exists attrs f

let is_memoized = (fun attr -> is_ "__Memoize" attr || is_ "__MemoizeLSB" attr)
let is_native   = is_ "__Native"
let is_foldable = is_ "__IsFoldable"
let is_dynamically_callable = is_ "__DynamicallyCallable"
let is_const    = is_ "__Const"
let is_sealed = is_ "__Sealed"
let is_lsb      = is_ "__LSB"

let has_memoized = has_ is_memoized
let has_native   = has_ is_native
let has_foldable = has_ is_foldable
let has_dynamically_callable = has_ is_dynamically_callable
let has_const    = has_ is_const
let has_sealed = has_ is_sealed
let has_lsb      = has_ is_lsb

let is_native_arg s attributes =
  let f attr =
    is_native attr &&
    List.exists (arguments attr)
      ~f:(function Typed_value.String s0 -> s = s0 | _ -> false)
  in
  List.exists attributes f
let is_native_opcode_impl = is_native_arg "OpCodeImpl"
let is_reads_caller_frame = is_native_arg "ReadsCallerFrame"
let is_writes_caller_frame = is_native_arg "WritesCallerFrame"
let is_accesses_caller_frame attr = is_reads_caller_frame attr || is_writes_caller_frame attr
let is_no_injection = is_native_arg "NoInjection"

let deprecation_info attributes =
  let f attr =
    if (name attr) = "__Deprecated" then Some (arguments attr) else None
  in
  List.find_map attributes f
