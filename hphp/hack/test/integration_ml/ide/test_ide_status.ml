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

let foo_name = "foo.php"
let foo_takes_int_contents = "
<?hh // strict

function foo(int $x) : void {}
"

let foo_takes_string_contents = "
<?hh // strict

function foo(string $x) : void {}
"

let bar_name = "bar.php"
let bar_contents = "<?hh // strict

function test(): void {
  foo(4);
}
"

let full_diagnostics = "
/bar.php:
File \"/bar.php\", line 4, characters 7-7:
Invalid argument (Typing[4110])
File \"/foo.php\", line 4, characters 14-19:
This is a string
File \"/bar.php\", line 4, characters 7-7:
It is incompatible with an int
"

let () =

  let env = Test.setup_server () in
  let env = Test.setup_disk env [
    (foo_name, foo_takes_int_contents);
    (bar_name, bar_contents); (* no errors *)
  ] in
  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_diagnostics loop_outputs; (* no diagnostics initially *)
  let env = Test.open_file env foo_name in
  let env, _ = Test.edit_file env foo_name foo_takes_string_contents in
  let env = Test.wait env in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  (* Change introduces an error in bar.php, but this file is not open in IDE
   * so we don't recheck it immediately. *)
  Test.assert_no_diagnostics loop_outputs;
  (* Asking for global error list will trigger recheck of bar.php *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse (ServerCommandTypes.STATUS))
  }) in
  let _, loop_outputs = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_outputs full_diagnostics
