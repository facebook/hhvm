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
let foo_contents =
"<?hh // strict

class Foo {
  public function f() : string {
    // UNSAFE
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

let bar_errors =
"File \"/bar.php\", line 6, characters 12-26:
Invalid argument (Typing[4110])
File \"/bar.php\", line 3, characters 19-21:
This is an int
File \"/foo.php\", line 4, characters 25-30:
It is incompatible with a string
"

let () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env [
    foo_name, "";
    foo_child_name, foo_child_contents;
    bar_name, bar_contents;
  ] in

  (* We need to suppress all the errors (see HH_FIXMEs above), otherwise the
   * logic that always rechecks the files with errors kicks in and does the
   * same job as phase2 fanout. We want to test the latter one in this test. *)
  Test.assert_no_errors env;

  (* restore parent, but with a mismatching return type of f() *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      foo_name, foo_contents;
    ]
  }) in
  Test.assertSingleError bar_errors (Errors.get_error_list env.ServerEnv.errorl)
