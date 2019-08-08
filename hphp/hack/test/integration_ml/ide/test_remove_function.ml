(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

module Test = Integration_test_base

let foo_name = "foo.php"

let foo_contents = "<?hh // strict

function foo() : void {}
"

let bar_name = "bar.php"

let bar_contents = "<?hh // strict

function test(): void {
  foo();
}
"

let full_diagnostics =
  "
/bar.php:
File \"/bar.php\", line 4, characters 3-5:
Unbound name: foo (a global function) (Naming[2049])

File \"/bar.php\", line 4, characters 3-5:
Unbound name (typing): foo (Typing[4107])
"

let test () =
  let env = Test.setup_server () in
  let env =
    Test.setup_disk
      env
      [(foo_name, foo_contents); (bar_name, bar_contents) (* no errors *)]
  in
  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  let (env, loop_outputs) = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_diagnostics loop_outputs;

  (* no diagnostics initially *)
  let env = Test.open_file env foo_name in
  (* Delete foo() *)
  let (env, _) = Test.edit_file env foo_name "" in
  let env = Test.wait env in
  let (env, loop_outputs) = Test.(run_loop_once env default_loop_input) in
  (* Change introduces an error in bar.php, but this file is not open in IDE
   * so we don't recheck it immediately. *)
  Test.assert_no_diagnostics loop_outputs;

  (* Asking for global error list will trigger recheck of bar.php *)
  let (_, loop_outputs) = Test.full_check env in
  Test.assert_diagnostics loop_outputs full_diagnostics
