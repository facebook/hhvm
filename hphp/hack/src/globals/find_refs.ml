(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Find local references mode *)
(*****************************************************************************)

let (find_refs_result: Pos.t list ref) = ref []
let (find_refs_target: (int * int) option ref) = ref None
