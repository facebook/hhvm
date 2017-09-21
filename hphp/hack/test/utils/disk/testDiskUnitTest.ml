(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Some tests apply only to the TestDisk module - we don't want to modify
 * the real hard disk. *)

(** Test writing to cwd directly without using realpath expansion in
* Path module. *)
let test_write_and_read_directly_cwd () =
  Disk.write_file ~file:"./a.txt" ~contents:"hello";
  let contents = Disk.cat "./a.txt" in
  contents = "hello"

let tests = [
  ("test_write_and_read_directly_cwd", test_write_and_read_directly_cwd);
]

let () =
  Unit_test.run_all tests
