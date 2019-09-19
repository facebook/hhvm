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

let foo_contents =
  Printf.sprintf
    "<?hh // strict
%s
/* HH_FIXME[4336] */
function foo(): string {
}"

let bar_name = Printf.sprintf "bar%d.php"

let bar_contents =
  Printf.sprintf "<?hh

function bar%d (): int {
  return foo();
}
"

let create_bar i = (bar_name i, bar_contents i)

let bar_106_name = bar_name 106

let bar_107_name = bar_name 107

let rec create_bars acc = function
  | 0 -> acc
  | i -> create_bars (create_bar i :: acc) (i - 1)

let assert_10_diagnostics loop_output =
  if SMap.cardinal (Test.get_diagnostics loop_output) <> 10 then
    Test.fail "Expected push diagnostics for 10 files"

let bar_107_foo_line_3_diagnostics =
  {|
/bar107.php:
File "/bar107.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
File "/bar107.php", line 3, characters 21-23:
Expected int
File "/foo.php", line 4, characters 17-22:
But got string
|}

let bar_107_foo_line_5_diagnostics =
  {|
/bar107.php:
File "/bar107.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
File "/bar107.php", line 3, characters 21-23:
Expected int
File "/foo.php", line 6, characters 17-22:
But got string
|}

let bar106_cleared = {|
/bar106.php:
|}

let bar107_cleared = {|
/bar107.php:
|}

let bar_108_foo_line_5_diagnostics =
  {|
/bar108.php:
File "/bar108.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
File "/bar108.php", line 3, characters 21-23:
Expected int
File "/foo.php", line 6, characters 17-22:
But got string
|}

let bar_109_foo_line_3_diagnostics =
  {|
/bar109.php:
File "/bar109.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
File "/bar109.php", line 3, characters 21-23:
Expected int
File "/foo.php", line 4, characters 17-22:
But got string
|}

let test () =
  let env = Test.setup_server () in
  (* 200 files with errors *)
  let disk_contents = [(foo_name, foo_contents "")] in
  let disk_contents = create_bars disk_contents 200 in
  let env = Test.setup_disk env disk_contents in
  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  (* At the beggining we get errorrs for 10 of them, and foo() is on line 3 *)
  assert_10_diagnostics loop_output;
  Test.assert_diagnostics_in
    loop_output
    bar_107_name
    bar_107_foo_line_3_diagnostics;

  (* Move foo 2 lines down *)
  let env = Test.open_file env foo_name ~contents:(foo_contents "\n\n") in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  (* See all already received diagnostics updated *)
  assert_10_diagnostics loop_output;
  Test.assert_diagnostics_in
    loop_output
    bar_107_name
    bar_107_foo_line_5_diagnostics;

  (* Fix one of the errors *)
  let (env, _) = Test.edit_file env bar_107_name "" in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output bar107_cleared;

  (* Trigger next full recheck to get more global errors *)
  let (env, loop_output) = Test.full_check env in
  (* Notice that foo position is correct on line 5 *)
  Test.assert_diagnostics loop_output bar_108_foo_line_5_diagnostics;

  (* Edit foo back to test "edit already opened file" scenario too. *)
  let (env, _) = Test.edit_file env foo_name (foo_contents "") in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  assert_10_diagnostics loop_output;

  let (env, _) = Test.edit_file env bar_106_name "" in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output bar106_cleared;
  let (env, loop_output) = Test.full_check env in
  (* Notice that foo position is back to line 3 *)
  Test.assert_diagnostics loop_output bar_109_foo_line_3_diagnostics;
  ignore env
