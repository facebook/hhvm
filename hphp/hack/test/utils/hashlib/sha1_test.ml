(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Asserter

let strip_output_empty_string () =
  let result = Sha1.strip_output_filename "" in
  match result with
  | None ->
    true
  | Some s ->
    Printf.eprintf "Error. Expected None but got Some %s" s;
    false

(** Just for resilience. *)
let strip_output_removes_tail_newline () =
  let expected = "f572d396fae9206628714fb2ce00f72e94f2258f" in
  let result = Sha1.strip_output_filename
    "f572d396fae9206628714fb2ce00f72e94f2258f  -\n" in
  match result with
  | None ->
    Printf.eprintf "Error. Sha1.strip_output_filename returned None\n";
    false
  | Some result ->
    String_asserter.assert_equals expected result
      "Sha1.strip_output_filename should strip newline at the end";
    true


let test_sha1sum_cmd () =
  let result = Sha1.digest "hello" in
  match result with
  | None ->
    Printf.eprintf "Error Sha1.digest returned None\n";
    false
  | Some digest ->
    String_asserter.assert_equals "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d"
      digest "sha1 hello with newline digest";
    true

let tests = [
  ("test_sha1sum_cmd", test_sha1sum_cmd);
  ("strip_output_empty_string", strip_output_empty_string);
  ("strip_output_removes_tail_newline", strip_output_removes_tail_newline);
]

let () =
  Unit_test.run_all tests
