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
open Hh_prelude
module Test = Integration_test_base

let foo_contents = "<?hh

{
"

let test () =
  let env = Test.setup_server () in
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [("foo.php", foo_contents)] })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  let expected_errors =
    [
      "File \"/foo.php\", line 3, characters 1-1:\n"
      ^ "Toplevel statements are not allowed. Use `__EntryPoint` attribute instead (Parsing[1002])";
      "File \"/foo.php\", line 3, characters 2-2:\n"
      ^ "A right brace `}` is expected here. (Parsing[1002])\n";
    ]
  in
  match List.zip expected_errors (Errors.get_error_list env.errorl) with
  | List.Or_unequal_lengths.Ok errs ->
    List.iter
      ~f:(fun (expected, err) -> Test.assertSingleError expected [err])
      errs
  | List.Or_unequal_lengths.Unequal_lengths -> Test.fail "Expected 2 errors."
