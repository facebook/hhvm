(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Unnamed local variables
TODO: We will need to rename the unnamed local variables so that their count
begins at the number of named local variables in the method.
 *)

type t =
 | Unnamed of int
 | Named of string
 | Pipe (* Will be rewritten to an unnamed local. *)

let next_local = ref 0

let get_unnamed_local () =
  let current = !next_local in
  next_local := current + 1;
  Unnamed current

let reset_local () =
  next_local := 0
