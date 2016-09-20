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

let take_int_name = "take_int.php"
let take_int_contents =
"<?hh // strict

function take_int(int $x) : void {}
"

let foo_name = "foo.php"
let foo_contents = Printf.sprintf
"<?hh // strict

class Foo {
  public function f() : %s {
    // UNSAFE
  }
}
"

let foo_child_name = "foo_child.php"
let foo_child_contents =
"<?hh // strict

class FooChild extends Foo {}
"

let bar_name = "bar.php"
let bar_contents =
"<?hh // strict

function test_foo(Foo $foo) : void {
  take_int($foo->f());
}
"

let baz_name = "baz.php"
let baz_contents =
"<?hh // strict

function test_foo_child(FooChild $foo_child) : void {
  take_int($foo_child->f());
}
"

let diagnostics = "
/bar.php:
File \"/bar.php\", line 4, characters 12-20:
Invalid argument (Typing[4110])
File \"/take_int.php\", line 3, characters 19-21:
This is an int
File \"/foo.php\", line 4, characters 25-30:
It is incompatible with a string

/baz.php:
File \"/baz.php\", line 4, characters 12-26:
Invalid argument (Typing[4110])
File \"/take_int.php\", line 3, characters 19-21:
This is an int
File \"/foo.php\", line 4, characters 25-30:
It is incompatible with a string
"

let () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env [
    take_int_name, take_int_contents;
    foo_name, (foo_contents "int");
    foo_child_name, foo_child_contents;
    bar_name, bar_contents;
    baz_name, baz_contents;
  ] in

  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in

  let env = Test.open_file env foo_name in
  let env = Test.open_file env bar_name in
  let env = Test.open_file env baz_name in

  let env = Test.wait env in
  let env, _ = Test.(run_loop_once env default_loop_input) in

  let env, _ = Test.edit_file env foo_name (foo_contents "string") in
  let env = Test.wait env in
  let _, loop_output = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output diagnostics
