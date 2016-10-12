(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Test = Integration_test_base

let foo_name = "foo.php"

let foo_contents = Printf.sprintf
"<?hh // strict

function foo() : %s {
  //UNSAFE
}
"

let foo_returns_int = foo_contents "int"
let foo_returns_string = foo_contents "string"

let bar_name = "bar.php"

let bar_contents =
"<?hh // strict


function test(): int {
  return foo();
}
"

let bar_diagnostics = "
/bar.php:
File \"/bar.php\", line 5, characters 10-14:
Invalid return type (Typing[4110])
File \"/bar.php\", line 4, characters 18-20:
This is an int
File \"/foo.php\", line 3, characters 18-23:
It is incompatible with a string
"

let bar_clear = "
/bar.php:
"

let () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env [
    foo_name, foo_returns_int;
    bar_name, bar_contents;
  ] in

  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in

  Test.assert_no_errors env;
  Test.assert_no_diagnostics loop_output;

  (* Open both files and make an edit to foo that reslts in errors in bar *)
  let env = Test.open_file env foo_name in
  let env = Test.open_file env bar_name in
  let env, _ = Test.edit_file env foo_name foo_returns_string in
  let env = Test.wait env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output bar_diagnostics;

  (* Close bar, make sure that errors is still there *)
  let env, _ = Test.close_file env bar_name in
  let env = Test.wait env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output bar_diagnostics;

  (* Fix foo, check that the error in bar was cleared - even though the
   * file is not currently open in IDE *)
  let env, _ = Test.edit_file env foo_name foo_returns_int in
  let env = Test.wait env in
  let _, loop_output = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output bar_clear
