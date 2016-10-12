(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

let foo_contents_with_parse_error = "<?hh
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

let run_and_check_autocomplete env =
  (* Simulate time passing to trigger recheck *)
  let env = Test.wait env in

  let env, loop_output = Test.ide_autocomplete env (bar_name, 3, 15) in
  Test.assert_autocomplete loop_output ["foo"];
  env, loop_output

let () =

  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in

  (* Create and put content in two files *)
  let env = Test.open_file env foo_name ~contents:foo_contents in
  let env = Test.open_file env bar_name ~contents:bar_contents in

  (* Check that autocompletions in one file are aware of definitions in
   * another one*)
  let env, loop_output = run_and_check_autocomplete env in
  if loop_output.rechecked_count != 2 then
    Test.fail "Expected 2 files to be rechecked";

  let env, _ = Test.edit_file env foo_name foo_contents_with_parse_error in
  (* If C had parse errors, we'll not update it's declarations, so
   * the result will not change *)
  let _, loop_output = run_and_check_autocomplete env in
  if loop_output.rechecked_count != 1 then
    Test.fail "Expected 1 file to be rechecked";
