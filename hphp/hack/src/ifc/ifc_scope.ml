(* Copyright (c) 2020, Facebook, Inc.
   All rights reserved. *)
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
