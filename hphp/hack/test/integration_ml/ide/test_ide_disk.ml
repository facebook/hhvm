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
  | es ->
    let buf = Buffer.create 256 in
    Buffer.add_string buf "Expected to have no errors, but instead got: ";
    List.iter
      (fun e ->
        Buffer.add_string buf "\n  - ";
        let str = Errors.to_string (Errors.to_absolute e) in
        Buffer.add_string buf str
      ) es;
    let msg = Buffer.contents buf in
    Test.fail msg

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
  let env = Test.subscribe_diagnostic env in
  (* There are no errors initially *)
  check_has_no_errors env;
  (* Open pre-existing file in editor *)
  let env = Test.open_file env foo_name in

  (* Update disk contents to contain errors *)
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      foo_name, foo_with_errors
    ]
  }) in
  assert loop_output.did_read_disk_changes;
  (* But since file is open in editor, contents from disk are ignored *)
  check_has_no_errors env;
  Test.assert_no_diagnostics loop_output;

  (* We edit the file to content with errors *)
  let env, _ = Test.edit_file env foo_name foo_with_errors in
  (* Sending a command just schedules a recheck, but it doesn't happen
   * immediately *)
  check_has_no_errors env;
  Test.assert_no_diagnostics loop_output;
  (* Simulate time passing since last command to trigger a recheck *)
  let env = Test.wait env in
  (* Next iteration executes the recheck and generates the errors *)
  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  (* IDE edits only create diagnostics, they don't update global error list *)
  check_has_no_errors env;
  Test.assert_has_diagnostics loop_output;

  (* Edit file back to have no errors *)
  let env, _ = Test.edit_file env foo_name foo_no_errors in
  let env = Test.wait env in
  let env, _ = Test.(run_loop_once env default_loop_input) in
  Test.assert_has_diagnostics loop_output;
  check_has_no_errors env;
  (* We close the file, disk contents should be taken into account again *)
  let env, _ = Test.close_file env foo_name in
  let env = Test.wait env in
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
