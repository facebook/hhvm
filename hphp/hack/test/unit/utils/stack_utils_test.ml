(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Stack_utils

let merge_bytes_test () =
  let stack = Stack.create () in
  Stack.push "abc" stack;
  Stack.push "def" stack;
  let result = Stack.merge_bytes stack in
  Asserter.String_asserter.assert_equals "abcdef" result "";
  true

let tests = [
  ("merge_bytes_test", merge_bytes_test);
]

let () =
  Unit_test.run_all tests
