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

let bar_initial_diagnostics =
  "
/bar.php:
File \"/bar.php\", line 6, characters 24-24:
No instance method `f` in `FooChild` (Typing[4053])
  File \"/bar.php\", line 5, characters 15-22:
  This is why I think it is an object of type FooChild
  File \"/foo_child.php\", line 5, characters 7-14:
  Declaration of `FooChild` is here
"

let bar_diagnostics =
  "
/bar.php:
File \"/bar.php\", line 6, characters 12-26:
Invalid argument (Typing[4110])
  File \"/bar.php\", line 3, characters 19-21:
  Expected `int`
  File \"/foo.php\", line 5, characters 25-30:
  But got `string`
"

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env =
    Test.Client.setup_disk
      env
      [
        (foo_name, "");
        (foo_child_name, foo_child_contents);
        (bar_name, bar_contents);
      ]
  in

  (* At first, bar.php reports "no member f found in FooChild"*)
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_diagnostics_string diagnostics bar_initial_diagnostics;

  (* We will define member f in Foo (the base of FooChild) *)
  let env = Test.Client.setup_disk env [(foo_name, foo_contents)] in
  (* But ClientIdeDaemon plays fast-and-lose with invalidation. It doesn't
     flag that FooChild decl must be recomputed until we open foo_child.php.
     And it doesn't flag that bar.php's TAST must be recomputed until
     we close and open that file. *)
  let (env, _diagnostics) = Test.Client.open_file env foo_child_name in
  let (env, _diagnostics) = Test.Client.close_file env bar_name in
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_diagnostics_string diagnostics bar_diagnostics;

  ignore env;
  ()
