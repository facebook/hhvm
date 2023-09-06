(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Integration_test_base_types
module Test = Integration_test_base

let diagnostic_subscription_id = 223

let foo_name = "foo.php"

let foo_contents = "<?hh
{
"

let foo_diagnostics =
  "
/foo.php:
File \"/foo.php\", line 2, characters 1-1:
Hack does not support top level statements. Use the `__EntryPoint` attribute on a function instead (Parsing[1002])

File \"/foo.php\", line 2, characters 2-2:
A right brace `}` is expected here. (Parsing[1002])"

let foo_clear_diagnostics = "
/foo.php:
"

let bar_name = "bar.php"

let bar_contents = "<?hh // strict
function test() {} // missing return type
"

let bar_diagnostics =
  "
/bar.php:
File \"/bar.php\", line 2, characters 10-13:
Was expecting a return type hint (Typing[4030])"

let assert_no_push_message loop_outputs =
  match loop_outputs.push_messages with
  | _ :: _ -> Test.fail "Unexpected push message"
  | [] -> ()

let test () =
  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in
  (* Initially there is a single typing error in bar.php *)
  let (env, loop_outputs) = Test.change_files env [(bar_name, bar_contents)] in
  (* Initial list of errors is pushed after subscribing *)
  Test.assert_diagnostics_string loop_outputs bar_diagnostics;

  (* Open and edit file to have errors *)
  let env = Test.open_file env foo_name ~contents:foo_contents in
  let env = Test.wait env in
  let (env, loop_outputs) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_string loop_outputs foo_diagnostics;
  let env = Test.wait env in
  let (env, _loop_outputs) = Test.(run_loop_once env default_loop_input) in

  (* assert_no_push_message loop_outputs; TODO: ??? *)

  (* Fix the errors in file *)
  let (env, _) = Test.edit_file env foo_name "" in
  let env = Test.wait env in
  let (env, loop_outputs) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_string loop_outputs foo_clear_diagnostics;
  let env = Test.wait env in
  let (env, _loop_outputs) = Test.(run_loop_once env default_loop_input) in

  (* assert_no_push_message loop_outputs; TODO: ??? *)

  (* Change the file, but still no new errors *)
  let (env, _) = Test.edit_file env foo_name "<?hh\n" in
  let env = Test.wait env in
  let (_, loop_outputs) = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_diagnostics loop_outputs
