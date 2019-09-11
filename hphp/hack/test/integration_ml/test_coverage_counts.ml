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
function h($b): int {
  $v = vec[1];
  $a = $v[0];
  return $a + $b;
}
function f(): int {
  return h(1) + 2;
}
"

let foo_name = "foo.php"

let bar_contents =
  "<?hh

        function f(): int {
          return 5
        }


"

let bar_name = "bar.php"

let test () =
  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes = [(foo_name, foo_contents); (bar_name, bar_contents)];
        })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";

  let (_, loop_output) = Test.coverage_counts env "/foo.php" in
  Test.assert_coverage_counts
    loop_output
    [
      "/foo.php( array_get< checked=1 partial=0 unchecked=0 >"
      ^ " call< checked=1 partial=0 unchecked=0 >"
      ^ " lvar< checked=4 partial=0 unchecked=1 > )";
    ]
