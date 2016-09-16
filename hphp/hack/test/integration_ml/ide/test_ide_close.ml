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

let foo_name = "foo.php"

let foo_disk_contents =
"<?hh // strict

function foo(string $x) : void {}

function test() : void {
  foo(4);
}
"

let foo_disk_errors = "
File \"/foo.php\", line 6, characters 7-7:
Invalid argument (Typing[4110])
File \"/foo.php\", line 3, characters 14-19:
This is a string
File \"/foo.php\", line 6, characters 7-7:
It is incompatible with an int
"

let foo_disk_diagnostics = "
/foo.php:
File \"/foo.php\", line 6, characters 7-7:
Invalid argument (Typing[4110])
File \"/foo.php\", line 3, characters 14-19:
This is a string
File \"/foo.php\", line 6, characters 7-7:
It is incompatible with an int
"

let foo_ide_contents =
"<?hh // strict

{
"

let foo_ide_errors = "
File \"/foo.php\", line 4, characters 1-0:
Expected } (Parsing[1002])
"
let foo_ide_diagnostics = "
/foo.php:
File \"/foo.php\", line 4, characters 1-0:
Expected } (Parsing[1002])
"

let () =

  let env = Test.setup_server () in
  let env = Test.setup_disk env [
    foo_name, foo_disk_contents
  ] in

  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in

  Test.assertSingleError foo_disk_errors (Errors.get_error_list env.errorl);
  Test.assert_diagnostics loop_output foo_disk_diagnostics;

  let env = Test.open_file env foo_name in
  let env, _ = Test.edit_file env foo_name foo_ide_contents in
  let env = Test.wait env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in

  (* Force update to global error list *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse ServerCommandTypes.STATUS)
  }) in

  Test.assertSingleError foo_ide_errors (Errors.get_error_list env.errorl);
  Test.assert_diagnostics loop_output foo_ide_diagnostics;

  (* Close the file and check that error lists are back to reflecting disk
   * contents *)
  let env, _ = Test.close_file env foo_name in
  let env = Test.wait env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in

  let env, _ = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse ServerCommandTypes.STATUS)
  }) in

  Test.assertSingleError foo_disk_errors (Errors.get_error_list env.errorl);
  Test.assert_diagnostics loop_output foo_disk_diagnostics
