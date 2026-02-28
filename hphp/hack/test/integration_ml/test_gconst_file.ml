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
open ServerEnv
module Test = Integration_test_base

let foo_contents = "<?hh // strict
  const int A = 5;
"

let bar_contents = "<?hh // strict
  const int B = A;
"

let foo_new_contents = "<?hh // strict
  const string A = '';
"

let test () =
  let env =
    Test.setup_server
      ~hhi_files:(Hhi.get_raw_hhi_contents () |> Array.to_list)
      ()
  in
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [("foo.php", foo_contents)] })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  Test.assert_no_diagnostics env;

  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [("bar.php", bar_contents)] })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  Test.assert_no_diagnostics env;

  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes = [("foo.php", foo_new_contents)];
        })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";

  let expected_error =
    "ERROR: File \"/bar.php\", line 2, characters 9-11:\n"
    ^ "Wrong type hint (Typing[4110])\n"
    ^ "  File \"/bar.php\", line 2, characters 9-11:\n"
    ^ "  Expected `int`\n"
    ^ "  File \"/foo.php\", line 2, characters 9-14:\n"
    ^ "  But got `string`"
  in
  Test.assertSingleDiagnostic
    expected_error
    (Diagnostics.get_diagnostic_list env.diagnostics)
