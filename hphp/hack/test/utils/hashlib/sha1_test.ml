(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Asserter

let test_sha1sum_cmd () =
  let result = Sha1.digest "hello" in
  String_asserter.assert_equals
    "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d"
    result
    "sha1 hello with newline digest";
  true

let tests = [("test_sha1sum_cmd", test_sha1sum_cmd)]

let () = Unit_test.run_all tests
