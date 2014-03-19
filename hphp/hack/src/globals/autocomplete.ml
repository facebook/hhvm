(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Utils

type autocomplete_result = {
    pos  : Pos.t option;
    ty   : string option;
    name : string;
  }

(*****************************************************************************)
(* Auto-complete mode *)
(*****************************************************************************)

let auto_complete = ref false
let auto_complete_result = ref (SMap.empty: autocomplete_result SMap.t)

(*****************************************************************************)
(* Argument info mode *)
(*****************************************************************************)
let (argument_info_target: (int * int) option ref) = ref None
let (argument_info_expected: ((string option * string) list) option ref) = ref None
let (argument_info_position: int option ref) = ref None


(*****************************************************************************)
(* Returns true if this is the identifier we want to auto-complete *)
(*****************************************************************************)

type autocomplete_type =
| Acid
| Acnew
| Actype
| Acclass_get
| Acvar

let make_result_without_pos_or_type name = {
    pos  = None;
    ty   = None;
    name = name;
  }

let make_result_without_type name p = {
    pos  = Some p;
    ty   = None;
    name = name;
  }

let make_result name p ty = {
    pos  = Some p;
    ty   = Some ty;
    name = name;
  }

let (argument_global_type: autocomplete_type option ref) = ref None
let auto_complete_for_global = ref ""
let auto_complete_suffix = "AUTO332"
let suffix_len = String.length auto_complete_suffix
let is_auto_complete x =
  !auto_complete && String.length x >= suffix_len &&
  let suffix = String.sub x (String.length x - suffix_len) suffix_len in
  suffix = auto_complete_suffix
