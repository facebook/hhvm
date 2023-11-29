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

let foo_contents = "<?hh
{
"

let foo_diagnostics =
  "
/foo.php:
File \"/foo.php\", line 2, characters 2-2:
A right brace `}` is expected here. (Parsing[1002])"

let foo_clear_diagnostics = "
/foo.php:
"

let bar_name = "bar.php"

let bar_contents = "<?hh // strict
function test() {} // missing return type
"

let bar_diagnostics =
  "
/bar.php:
File \"/bar.php\", line 2, characters 10-13:
Was expecting a return type hint (Typing[4030])"

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  (* Initially bar.php has a single typing error on disk *)
  let env = Test.Client.setup_disk env [(bar_name, bar_contents)] in
  (* We'll find the error upon opening *)
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  (* Initial list of errors is pushed after subscribing *)
  Test.Client.assert_diagnostics_string diagnostics bar_diagnostics;

  (* Open and edit foo.php (different file) with errors *)
  let (env, diagnostics) = Test.Client.edit_file env foo_name foo_contents in
  Test.Client.assert_diagnostics_string diagnostics foo_diagnostics;

  (* Fix the errors in file foo.php *)
  let (env, diagnostics) = Test.Client.edit_file env foo_name "" in
  Test.Client.assert_diagnostics_string diagnostics foo_clear_diagnostics;

  (* Change foo.php, but still no new errors *)
  let (env, diagnostics) = Test.Client.edit_file env foo_name "<?hh\n" in
  Test.Client.assert_no_diagnostics diagnostics;

  ignore env;
  ()
