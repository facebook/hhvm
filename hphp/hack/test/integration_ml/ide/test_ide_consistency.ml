(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Integration_test_base_types
module Test = Integration_test_base

let foo_name = "foo.php"

let foo_contents = "<?hh
class C {
  public function foo() {

  }
}
"

let foo_contents_with_parse_error =
  "<?hh
class C {
  public function bar() {PARSE_ERROR

  }
}
"

let bar_name = "bar.php"

let bar_contents = "<?hh
function test() {
  (new C())->f
}
"

let run_and_check_autocomplete env expected_rechecked =
  (* Simulate time passing to trigger recheck *)
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  if loop_output.rechecked_count != expected_rechecked then
    Test.fail
      (Printf.sprintf "Expected %d files to be rechecked" expected_rechecked);

  let (env, loop_output) = Test.ide_autocomplete env (bar_name, 3, 15) in
  Test.assert_ide_autocomplete loop_output ["foo"];
  (env, loop_output)

let test () =
  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in
  (* Create and put content in two files *)
  let env = Test.open_file env foo_name ~contents:foo_contents in
  let env = Test.open_file env bar_name ~contents:bar_contents in
  (* Check that autocompletions in one file are aware of definitions in
   * another one*)
  let (env, _) = run_and_check_autocomplete env 2 in
  let (env, _) = Test.edit_file env foo_name foo_contents_with_parse_error in
  (* If C had parse errors, we'll not update it's declarations, so
   * the result will not change *)
  let _ = run_and_check_autocomplete env 1 in
  ()
