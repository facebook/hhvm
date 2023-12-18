open Profile

let test_with_gc_alloc () =
  with_gc_alloc (fun query ->
      let pair = ref (1, 2) in
      let now () =
        match query () with
        | { Profile.minor_words; _ } -> minor_words
      in
      let init = now () in
      while now () -. init < 100. do
        pair := (snd !pair, fst !pair) (* trigger allocation *)
      done;
      42)
  = 42

(* verify that the continuation forwards the return value *)

let test_merge_alloc () =
  let alloc1 = { minor_words = 1.; major_words = 2.; promo_words = 0. } in
  let alloc2 = { minor_words = 5.; major_words = 6.; promo_words = 2. } in
  sum_alloc (sub_alloc alloc1 alloc2) alloc2 = alloc1

let test_avg_mem () =
  let heap1 = { heap_major_words = 2.; heap_major_chunks = 6. } in
  let heap2 = { heap_major_words = 5.; heap_major_chunks = 3. } in
  avg_mem heap1 heap2
  |> avg_mem ~size2:2 heap1
  = {
      heap_major_words = (2. +. 5. +. 2.) /. 3.;
      heap_major_chunks = (6. +. 3. +. 6.) /. 3.;
    }

let test_profile_longer_than () =
  let (user_time, nbr_runs, mem) =
    profile_longer_than
      (fun () ->
        for i = 0 to 10000 do
          (* ensure the unit of work is measurable *)
          ignore @@ List.fold_left ( + ) 0 [1; 2; 3; 4; 5; 6; 7; 8; 9]
        done)
      0.01
  in
  if nbr_runs < 1 then failwith "not ran at least once";
  (* to avoid flakiness, use 5 orders of magnitude smaller *)
  user_time >= 1e-8
  (* In OCaml 5, heap_major_chunks will always be zero. *)
  (* && mem.heap_major_chunks >= 1. *)
  && mem.heap_major_words >= 0.

let test_profile_longer_than_min_runs_gt1 () =
  let min_runs = 7 in
  let cnt = ref 0 in
  let (_, nbr_runs, _) =
    profile_longer_than ~min_runs (fun () -> cnt := !cnt + 1) 0.0
  in
  nbr_runs = min_runs && min_runs = !cnt

let () =
  Unit_test.run_all
    [
      ("test_with_gc_alloc", test_with_gc_alloc);
      ("test_merge_alloc", test_merge_alloc);
      ("test_avg_mem", test_avg_mem);
      ("test_profile_longer_than", test_profile_longer_than);
      ( "test_profile_longer_than_min_runs_gt1",
        test_profile_longer_than_min_runs_gt1 );
    ]
