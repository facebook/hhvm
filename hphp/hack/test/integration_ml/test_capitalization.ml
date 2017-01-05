(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Integration_test_base_types

module Test = Integration_test_base

let foo1_contents = "<?hh
  class foo {}
"
let foo2_contents = "<?hh
  class FOO {}
"

let () =
  let env = Test.setup_server () in
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      "foo1.php", foo1_contents;
      "foo2.php", foo2_contents;
    ]
  }) in

  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  (* TODO: get rid of repeating errors in error list *)
  let expected_error =
  "File \"/foo2.php\", line 2, characters 9-11:\n" ^
  "Could not find FOO (Naming[2006])\n" ^
  "File \"/foo1.php\", line 2, characters 9-11:\n" ^
  "Did you mean foo?\n\n" ^
  "File \"/foo2.php\", line 2, characters 9-11:\n" ^
  "Could not find FOO (Naming[2006])\n" ^
  "File \"/foo1.php\", line 2, characters 9-11:\n" ^
  "Did you mean foo?\n\n" ^
  "File \"/foo2.php\", line 2, characters 9-11:\n" ^
  "Name already bound: FOO (Naming[2012])\n" ^
  "File \"/foo1.php\", line 2, characters 9-11:\n" ^
  "Previous definition foo differs only in capitalization\n"
  in
  Test.assert_errors env expected_error;
  (* Change a wholly unrelated file. *)
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      "bar.php", ""
    ]
  }) in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  (* The same errors should still be there *)
  Test.assert_errors env expected_error;
