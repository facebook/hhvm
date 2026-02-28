(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** see .mli **)

type t = string

let make =
  let prefix_count = ref 0 in
  fun () ->
    incr prefix_count;
    string_of_int !prefix_count ^ "$"

let make_key prefix k = prefix ^ k

let remove prefix k =
  let prefix_size = String.length prefix in
  assert (String.sub k 0 prefix_size = prefix);
  String.sub k prefix_size (String.length k - prefix_size)
