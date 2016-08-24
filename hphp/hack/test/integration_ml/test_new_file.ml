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
open ServerEnv

module Test = Integration_test_base

let foo_contents = "<?hh

{
"

let () =
  let env = Test.setup_server () in
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      "foo.php", foo_contents
    ]
  }) in

  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  let expected_error =
    "File \"/foo.php\", line 4, characters 1-0:\n" ^
    "Expected } (Parsing[1002])\n" in
  Test.assertSingleError expected_error (Errors.get_error_list env.errorl);
