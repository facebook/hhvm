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

let take_int_name = "take_int.php"

let take_int_contents = "<?hh // strict

function take_int(int $x) : void {}
"

let foo_name = "foo.php"

let foo_contents =
  Printf.sprintf
    "<?hh // strict

class Foo {
  /* HH_FIXME[4336] */
  public function f() : %s {
  }
}
"

let foo_child_name = "foo_child.php"

let foo_child_contents = "<?hh // strict

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

let diagnostics =
  "
/bar.php:
File \"/bar.php\", line 4, characters 12-20:
Invalid argument (Typing[4110])
  File \"/take_int.php\", line 3, characters 19-21:
  Expected `int`
  File \"/foo.php\", line 5, characters 25-30:
  But got `string`

/baz.php:
File \"/baz.php\", line 4, characters 12-26:
Invalid argument (Typing[4110])
  File \"/take_int.php\", line 3, characters 19-21:
  Expected `int`
  File \"/foo.php\", line 5, characters 25-30:
  But got `string`
"

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents =
  "
allowed_fixme_codes_strict = 4336
allowed_decl_fixme_codes = 4336
"

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let hhconfig_path =
    Relative_path.create Relative_path.Root hhconfig_filename
  in
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) =
    ServerConfig.load ~silent:false hhconfig_path options
  in
  let env = Test.setup_server ~custom_config () in
  let env =
    Test.setup_disk
      env
      [
        (take_int_name, take_int_contents);
        (foo_name, foo_contents "int");
        (foo_child_name, foo_child_contents);
        (bar_name, bar_contents);
        (baz_name, baz_contents);
      ]
  in
  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  let env = Test.open_file env foo_name in
  let env = Test.open_file env bar_name in
  let env = Test.open_file env baz_name in
  let env = Test.wait env in
  let (env, _) = Test.(run_loop_once env default_loop_input) in
  let (env, _) = Test.edit_file env foo_name (foo_contents "string") in
  let env = Test.wait env in
  let (_, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output diagnostics
