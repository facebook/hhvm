(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Buffered_line_reader
open Asserter

let test_mixed_read () =
  let open String_asserter in
  (* Test line-based reading, LF-terminated *)
  let msg1 = "hello line" in
  (* Test line-based reading, CRLF-terminated *)
  let msg2 = "world line" in
  (* Test an empty CRLF-terminated line, like at the end of http headers *)
  let msg3 = "" in
  (* Test a length-based string that's sent in two separate writes *)
  let msg4a = "bytes" in
  let msg4b = " without newlines" in
  (* Test a length-based string that includes LF *)
  let msg5 = "bytes text with \n newlines" in
  (* Test a few bytes of length-based, followed by line-based *)
  let msg6 = "bytes" in
  let msg7 = "line" in
  (* Must do this with threads because of blocking get_next_bytes of msg4a+b *)
  let (fd_in, fd_out) = Unix.pipe () in
  match Unix.fork () with
  | 0 ->
    Unix.close fd_in;
    let _ = Unix.write fd_out (msg1 ^ "\n") 0 (String.length msg1 + 1) in
    let _ = Unix.write fd_out (msg2 ^ "\r\n") 0 (String.length msg2 + 2) in
    let _ = Unix.write fd_out (msg3 ^ "\r\n") 0 (String.length msg3 + 2) in
    let _ = Unix.write fd_out msg4a 0 (String.length msg4a) in
    let _ = Unix.sleepf 0.1 in
    let _ = Unix.write fd_out msg4b 0 (String.length msg4b) in
    let _ = Unix.write fd_out msg5 0 (String.length msg5) in
    let _ = Unix.write fd_out msg6 0 (String.length msg6) in
    let _ = Unix.write fd_out (msg7 ^ "\n") 0 (String.length msg7 + 1) in
    Unix.close fd_out;
    exit 0
  | pid ->
    Unix.close fd_out;
    let reader = Buffered_line_reader.create fd_in in
    assert_equals msg1 (get_next_line reader) "msg1";
    assert_equals msg2 (get_next_line reader) "msg2";
    assert_equals msg3 (get_next_line reader) "msg3";
    let msg4 = msg4a ^ msg4b in
    assert_equals msg4 (get_next_bytes reader (String.length msg4)) "msg4";
    assert_equals msg5 (get_next_bytes reader (String.length msg5)) "msg5";
    assert_equals msg6 (get_next_bytes reader (String.length msg6)) "msg6";
    assert_equals msg7 (get_next_line reader) "msg7";
    Unix.close fd_in;
    true

let tests = [
  "test_mixed_read", test_mixed_read;
]

let () =
  Unit_test.run_all tests
