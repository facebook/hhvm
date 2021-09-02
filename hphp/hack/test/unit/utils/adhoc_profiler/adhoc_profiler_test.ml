(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open OUnit2

module Time = struct
  let time = ref 0.

  let advance duration = time := !time +. duration

  let get () =
    advance 0.01;
    !time
end

let () = Adhoc_profiler.Test.set_time_getter Time.get

let assert_strings_equal =
  OUnit2.assert_equal ~cmp:String.equal ~printer:(Printf.sprintf "%s")

let test1 _ =
  let f () =
    Adhoc_profiler.create ~name:"test1" @@ fun profiler ->
    Time.advance 1.2;
    let () =
      Adhoc_profiler.count_leaf ~name:"step1" profiler @@ fun () ->
      Time.advance 0.3
    in
    Time.advance 0.1;
    let () =
      Adhoc_profiler.count ~name:"step2" profiler @@ fun profiler ->
      Time.advance 0.7;
      let () =
        Adhoc_profiler.count_leaf ~name:"step2.1" profiler @@ fun () ->
        Time.advance 1.0
      in
      let () =
        Adhoc_profiler.count_leaf ~name:"step2.2" profiler @@ fun () ->
        Time.advance 2.0
      in
      ()
    in
    ()
  in
  f ();
  Time.advance 0.6;
  f ();
  let profile =
    Adhoc_profiler.CallTree.to_string @@ Adhoc_profiler.get_and_reset ()
  in
  let expected_profile =
    {|test1                                                                           2           10.780 sec   100.00%
  step1                                                                           2           0.620 sec   5.75%
  step2                                                                           2           7.500 sec   69.57%
    step2.1                                                                         2           2.020 sec   18.74%
    step2.2                                                                         2           4.020 sec   37.29%
|}
  in
  assert_strings_equal expected_profile profile;
  ()

let test_branch _ =
  let f b =
    Adhoc_profiler.create ~name:"test_branch" @@ fun profiler ->
    Time.advance 1.2;
    let () =
      if b then (
        let () =
          Adhoc_profiler.count_leaf ~name:"step1" profiler @@ fun () ->
          Time.advance 0.3
        in
        Time.advance 0.1;
        ()
      ) else
        let () =
          Adhoc_profiler.count ~name:"step2" profiler @@ fun profiler ->
          Time.advance 0.7;
          let () =
            Adhoc_profiler.count_leaf ~name:"step2.1" profiler @@ fun () ->
            Time.advance 1.0
          in
          let () =
            Adhoc_profiler.count_leaf ~name:"step2.2" profiler @@ fun () ->
            Time.advance 2.0
          in
          ()
        in
        ()
    in
    ()
  in
  f true;
  Time.advance 0.6;
  f false;
  f false;
  let profile =
    Adhoc_profiler.CallTree.to_string @@ Adhoc_profiler.get_and_reset ()
  in
  let expected_profile =
    {|test_branch                                                                     3           11.570 sec   100.00%
  step1                                                                           1           0.310 sec   2.68%
  step2                                                                           2           7.500 sec   64.82%
    step2.1                                                                         2           2.020 sec   17.46%
    step2.2                                                                         2           4.020 sec   34.75%
|}
  in
  assert_strings_equal expected_profile profile;
  ()

let test_rec _ =
  let lt_zero ~prof n =
    Adhoc_profiler.count_leaf prof ~name:"lt_zero" @@ fun () ->
    Time.advance 0.1;
    n <= 0
  in
  let rec f ~prof n =
    Adhoc_profiler.count prof ~name:"f" @@ fun prof ->
    Time.advance 0.1;
    if lt_zero ~prof n then
      ()
    else
      f ~prof (n - 1)
  in
  let () = Adhoc_profiler.create ~name:"test_rec" @@ fun prof -> f ~prof 35 in
  let profile =
    Adhoc_profiler.CallTree.to_string @@ Adhoc_profiler.get_and_reset ()
  in
  let expected_profile =
    {|test_rec                                                                        1           7.630 sec   100.00%
  f                                                                               1           7.610 sec   99.74%
    f                                                                               1           7.370 sec   96.59%
      f                                                                               1           7.130 sec   93.45%
        f                                                                               1           6.890 sec   90.30%
          f                                                                               1           6.650 sec   87.16%
            f                                                                               1           6.410 sec   84.01%
              f                                                                               1           6.170 sec   80.87%
                f                                                                               1           5.930 sec   77.72%
                  f                                                                               1           5.690 sec   74.57%
                    f                                                                               1           5.450 sec   71.43%
                    lt_zero                                                                         1           0.110 sec   1.44%
                  lt_zero                                                                         1           0.110 sec   1.44%
                lt_zero                                                                         1           0.110 sec   1.44%
              lt_zero                                                                         1           0.110 sec   1.44%
            lt_zero                                                                         1           0.110 sec   1.44%
          lt_zero                                                                         1           0.110 sec   1.44%
        lt_zero                                                                         1           0.110 sec   1.44%
      lt_zero                                                                         1           0.110 sec   1.44%
    lt_zero                                                                         1           0.110 sec   1.44%
|}
  in
  assert_strings_equal expected_profile profile;
  ()

let () =
  "adhoc_profiler_test"
  >::: [
         "test1" >:: test1;
         "test_branch" >:: test_branch;
         "test_rec" >:: test_rec;
       ]
  |> run_test_tt_main
