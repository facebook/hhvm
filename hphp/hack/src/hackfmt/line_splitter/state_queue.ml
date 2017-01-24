(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

type t = {
  queue: Solve_state.t list;
}

let make q =
  {queue = q;}

let add t state =
  { queue = state :: t.queue }

let is_empty t =
  List.length t.queue = 0

let get_next t =
  (* TODO: make this queue into a heap instead of sorting here *)
  let queue = List.sort t.queue ~cmp:Solve_state.compare in
  match queue with
    | hd :: tl ->
      {queue = tl}, hd
    | [] -> raise (Failure "Queue is empty when calling get_next\n")
