(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = Id of int

let to_string (Id i) = string_of_int i

let next_iterator = ref 0

let num_iterators = ref 0

let get_iterator () =
  let current = !next_iterator in
  next_iterator := current + 1;
  num_iterators := max !num_iterators !next_iterator;
  Id current

let free_iterator () = next_iterator := !next_iterator - 1

let reset_iterator () =
  next_iterator := 0;
  num_iterators := 0
