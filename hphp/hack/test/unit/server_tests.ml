(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

let test_dmesg_parser () =
  let input = [
    "[3034339.262439] Out of memory: Kill process 2758734 (hh_server) \
    score 253 or sacrifice child";
  ] in
  ServerMonitor.find_oom_in_dmesg_output 2758734 input


let tests = [
  "test_dmesg_parser", test_dmesg_parser;
]

let run (name, f) =
  Printf.printf "Running %s ... %!" name;
  let result = f () in
  (if result
  then Printf.printf "ok\n%!"
  else Printf.printf "fail\n%!");
  result


let () =
  exit (if List.for_all tests run then 0 else 1)
