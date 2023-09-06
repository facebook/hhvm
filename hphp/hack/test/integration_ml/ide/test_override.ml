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
`C` has no parent class with a method `foo` to override (Typing[4087])
"

let c_diagnostics =
  "
/C.php:
File \"/C.php\", line 5, characters 19-21:
`C` has no parent class with a method `foo` to override (Typing[4087])
"

let c_clear_diagnostics = "
/C.php:
"

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  let env =
    Test.Client.setup_disk
      env
      [
        (a_name, a_with_foo_contents); (b_name, b_contents); (c_name, c_contents);
      ]
  in

  (* c has no errors *)
  let (env, diagnostics) = Test.Client.open_file env c_name in
  Test.Client.assert_no_diagnostics diagnostics;

  (* when we edit a to cause downstream errors,
     then c will only discover the errors after b has been touched *)
  let env = Test.Client.setup_disk env [(a_name, a_without_foo_contents)] in

  (* TODO(ljw): this produces Decl_elems_bug
     let (env, diagnostics) = Test.Client.open_file env c_name in
     Test.Client.assert_no_diagnostics diagnostics;
     let env = Test.Client.setup_disk env [(b_name, b_contents)] in
     let (env, diagnostics) = Test.Client.open_file env c_name in
     Test.Client.assert_diagnostics_string diagnostics c_diagnostics;
  *)

  (* edit a to fix the downstream errors *)
  let env = Test.Client.setup_disk env [(a_name, a_with_foo_contents)] in
  let (env, _diagnostics) = Test.Client.open_file env c_name in
  (* TODO(ljw): removed due to the above Decl_elems_bug. Test.Client.assert_diagnostics_string diagnostics c_diagnostics; *)
  let env = Test.Client.setup_disk env [(b_name, b_contents)] in
  let (env, diagnostics) = Test.Client.open_file env c_name in
  Test.Client.assert_no_diagnostics diagnostics;

  ignore env;
  ()
