(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type label = int

(* Numbers for string label *)
let next_label = ref 0

let get_next_label () =
  let current = !next_label in
  next_label := current + 1;
  current

let reset_label () =
  next_label := 0

(* Unnamed local variables
TODO: We will need to rename the unnamed local variables so that their count
begins at the number of named local variables in the method.
 *)
let next_local = ref 0

let get_unnamed_local () =
  let current = !next_local in
  next_local := current + 1;
  Hhbc_ast.Local_unnamed current

let reset_local () =
  next_local := 0

(* Numbers for array, map, dict, set, shape labels *)
let next_data_label = ref 0

let get_next_data_label () =
  let current = !next_data_label in
  next_data_label := current + 1;
  current
