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

let foo_contents = "<?hh // strict

function foo() : void {}
"

let bar_name = "bar.php"

let bar_contents = "<?hh // strict

function test(): void {
  foo();
}
"

let full_diagnostics =
  "
/bar.php:
File \"/bar.php\", line 4, characters 3-5:
Unbound name: `foo` (a global function) (Naming[2049])

File \"/bar.php\", line 4, characters 3-5:
Unbound name (typing): `foo` (Typing[4107])
"

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env =
    Test.Client.setup_disk
      env
      [(foo_name, foo_contents); (bar_name, bar_contents) (* no errors *)]
  in

  (* no diagnostics initially *)
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_no_diagnostics diagnostics;

  (* Delete foo() *)
  let env = Test.Client.setup_disk env [(foo_name, "")] in
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_diagnostics_string diagnostics full_diagnostics;

  ignore env;
  ()
