(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(*
TODO: Iterators need to nest appropriately.
 *)

type t = Id of int

let to_int (Id i) = i

let to_string (Id i) = string_of_int i

let next_iterator = ref 0

let get_iterator () =
  let current = !next_iterator in
  next_iterator := current + 1;
  Id current

let reset_iterator () =
  next_iterator := 0
