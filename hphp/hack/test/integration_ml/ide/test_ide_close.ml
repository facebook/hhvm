(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_prelude
module Test = Integration_test_base

let foo_name = "foo.php"

let foo_disk_contents =
  "<?hh // strict

function foo(string $x) : void {}

function test() : void {
  foo(4);
}
"

let foo_disk_diagnostics =
  "
/foo.php:
File \"/foo.php\", line 6, characters 7-7:
Invalid argument (Typing[4110])
  File \"/foo.php\", line 3, characters 14-19:
  Expected `string`
  File \"/foo.php\", line 6, characters 7-7:
  But got `int`
"

let foo_ide_contents = "<?hh

{
"

let foo_ide_diagnostics =
  "
/foo.php:
File \"/foo.php\", line 3, characters 2-2:
A right brace `}` is expected here. (Parsing[1002])
"

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env = Test.Client.setup_disk env [(foo_name, foo_disk_contents)] in
  let (env, diagnostics) = Test.Client.open_file env foo_name in
  Test.Client.assert_diagnostics_string diagnostics foo_disk_diagnostics;

  let (env, diagnostics) =
    Test.Client.edit_file env foo_name foo_ide_contents
  in
  Test.Client.assert_diagnostics_string diagnostics foo_ide_diagnostics;

  (* Close the file and check that error lists are back to reflecting disk contents *)
  let (env, diagnostics) = Test.Client.close_file env foo_name in
  Test.Client.assert_diagnostics_string diagnostics foo_disk_diagnostics;
  ignore env;
  ()
