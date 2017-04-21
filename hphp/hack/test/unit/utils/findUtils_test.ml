(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let test_empty_path () =
  not (FindUtils.has_ancestor "" "hello")

let test_no_parent () =
  not (FindUtils.has_ancestor "foo.php" "hello")

let test_parent_matches () =
  FindUtils.has_ancestor "experimental/foo.php" "experimental"

let test_grandparent_matches () =
  FindUtils.has_ancestor "experimental/dict/foo.php" "experimental"

let test_no_match () =
  not (FindUtils.has_ancestor "abc/def/g/hhi/foo.php" "experimental")

let tests = [
  "empty_path", test_empty_path;
  "no_parent", test_no_parent;
  "parent_matches", test_parent_matches;
  "grandparent_matches", test_grandparent_matches;
  "no_match", test_no_match;
]

let () =
  Unit_test.run_all tests
