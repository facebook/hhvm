(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* We need some estimate when to stop running major_slice to avoid just marking and sweeping
 * in a busy loop forever. There might be some more accurate counters hidden somewhere, but for
 * now I will just very conservatively assume that EVERY word allocated in major heap is potentially
 * garbage and will keep running major_slice as long as the sum of its arguments is less than total
 * major heap words allocated since program start. This is obviously way too much:
 * - some of the words are not garbage
 * - some (probably many) of the words will be already collected by automatic slices
 * - there is some chance that major_slice argument doesn't mean what I think it means
 *
 * It seems to be working in practice - it stops marking and sweeping reasonably fast
 * (below 200ms) after allocations stop, and it does smooth out some test cases I tried *)
let gc_slice_sum_ref = ref 0.0

let gc slice =
  let { Gc.major_words; _ } = Gc.quick_stat () in
  let next_sum = !gc_slice_sum_ref +. float_of_int slice in
  if major_words < next_sum then
    true
  else
    let (_ : int) = Gc.major_slice slice in
    gc_slice_sum_ref := next_sum;
    false

let rec select ~slice ~deadline fd_list =
  let (ready_fds, _, _) = Unix.select fd_list [] [] 0.0 in
  let t = Unix.gettimeofday () in
  match ready_fds with
  | [] when slice = 0 -> []
  | [] when t < deadline ->
    let is_finished = gc slice in
    if is_finished then
      ready_fds
    else
      select ~slice ~deadline fd_list
  | _ -> ready_fds

let select ~slice ~timeout fd_list =
  let deadline = Unix.gettimeofday () +. timeout in
  select ~slice ~deadline fd_list
