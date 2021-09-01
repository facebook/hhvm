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

  let get () = !time
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
  let profile = Adhoc_profiler.to_string @@ Adhoc_profiler.get_and_reset () in
  let expected_profile =
    {|test1                                                                           2           10.600 sec   100.00%
  step1                                                                           2           0.600 sec   5.66%
  step2                                                                           2           7.400 sec   69.81%
    step2.1                                                                         2           2.000 sec   18.87%
    step2.2                                                                         2           4.000 sec   37.74%
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
  let profile = Adhoc_profiler.to_string @@ Adhoc_profiler.get_and_reset () in
  let expected_profile =
    {|test_branch                                                                     3           11.400 sec   100.00%
  step1                                                                           1           0.300 sec   2.63%
  step2                                                                           2           7.400 sec   64.91%
    step2.1                                                                         2           2.000 sec   17.54%
    step2.2                                                                         2           4.000 sec   35.09%
|}
  in
  assert_strings_equal expected_profile profile;
  ()

let () =
  "adhoc_profiler_test"
  >::: ["test1" >:: test1; "test_branch" >:: test_branch]
  |> run_test_tt_main
