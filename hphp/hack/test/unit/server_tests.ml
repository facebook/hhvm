(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

let test_process_data =
  ServerProcess.
    {
      pid = 2758734;
      finale_file = "2758734.fin";
      start_t = 0.0;
      in_fd = Unix.stdin;
      out_fds = [("default", Unix.stdout)];
      last_request_handoff = ref 0.0;
    }

let test_dmesg_parser () =
  let input =
    [ "[3034339.262439] Out of memory: Kill process 2758734 (hh_server) score 253 or sacrifice child"
    ]
  in
  Sys_utils.find_oom_in_dmesg_output
    test_process_data.ServerProcess.pid
    "hh_server"
    input

let ensure_count (count : int) : unit =
  let deferred = Deferred_decl.get ~f:(fun d -> d) in
  Asserter.Int_asserter.assert_equals
    count
    (List.length deferred)
    "The number of deferred items should match the expected value"

let test_deferred_decl_add () =
  Deferred_decl.reset ();
  ensure_count 0;

  Deferred_decl.add (Relative_path.create Relative_path.Dummy "foo");
  ensure_count 1;

  Deferred_decl.add (Relative_path.create Relative_path.Dummy "foo");
  ensure_count 1;

  Deferred_decl.add (Relative_path.create Relative_path.Dummy "bar");
  ensure_count 2;

  Deferred_decl.reset ();
  ensure_count 0;

  true

let ensure_threshold ~(threshold : int) ~(limit : int) ~(expected : int) : unit
    =
  Deferred_decl.reset ();
  ensure_count 0;

  let deferred_count = ref 0 in
  for i = 1 to limit do
    let path = Printf.sprintf "foo-%d" i in
    let relative_path = Relative_path.create Relative_path.Dummy path in
    try Deferred_decl.should_defer ~d:relative_path ~threshold
    with Deferred_decl.Defer d ->
      Asserter.Bool_asserter.assert_equals
        (i >= threshold)
        true
        (Printf.sprintf
           "We should have reached the threshold %d, i=%d"
           threshold
           i);
      Asserter.String_asserter.assert_equals
        (Relative_path.suffix d)
        path
        "The deferred path should be the last one we saw";
      deferred_count := !deferred_count + 1
  done;

  Asserter.Int_asserter.assert_equals
    expected
    !deferred_count
    (Printf.sprintf
       "Unexpected deferred count; threshold: %d; limit: %d; expected: %d"
       threshold
       limit
       expected)

let test_deferred_decl_should_defer () =
  ensure_threshold ~threshold:0 ~limit:1 ~expected:1;
  ensure_threshold ~threshold:1 ~limit:2 ~expected:1;
  ensure_threshold ~threshold:2 ~limit:1 ~expected:0;
  ensure_threshold ~threshold:1 ~limit:5 ~expected:4;

  true

let tests =
  [ ("test_deferred_decl_add", test_deferred_decl_add);
    ("test_deferred_decl_should_defer", test_deferred_decl_should_defer);
    ("test_dmesg_parser", test_dmesg_parser) ]

let () = Unit_test.run_all tests
