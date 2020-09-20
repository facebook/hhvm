(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type t = int

let pp fmt n = Format.fprintf fmt "<scope%d>" n

let compare = Int.compare

let equal = Int.equal

let alloc =
  let next = ref 0 in
  fun () ->
    incr next;
    !next
