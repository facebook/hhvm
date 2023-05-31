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

let foo_parent_name = "foo_parent.php"

let foo_parent_contents1 =
  "<?hh // strict

class FooParent {
  public function lol() : void {}
}
"

let foo_parent_contents2 = "<?hh // strict

class FooParent {}
"

let foo_name = "foo.php"

let foo_contents1 =
  "<?hh // strict

class Foo extends FooParent {
  public function bar() : void {
  }
}
"

let foo_contents2 = "<?hh // strict

class Fo
"

let foo_diagnostics =
  "
/foo.php:
File \"/foo.php\", line 3, characters 9-9:
A left brace `{` is expected here. (Parsing[1002])
"

let autocomplete_contents =
  "<?hh

function test(Foo $foo) {
  $foo->AUTO332
}

"

let test () =
  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in
  let env = Test.open_file env foo_parent_name ~contents:foo_parent_contents1 in
  let env = Test.open_file env foo_name ~contents:foo_contents1 in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_diagnostics loop_output;

  (* Introduce an irrecoverable parsing error in Foo definition - if we would
   * proceed to try to redeclare foo.php, Foo will no longer exist *)
  let (env, _) = Test.edit_file env foo_name foo_contents2 in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_string loop_output foo_diagnostics;

  (* Change the Foo parent to trigger invalidating Foo *)
  let (env, _) = Test.edit_file env foo_parent_name foo_parent_contents2 in
  let env = Test.wait env in
  let (env, _) = Test.(run_loop_once env default_loop_input) in
  (* Check that Foo definiton is still available for querying *)
  let env = Test.open_file env "test.php" ~contents:autocomplete_contents in
  let (_, loop_output) = Test.ide_autocomplete env ("test.php", 4, 9) in
  Test.assert_ide_autocomplete_does_not_contain loop_output ["bar"]
