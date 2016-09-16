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

let () =

  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in

  let env = Test.subscribe_diagnostic ~id:diagnostic_subscription_id env in
  let env = Test.open_file env foo_name in
  let env, _ = Test.edit_file env foo_name foo_contents in
  let env = Test.wait env in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_outputs foo_diagnostics;
  let env = Test.wait env in
  let _, loop_outputs = Test.(run_loop_once env default_loop_input) in
  match loop_outputs.push_message with
  | Some _ -> Test.fail "Unexpected push message"
  | None -> ()
