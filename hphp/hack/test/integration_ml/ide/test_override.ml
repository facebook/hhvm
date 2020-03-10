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

let a_name = "A.php"

let a_with_foo_contents =
  "<?hh // strict

interface A {
  public function foo(): void;
}
"

let a_without_foo_contents = "<?hh // strict

interface A {
}
"

let b_name = "B.php"

let b_contents = "<?hh // strict
abstract class B implements A {}
"

let c_name = "C.php"

let c_contents =
  "<?hh // strict

class C extends B {
  <<__Override>>
  public function foo(): void {}
}
"

let c_errors =
  "
File \"/C.php\", line 5, characters 19-21:
C has no parent class with a method `foo` to override (Typing[4087])
"

let c_diagnostics =
  "
/C.php:
File \"/C.php\", line 5, characters 19-21:
C has no parent class with a method `foo` to override (Typing[4087])
"

let c_clear_diagnostics = "
/C.php:
"

let test () =
  let env = Test.setup_server () in
  let env =
    Test.setup_disk
      env
      [
        (a_name, a_with_foo_contents); (b_name, b_contents); (c_name, c_contents);
      ]
  in
  Test.assert_no_errors env;

  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env in
  let env = Test.open_file env a_name ~contents:a_without_foo_contents in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_diagnostics loop_output;

  let (env, loop_output) = Test.full_check env in
  Test.assert_env_errors env c_errors;

  Test.assert_diagnostics loop_output c_diagnostics;

  let (env, _) = Test.edit_file env a_name a_with_foo_contents in
  let env = Test.wait env in
  let (_, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_output c_clear_diagnostics
