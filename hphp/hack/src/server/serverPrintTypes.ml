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
(* The code to retrieve type information in a file *)
(*****************************************************************************)
open Utils
module SE = ServerEnv

let go fn genv env ic oc =
  assert (!(Typing_defs.type_acc) = []);
  Typing_defs.accumulate_types := true;
  ServerIdeUtils.recheck [fn];
  let result = !(Typing_defs.type_acc) in
  Silent.is_silent_mode := false;
  Typing_defs.accumulate_types := false;
  Typing_defs.type_acc := [];
  Marshal.to_channel oc result [];
  flush oc
