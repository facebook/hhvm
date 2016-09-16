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

let diagnostic_subscription_id = 223

let foo_name = "foo.php"

let foo_contents = "<?hh
{
"

let foo_diagnostics = "
/foo.php:
File \"/foo.php\", line 3, characters 1-0:
Expected } (Parsing[1002])
"

let foo_clear_diagnostics = "
/foo.php:
"

let bar_name = "bar.php"

let bar_contents = "
<?hh // strict
function test() {} // missing return type
"

let bar_diagnostics = "
/bar.php:
File \"/bar.php\", line 3, characters 10-13:
Was expecting a return type hint (Typing[4030])
"

let assert_no_push_message loop_outputs =
  match loop_outputs.push_message with
  | Some _ -> Test.fail "Unexpected push message"
  | None -> ()

let () =

  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in
  (* Initially there is a single typing error in bar.php *)
  let env = Test.setup_disk env [(bar_name, bar_contents)] in
  let env = Test.subscribe_diagnostic ~id:diagnostic_subscription_id env in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  (* Initial list of errors is pushed after subscribing *)
  Test.assert_diagnostics loop_outputs bar_diagnostics;
  let env = Test.open_file env foo_name in
  (* Edit file to have errors *)
  let env, _ = Test.edit_file env foo_name foo_contents in
  let env = Test.wait env in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_outputs foo_diagnostics;
  let env = Test.wait env in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  assert_no_push_message loop_outputs;
  (* Fix the errors in file *)
  let env, _ = Test.edit_file env foo_name "" in
  let env = Test.wait env in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_outputs foo_clear_diagnostics;
  let env = Test.wait env in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  assert_no_push_message loop_outputs;
  (* Change the file, but still no errors *)
  let env, _ = Test.edit_file env foo_name "<?hh\n" in
  let env = Test.wait env in
  let _, loop_outputs = Test.(run_loop_once env default_loop_input) in
  assert_no_push_message loop_outputs
