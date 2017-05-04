(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type t =
  (* Unnamed local, numbered from 0, and rebased above named locals
   * at the last moment when writing out the assembly file *)
 | Unnamed of int
   (* Named local, necessarily starting with `$` *)
 | Named of string
   (* Will be rewritten to an unnamed local. *)
 | Pipe

let next_local = ref 0

let get_unnamed_local () =
  let current = !next_local in
  next_local := current + 1;
  Unnamed current

let reset_local () =
  next_local := 0
