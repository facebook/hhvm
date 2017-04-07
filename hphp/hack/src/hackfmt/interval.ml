(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Helpers for half-open intervals *)

type t = int * int

let intervals_overlap (a: t) (b: t) : bool =
  let a_start, a_end = a in
  let b_start, b_end = b in
  a_start = b_start ||
  a_start < b_start && b_start < a_end ||
  b_start < a_start && a_start < b_end
