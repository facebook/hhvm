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
open ServerCommandTypes
open ServerEnv

module Test = Integration_test_base

let foo_name = "foo.php"

let foo_no_errors = "<?hh
function foo() {

}
"

let foo_with_errors = "<?hh
{"

let bar_name = "bar.php"

let foo2_definition = "function foo2() {}\n"

let bar_contents = "<?hh
function test() {
  fo
}
"

let check_has_no_errors env =
  match Errors.get_error_list env.errorl with
  | [] -> ()
  | _ -> Test.fail "Expected to have no errors"

let check_has_errors env =
  match Errors.get_error_list env.errorl with
  | [] -> Test.fail "Expected to have errors"
  | _ -> ()

let () =

  let env = Test.setup_server () in
  let env = Test.setup_disk env [
    foo_name, foo_no_errors
  ] in
  let env = Test.connect_persistent_client env in
  (* There are no errors initially *)
  check_has_no_errors env;

  (* Open pre-existing file in editor *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (OPEN_FILE foo_name)
  }) in

  (* Update disk contents to contain errors *)
  let env, loop_output = Test.(run_loop_once env {default_loop_input with
    disk_changes = [
      foo_name, foo_with_errors
    ]
  }) in
  assert loop_output.did_read_disk_changes;
  (* But since file is open in editor, contents from disk are ignored *)
  check_has_no_errors env;

  (* We edit the file to content with errors *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (EDIT_FILE
      (foo_name, [File_content.{range = None; text = foo_with_errors;}])
    )
  }) in
  (* Sending a command just schedules a recheck, but it doesn't happen
   * immediately *)
  check_has_no_errors env;
  (* Simulate time passing since last command to trigger a recheck *)
  let env = { env with last_command_time = 0.0 } in
  (* Next iteration executes the recheck and generates the errors *)
  let env, _ = Test.(run_loop_once env default_loop_input) in
  check_has_errors env;

  (* Edit file back to have no errors *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (EDIT_FILE
      (foo_name, [File_content.{range = None; text = foo_no_errors;}])
    )
  }) in
  let env = { env with last_command_time = 0.0 } in
  let env, _ = Test.(run_loop_once env default_loop_input) in
  check_has_no_errors env;
  (* We close the file, disk contents should be taken into account again *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (CLOSE_FILE foo_name)
  }) in

  let env = { env with last_command_time = 0.0 } in
  (* TODO: this should be unnecessary, closing the file should recheck the disk
   * contents automatically *)
  let env, _  = Test.(run_loop_once env {default_loop_input with
    disk_changes = [
      foo_name, foo_with_errors
    ]
  }) in
  assert loop_output.did_read_disk_changes;
  (* Disk errors are now reflected *)
  check_has_errors env;
