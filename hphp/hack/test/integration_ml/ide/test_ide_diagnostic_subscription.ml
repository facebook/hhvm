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
open Reordered_argument_collections
open ServerCommandTypes

module Test = Integration_test_base

let diagnostic_subscription_id = 223

let foo_name = "foo.php"

let foo_contents = "<?hh
{
"

let () =

  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in

  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (
      SUBSCRIBE_DIAGNOSTIC diagnostic_subscription_id
    )
  }) in
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (OPEN_FILE foo_name)
  }) in
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (EDIT_FILE
      (foo_name, [File_content.{range = None; text = foo_contents;}])
    )
  }) in
  let env = ServerEnv.{ env with last_command_time = 0.0 } in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  (match loop_outputs.push_message with
  | Some (DIAGNOSTIC (id, errors))
      when id = diagnostic_subscription_id ->
    if not @@ (SMap.cardinal errors = 1) then
      Test.fail "Expected errors for single file";
    begin match SMap.get errors (Test.prepend_root foo_name) with
      | Some [error] ->
        let error = Errors.to_string error in
        Test.assertEqual
          ("File \"/foo.php\", line 3, characters 1-0:\n" ^
          "Expected } (Parsing[1002])\n")
          error
      | _ -> Test.fail "Expected exactly one error"
    end
  | _ -> Test.fail "Expected push diagnostics");
  let env = ServerEnv.{ env with last_command_time = 0.0 } in
  let _, loop_outputs = Test.(run_loop_once env default_loop_input) in
  match loop_outputs.push_message with
  | Some _ -> Test.fail "Unexpected push message"
  | None -> ()
