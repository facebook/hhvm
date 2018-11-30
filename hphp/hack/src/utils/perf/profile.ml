(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type gc_alloc = {
  minor_words: float;
  major_words: float;
}
let make_merge_alloc merge_absolute alloc1 alloc2 = {
  minor_words = merge_absolute alloc1.minor_words alloc2.minor_words;
  major_words = merge_absolute alloc1.major_words alloc2.major_words;
}
let sum_alloc = make_merge_alloc (+.)
let sub_alloc = make_merge_alloc (-.)
let gc_alloc_neutral : gc_alloc = { minor_words = 0.; major_words = 0.; }

(** Accepts a closure with a query parameter that can be called to check GC
 * allocation counts (minor & major words) since the closure was called, e.g.:
 * with_gc_alloc fun delta -> // use delta () to get _relative_ counters )
 *)
let with_gc_alloc func =
  let query = fun () -> match Gc.quick_stat () with
  | { Gc. minor_words; major_words; _ } -> { minor_words; major_words } in
  let before = query () in
  let delta = fun () -> sub_alloc (query ()) before in
  func delta

let measure_gc_alloc func = with_gc_alloc (fun delta -> func delta, delta ())
let fake_gc_alloc func = func (fun () -> gc_alloc_neutral), gc_alloc_neutral

let query_user_time ?(children=true) () =
  let tm = Unix.times () in
  tm.Unix.tms_utime +. if children then tm.Unix.tms_cutime else 1.

let query_sys_time ?(children=true) () =
  let tm = Unix.times () in
  tm.Unix.tms_stime +. if children then tm.Unix.tms_cstime else 1.

let query_real_time = Unix.gettimeofday

(** Given a function to profile, profile it multiple times such that it
 * runs at least min_time seconds, returning the average time, GC allocations,
 * as well as the number of runs.  It return 0 for the latter if the first run fails
 * so that failures can be unambiguously identified and filtered out if needed.
 *)
let profile_longer_than run ?(min_runs=1) min_time =
  let rec work dt_user_tot nbr_runs =
    let t_user0 = query_user_time () in
    let run_incr =
      try run (); 1
      with _ -> 0 (* distinguish failures by letting run count stay 0 *)
    in
    let dt_user = (query_user_time ()) -. t_user0 in
    let dt_user_tot = dt_user_tot +. dt_user in
    let nbr_runs = nbr_runs + run_incr in
    if (dt_user_tot < min_time || nbr_runs < min_runs) && run_incr > 0 then
      (work[@tailcall]) dt_user_tot nbr_runs
    else
      dt_user_tot, nbr_runs
  in
  let dt_user_tot, nbr_runs = work 0. 0 in
  let to_div n = if nbr_runs = 0 then 1. else float_of_int n in
  dt_user_tot /. (to_div nbr_runs), nbr_runs
