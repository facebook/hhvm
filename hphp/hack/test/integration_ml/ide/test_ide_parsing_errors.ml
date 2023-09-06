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
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env =
    Test.Client.setup_disk
      env
      [(foo_parent_name, foo_parent_contents1); (foo_name, foo_contents1)]
  in

  (* Introduce an irrecoverable parsing error in Foo definition - if we would
   * proceed to try to redeclare foo.php, Foo will no longer exist *)
  let env = Test.Client.setup_disk env [(foo_name, foo_contents2)] in
  let (env, diagnostics) = Test.Client.open_file env foo_name in
  Test.Client.assert_diagnostics_string diagnostics foo_diagnostics;

  (* Change the Foo parent to trigger invalidating Foo *)
  let env =
    Test.Client.setup_disk env [(foo_parent_name, foo_parent_contents2)]
  in
  (* If Foo is irrecoverable, then it shouldn't be available for completion *)
  let (env, _diagnostics) =
    Test.Client.edit_file env "test.php" autocomplete_contents
  in
  let (env, response) =
    ClientIdeDaemon.Test.handle
      env
      ClientIdeMessage.(
        Completion
          ( Test.doc "test.php" autocomplete_contents,
            Test.loc 4 9,
            { is_manually_invoked = true } ))
  in
  Test.assert_ide_completions response [];

  ignore env;
  ()
