(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Integration_test_base_types
module Test = Integration_test_base

let foo_contents =
  "<?hh
function g(string $x) : int  {
  /* HH_FIXME[4110] purposeful bad type */
  return $x;
}
"

let foo_name = "foo.php"

let test () =
  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [(foo_name, foo_contents)] })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";

  (* what string goes after env? *)
  let (_, loop_output) = Test.coverage_levels env "/foo.php" in
  Test.assert_coverage_levels
    loop_output
    [
      "checked: 0";
      "partial: 2";
      "unchecked: 0";
      "File \"/foo.php\", line 4, characters 10-11: partial";
    ]
