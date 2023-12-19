(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Integration_test_base_types
module Test = Integration_test_base

let foo_name = "foo.php"

let foo_contents_before =
  "<?hh // strict

class Foo {
  public function f() : void {}
}
"

let foo_contents_after = "<?hh // strict

class Foo {}
"

let foo_child_name = "foo_child.php"

let foo_child_contents = "<?hh // strict

class FooChild extends Foo {}
"

let bar_name = "bar.php"

let bar_contents =
  "<?hh // strict

function test(FooChild $foo_child) : void {
  $foo_child->f();
}
"

let bar_errors =
  "File \"/bar.php\", line 4, characters 15-15:
No instance method `f` in `FooChild` (Typing[4053])
  File \"/bar.php\", line 3, characters 15-22:
  This is why I think it is an object of type FooChild
  File \"/foo_child.php\", line 3, characters 7-14:
  Declaration of `FooChild` is here
"

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents = ""

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) = ServerConfig.load ~silent:false options in
  let env = Test.setup_server ~custom_config () in
  let env =
    Test.setup_disk
      env
      [
        (foo_name, foo_contents_before);
        (foo_child_name, foo_child_contents);
        (bar_name, bar_contents);
      ]
  in
  (* We need to suppress all the errors (see HH_FIXMEs above), otherwise the
   * logic that always rechecks the files with errors kicks in and does the
   * same job as phase2 fanout. We want to test the latter one in this test. *)
  Test.assert_no_errors env;

  (* restore parent, but with a mismatching return type of f() *)
  let (env, _) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes = [(foo_name, foo_contents_after)];
        })
  in
  Test.assertSingleError bar_errors (Errors.get_error_list env.ServerEnv.errorl)
