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

let foo_contents =
  "<?hh // strict

class Foo {
  /* HH_FIXME[4110] */
  public function f() : string {
  }
}
"

let foo_child_name = "foo_child.php"

let foo_child_contents =
  "<?hh // strict

/* HH_FIXME[2049] */
/* HH_FIXME[4123] */
class FooChild extends Foo {}
"

let bar_name = "bar.php"

let bar_contents =
  "<?hh // strict

function take_int(int $x) : void {}

function test(FooChild $foo_child) : void {
  take_int($foo_child->f());
}
"

let bar_diagnostics =
  "
/bar.php:
File \"/bar.php\", line 6, characters 12-26:
Invalid argument (Typing[4110])
File \"/bar.php\", line 3, characters 19-21:
Expected int
File \"/foo.php\", line 5, characters 25-30:
But got string
"

let test () =
  let env = Test.setup_server () in
  let env =
    Test.setup_disk
      env
      [
        (foo_name, "");
        (foo_child_name, foo_child_contents);
        (bar_name, bar_contents);
      ]
  in
  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_diagnostics loop_output;

  let env = Test.open_file env foo_name in
  let env = Test.open_file env bar_name in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_diagnostics loop_output;

  let (env, _) = Test.edit_file env foo_name foo_contents in
  let env = Test.wait env in
  let (_, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output bar_diagnostics
