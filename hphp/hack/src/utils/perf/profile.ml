(*
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
  (* words in major heap _including_ promoted ones *)
  promo_words: float; (* words promoted from minor to major heap *)
}

let make_merge_alloc merge_absolute alloc1 alloc2 =
  {
    minor_words = merge_absolute alloc1.minor_words alloc2.minor_words;
    major_words = merge_absolute alloc1.major_words alloc2.major_words;
    promo_words = merge_absolute alloc1.promo_words alloc2.promo_words;
  }

let sum_alloc = make_merge_alloc ( +. )

let sub_alloc = make_merge_alloc ( -. )

let gc_alloc_neutral : gc_alloc =
  { minor_words = 0.; major_words = 0.; promo_words = 0. }

(** Accepts a closure with a query parameter that can be called to check GC
 * allocation counts (minor & major words) since the closure was called, e.g.:
 * with_gc_alloc fun delta -> // use delta () to get _relative_ counters )
 *)
let with_gc_alloc func =
  let query () =
    match Gc.quick_stat () with
    | { Gc.minor_words; major_words; promoted_words; _ } ->
      { minor_words; major_words; promo_words = promoted_words }
  in
  let before = query () in
  let delta () = sub_alloc (query ()) before in
  func delta

let measure_gc_alloc func = with_gc_alloc (fun delta -> (func delta, delta ()))

let fake_gc_alloc func = (func (fun () -> gc_alloc_neutral), gc_alloc_neutral)

(* Non-cumulative statistic (i.e., non-counters) related to heap *)
type mem_stat = {
  heap_major_words: float;
  (* see Gc.heap_words *)
  heap_major_chunks: float; (* see Gc.heap_chunks *)
}

let avg_mem ?(size1 = 1) ?(size2 = 1) mem1 mem2 =
  let avg x1 x2 =
    ((float_of_int size1 *. x1) +. (float_of_int size2 *. x2))
    /. (float_of_int @@ (size1 + size2))
  in
  {
    heap_major_words = avg mem1.heap_major_words mem2.heap_major_words;
    heap_major_chunks = avg mem1.heap_major_chunks mem2.heap_major_chunks;
  }

let mem_neutral : mem_stat = { heap_major_words = 0.; heap_major_chunks = 0. }

(* Returns the current memory stats that are not counters *)
let query_mem () =
  match Gc.quick_stat () with
  | { Gc.heap_words; heap_chunks; _ } ->
    {
      heap_major_words = float_of_int heap_words;
      heap_major_chunks = float_of_int heap_chunks;
    }

let query_user_time ?(children = true) () =
  let tm = Unix.times () in
  tm.Unix.tms_utime
  +.
  if children then
    tm.Unix.tms_cutime
  else
    1.

let query_sys_time ?(children = true) () =
  let tm = Unix.times () in
  tm.Unix.tms_stime
  +.
  if children then
    tm.Unix.tms_cstime
  else
    1.

let query_real_time = Unix.gettimeofday

(** Given a function to profile, profiles it multiple times such that it
 * runs at least min_time seconds, returning the average time, non-aggregatable heap stats from
 * the first run and number of runs.  It returns 0 for the number of runs if the first run fails
 * so that failures can be unambiguously identified and filtered out if needed.
 *)
let profile_longer_than run ?(min_runs = 1) ?(retry = true) min_time =
  let rec work ?mem_stat dt_user_tot nbr_runs =
    let t_user0 = query_user_time () in
    let run_incr =
      try
        run ();
        1
      with e ->
        if retry then
          0
        (* distinguish failures by letting run count stay 0 *)
        else
          raise e
    in
    let dt_user = query_user_time () -. t_user0 in
    let dt_user_tot = dt_user_tot +. dt_user in
    let mem_stat =
      if nbr_runs = 0 then
        Some (query_mem ())
      else
        mem_stat
    in
    let nbr_runs = nbr_runs + run_incr in
    if (dt_user_tot < min_time || nbr_runs < min_runs) && run_incr > 0 then
      (work [@tailcall]) dt_user_tot nbr_runs ?mem_stat
    else
      (dt_user_tot, nbr_runs, mem_stat)
  in
  let (dt_user_tot, nbr_runs, mem_stat) = work 0. 0 in
  let to_div n =
    if nbr_runs = 0 then
      1.
    else
      float_of_int n
  in
  (dt_user_tot /. to_div nbr_runs, nbr_runs, Base.Option.value_exn mem_stat)
