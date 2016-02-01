(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let test_process_data =
  ServerProcess.{
    pid = 2758734;
    name = "hh_server";
    start_t = 0.0;
    in_fd = Unix.stdin;
    out_fd = Unix.stdout;
    log_file = "";
    last_request_handoff = ref 0.0;
  }

let test_dmesg_parser () =
  let input = [
    "[3034339.262439] Out of memory: Kill process 2758734 (hh_server) \
    score 253 or sacrifice child";
  ] in
  ServerProcessTools.find_oom_in_dmesg_output test_process_data input

let tests = [
  "test_dmesg_parser", test_dmesg_parser;
]

let () =
  Unit_test.run_all tests
