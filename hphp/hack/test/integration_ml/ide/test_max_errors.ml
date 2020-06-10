(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Test = Integration_test_base

let foo_name = "foo.php"

let foo_contents =
  Printf.sprintf "<?hh
/* HH_FIXME[4336] */
function foo(): %s {

}
"

let foo_returns_int = foo_contents "int"

let foo_returns_string = foo_contents "string"

let bar_name = Printf.sprintf "bar%d.php"

let bar_contents =
  Printf.sprintf "<?hh

function bar%d (): int {
  return foo();
}
"

let baz_name = "baz.php"

let baz_contents = "<?hh

{
"

let create_bar i = (bar_name i, bar_contents i)

let rec create_bars acc = function
  | 0 -> acc
  | i -> create_bars (create_bar i :: acc) (i - 1)

let test () =
  let env = Test.setup_server () in
  let disk_contests = [(foo_name, foo_returns_int)] in
  let disk_contests = create_bars disk_contests 2 in
  let env = Test.setup_disk env disk_contests in
  let env = Test.connect_persistent_client env in
  let (env, _) = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_errors env;
  let (env, loop_output) = Test.status env in
  Test.assert_error_count loop_output ~expected_count:0;
  let (env, loop_output) = Test.status ~max_errors:(Some 2) env in
  Test.assert_error_count loop_output ~expected_count:0;

  (* Introduce a few errors *)
  let env = Test.open_file env foo_name ~contents:foo_returns_string in
  let env = Test.open_file env baz_name ~contents:baz_contents in
  let env = Test.wait env in
  let (env, _) = Test.full_check env in
  let (env, loop_output) = Test.status env in
  Test.assert_error_count loop_output ~expected_count:4;
  let (env, loop_output) = Test.status ~max_errors:(Some 2) env in
  Test.assert_error_count loop_output ~expected_count:2;
  let (env, loop_output) = Test.status env in
  Test.assert_error_count loop_output ~expected_count:4;
  ignore env
