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

let foo_parent_name = "foo_parent.php"

let foo_parent_contents =
"<?hh // strict

class FooParent {
  public function lol() : void {}
}
"

let foo_name = "foo.php"

let foo_contents =
"<?hh // strict

class Foo extends FooParent {
  public function test() : void {
    $this->lol();
  }
}
"

let foo_parent_missing_diagnostics = "
/foo.php:
File \"/foo.php\", line 3, characters 7-9:
Foo has a non-<?hh grandparent; this is not allowed in strict mode because " ^
"that parent may define methods of unknowable name and type (Typing[4123])

File \"/foo.php\", line 3, characters 19-27:
Unbound name: FooParent (an object type) (Naming[2049])

File \"/foo.php\", line 3, characters 19-27:
Unbound name: FooParent (an object type) (Naming[2049])
"

let clear_foo_diagnostics = "
/foo.php:

"

let () =

  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in

  let env = Test.open_file env foo_parent_name ~contents:foo_parent_contents in
  let env = Test.open_file env foo_name ~contents:foo_contents in

  let env = Test.wait env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_diagnostics loop_output;

  let env, _ = Test.edit_file env foo_parent_name "" in
  let env = Test.wait env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output foo_parent_missing_diagnostics;

  let env, _ = Test.edit_file env foo_parent_name foo_parent_contents in
  let env = Test.wait env in
  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output clear_foo_diagnostics;

  let env, _ = Test.edit_file env foo_parent_name "" in
  let env = Test.wait env in
  let _, loop_output = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output foo_parent_missing_diagnostics
