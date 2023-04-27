(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open ServerEnv
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

let foo_disk_errors =
  "
File \"/foo.php\", line 6, characters 7-7:
Invalid argument (Typing[4110])
  File \"/foo.php\", line 3, characters 14-19:
  Expected `string`
  File \"/foo.php\", line 6, characters 7-7:
  But got `int`
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

let foo_ide_errors =
  [
    "
File \"/foo.php\", line 3, characters 1-1:
Hack does not support top level statements. Use the `__EntryPoint` attribute on a function instead (Parsing[1002])
";
    "
File \"/foo.php\", line 3, characters 2-2:
A right brace `}` is expected here. (Parsing[1002])
";
  ]

let foo_ide_diagnostics =
  "
/foo.php:
File \"/foo.php\", line 3, characters 1-1:
Hack does not support top level statements. Use the `__EntryPoint` attribute on a function instead (Parsing[1002])

File \"/foo.php\", line 3, characters 2-2:
A right brace `}` is expected here. (Parsing[1002])
"

let test () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env [(foo_name, foo_disk_contents)] in
  let env = Test.connect_persistent_client env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assertSingleError foo_disk_errors (Errors.get_error_list env.errorl);
  Test.assert_diagnostics_string loop_output foo_disk_diagnostics;

  let env = Test.open_file env foo_name in
  let (env, _) = Test.edit_file env foo_name foo_ide_contents in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  (* Force update to global error list *)
  let (env, _) = Test.status env in
  begin
    match List.zip foo_ide_errors (Errors.get_error_list env.errorl) with
    | List.Or_unequal_lengths.Ok errs ->
      List.iter
        ~f:(fun (expected, err) -> Test.assertSingleError expected [err])
        errs
    | List.Or_unequal_lengths.Unequal_lengths ->
      Test.fail "Expected 2 errors.\n"
  end;
  Test.assert_diagnostics_string loop_output foo_ide_diagnostics;

  (* Close the file and check that error lists are back to reflecting disk
   * contents *)
  let (env, _) = Test.close_file env foo_name in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  let (env, _) = Test.status env in
  Test.assertSingleError foo_disk_errors (Errors.get_error_list env.errorl);
  Test.assert_diagnostics_string loop_output foo_disk_diagnostics
