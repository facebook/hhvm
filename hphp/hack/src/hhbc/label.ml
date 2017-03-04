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
  | Regular of int
  | Catch of int
  | Fault of int
  | DefaultArg of int

let id label =
  match label with
  | Regular id
  | Catch id
  | Fault id
  | DefaultArg id -> id

(* Numbers for string label *)
let next_label = ref 0

let get_next_label () =
  let current = !next_label in
  next_label := current + 1;
  current

let next_regular () =
  Regular (get_next_label())

let next_catch () =
  Catch (get_next_label())

let next_fault () =
  Fault (get_next_label())

let next_default_arg () =
  DefaultArg (get_next_label())

let reset_label () =
  next_label := 0

(* Numbers for array, map, dict, set, shape labels *)
let next_data_label = ref 0

let get_next_data_label () =
  let current = !next_data_label in
  next_data_label := current + 1;
  current
