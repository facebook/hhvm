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

let foo1_name = "foo1.php"

let foo2_name = "foo2.php"

let foo_contents = "<?hh // strict

function foo() : void {}
"

let bar_contents = "<?hh // strict
function test(): void {
  foo();
}
"

let init_diagnostics =
  {|
/foo2.php:
File "/foo2.php", line 3, characters 10-12:
Name already bound: `foo` (Naming[2012])
  File "/foo1.php", line 3, characters 10-12:
  Previous definition is here
|}

let final_diagnostics = {|
/foo2.php:
|}

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env =
    Test.Client.setup_disk
      env
      [
        (foo1_name, foo_contents);
        (foo2_name, foo_contents);
        ("bar.php", bar_contents);
      ]
  in

  let (env, diagnostics) = Test.Client.open_file env foo2_name in
  Test.Client.assert_diagnostics_string diagnostics init_diagnostics;

  (* Replace one of the duplicate definitions with a parsing error *)
  let env = Test.Client.setup_disk env [(foo2_name, "<?hh // strict \n {")] in
  let (env, diagnostics) = Test.Client.open_file env foo2_name in
  Test.Client.assert_diagnostics_string
    diagnostics
    {|
/foo2.php:
File "/foo2.php", line 2, characters 2-2:
Hack does not support top level statements. Use the `__EntryPoint` attribute on a function instead (Parsing[1002])

File "/foo2.php", line 3, characters 1-1:
A right brace `}` is expected here. (Parsing[1002])
|};

  (* Remove the parsing error - there should be no errors after that *)
  let env = Test.Client.setup_disk env [(foo2_name, "<?hh // strict")] in
  let (env, diagnostics) = Test.Client.open_file env foo2_name in
  Test.Client.assert_no_diagnostics diagnostics;

  ignore env;
  ()
