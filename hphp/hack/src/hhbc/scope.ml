(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Instruction_sequence
open Iterator
open Local

(* Run emit () in a new unnamed local scope, which produces three instruction
 * blocks -- before, inner, after. If emit () registered any unnamed locals, the
 * inner block will be wrapped in a try/catch that will unset these unnamed
 * locals upon exception. *)
let with_unnamed_locals emit =
  let current_next_local = !next_local in
  let current_temp_local_map = !temp_local_map in
  let (before, inner, after) = emit () in
  if current_next_local = !next_local then
    gather [before; inner; after]
  else
    let local_unsets =
      gather
      @@ List.init (!next_local - current_next_local) ~f:(fun idx ->
             instr_unsetl (Unnamed (idx + current_next_local)))
    in
    next_local := current_next_local;
    temp_local_map := current_temp_local_map;
    gather [before; create_try_catch inner local_unsets; after]

(* Run emit () in a new unnamed local and iterator scope, which produces three
 * instruction blocks -- before, inner, after. If emit () registered any unnamed
 * locals or iterators, the inner block will be wrapped in a try/catch that will
 * unset these unnamed locals and free these iterators upon exception. *)
let with_unnamed_locals_and_iterators emit =
  let current_next_local = !next_local in
  let current_temp_local_map = !temp_local_map in
  let current_next_iterator = !next_iterator in
  let (before, inner, after) = emit () in
  if current_next_local = !next_local && current_next_iterator = !next_iterator
  then
    gather [before; inner; after]
  else
    let local_unsets =
      gather
      @@ List.init (!next_local - current_next_local) ~f:(fun idx ->
             instr_unsetl (Unnamed (idx + current_next_local)))
    in
    let iter_frees =
      gather
      @@ List.init (!next_iterator - current_next_iterator) ~f:(fun idx ->
             instr_iterfree (Id (idx + current_next_iterator)))
    in
    next_local := current_next_local;
    temp_local_map := current_temp_local_map;
    next_iterator := current_next_iterator;
    gather
      [
        before; create_try_catch inner (gather [local_unsets; iter_frees]); after;
      ]

(* An equivalent of with_unnamed_locals () that allocates a single local and
 * passes it to emit (). *)
let with_unnamed_local emit =
  with_unnamed_locals @@ fun () ->
  let tmp = Local.get_unnamed_local () in
  emit tmp

(* Pop the top of the stack into an unnamed local, run emit (), then push the
 * stashed value to the top of the stack. *)
let stash_top_in_unnamed_local emit =
  with_unnamed_locals @@ fun () ->
  let tmp = Local.get_unnamed_local () in
  (instr_popl tmp, emit (), instr_pushl tmp)
