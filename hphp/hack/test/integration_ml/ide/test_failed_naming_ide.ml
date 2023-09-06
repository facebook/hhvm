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

let foo1_name = "foo1.php"

let foo2_name = "foo2.php"

let foo_contents = "<?hh // strict

function foo() : void {}
"

let bar_contents = "<?hh // strict
function test(): void {
  foo();
}
"

let init_errors =
  {|
File "/foo2.php", line 3, characters 10-12:
Name already bound: `foo` (Naming[2012])
  File "/foo1.php", line 3, characters 10-12:
  Previous definition is here
|}

let init_diagnostics =
  {|
/foo2.php:
File "/foo2.php", line 3, characters 10-12:
Name already bound: `foo` (Naming[2012])
  File "/foo1.php", line 3, characters 10-12:
  Previous definition is here
|}

let final_diagnostics = {|
/foo2.php:
|}

let test () =
  let env = Test.setup_server () in
  let env =
    Test.setup_disk
      env
      [
        (foo1_name, foo_contents);
        (foo2_name, foo_contents);
        ("bar.php", bar_contents);
      ]
  in
  Test.assert_env_errors env init_errors;

  let env = Test.connect_persistent_client env in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_string loop_output init_diagnostics;

  (* Replace one of the duplicate definitions with a parsing error *)
  let env = Test.open_file env foo2_name ~contents:"<?hh // strict \n {" in
  (* Trigger a lazy recheck *)
  let env = Test.wait env in
  let (env, _) = Test.(run_loop_once env default_loop_input) in
  (* Files with parsing errors are not redeclared during lazy recheck, so
   * failed_naming should remain unchanged *)
  let failed = Errors.get_failed_files env.ServerEnv.errorl in
  let found =
    Relative_path.Set.mem failed (Relative_path.from_root ~suffix:foo1_name)
    || Relative_path.Set.mem failed (Relative_path.from_root ~suffix:foo2_name)
  in
  if not found then Test.fail "File missing from failed";

  (* Remove the parsing error - there should be no errors after that *)
  let (env, _) = Test.edit_file env foo2_name "<?hh // strict" in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_string loop_output final_diagnostics;

  (* Trigger a global recheck just to be sure *)
  let (env, _) = Test.status env in
  Test.assert_no_errors env
