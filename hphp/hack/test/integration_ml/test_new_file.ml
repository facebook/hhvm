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

function f(): int {
  return 3
}
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
      "ERROR: File \"/foo.php\", line 4, characters 11-11:\n"
      ^ "A semicolon `;` is expected here. (Parsing[1002])";
    ]
  in
  match
    List.zip expected_errors (Diagnostics.get_diagnostic_list env.diagnostics)
  with
  | List.Or_unequal_lengths.Ok errs ->
    List.iter
      ~f:(fun (expected, err) -> Test.assertSingleDiagnostic expected [err])
      errs
  | List.Or_unequal_lengths.Unequal_lengths -> Test.fail "Expected 1 error."
