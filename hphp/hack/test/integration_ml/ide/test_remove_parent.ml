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

let foo_parent_missing_diagnostics =
  {|
/foo.php:
File "/foo.php", line 3, characters 19-27:
Unbound name: `FooParent` (an object type) (Naming[2049])

File "/foo.php", line 5, characters 12-14:
No instance method `lol` in `Foo` (Typing[4053])
  File "/foo.php", line 4, characters 19-22:
  Did you mean `test` instead?
  File "/foo.php", line 3, characters 7-9:
  This is why I think it is an object of type Foo
  File "/foo.php", line 3, characters 7-9:
  Declaration of `Foo` is here
|}

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env =
    Test.Client.setup_disk
      env
      [(foo_parent_name, foo_parent_contents); (foo_name, foo_contents)]
  in

  let (env, diagnostics) = Test.Client.open_file env foo_name in
  Test.Client.assert_no_diagnostics diagnostics;

  let env = Test.Client.setup_disk env [(foo_parent_name, "")] in
  let (env, diagnostics) = Test.Client.open_file env foo_name in
  Test.Client.assert_diagnostics_string
    diagnostics
    foo_parent_missing_diagnostics;

  let env =
    Test.Client.setup_disk env [(foo_parent_name, foo_parent_contents)]
  in
  let (env, diagnostics) = Test.Client.open_file env foo_name in
  Test.Client.assert_no_diagnostics diagnostics;

  ignore env;
  ()
