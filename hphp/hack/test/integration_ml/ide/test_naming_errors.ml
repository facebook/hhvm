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

let test () =
  Test.Client.with_env ~custom_config:None @@ fun env ->
  (* Two bar files use `foo` which is still unbound. *)
  let env =
    Test.Client.setup_disk
      env
      [
        ( "bar_expects_int.php",
          "<?hh\nfunction bar_int(): int { return foo(); }\n" );
        ( "bar_expects_string.php",
          "<?hh\nfunction bar_string(): string { return foo(); }\n" );
      ]
  in
  let (env, diagnostics) = Test.Client.open_file env "bar_expects_int.php" in
  Test.Client.assert_diagnostics_string
    diagnostics
    {|
/bar_expects_int.php:
File "/bar_expects_int.php", line 2, characters 34-36:
Unbound name: `foo` (a global function) (Naming[2049])

File "/bar_expects_int.php", line 2, characters 34-36:
Unbound name (typing): `foo` (Typing[4107])
|};
  let (env, diagnostics) = Test.Client.open_file env "bar_expects_string.php" in
  Test.Client.assert_diagnostics_string
    diagnostics
    {|
/bar_expects_string.php:
File "/bar_expects_string.php", line 2, characters 40-42:
Unbound name: `foo` (a global function) (Naming[2049])

File "/bar_expects_string.php", line 2, characters 40-42:
Unbound name (typing): `foo` (Typing[4107])
|};

  (* Now create `foo` in foo_string.php which returns string. *)
  let env =
    Test.Client.setup_disk
      env
      [
        ( "foo_returns_string.php",
          "<?hh\nfunction foo() : string { return \"\"; }\n" );
      ]
  in
  let (env, diagnostics) = Test.Client.open_file env "bar_expects_int.php" in
  let bar_expects_int_but_got_string =
    {|
/bar_expects_int.php:
File "/bar_expects_int.php", line 2, characters 34-38:
Invalid return type (Typing[4110])
  File "/bar_expects_int.php", line 2, characters 21-23:
  Expected `int`
  File "/foo_returns_string.php", line 2, characters 18-23:
  But got `string`
|}
  in
  Test.Client.assert_diagnostics_string
    diagnostics
    bar_expects_int_but_got_string;
  let (env, diagnostics) = Test.Client.open_file env "bar_expects_string.php" in
  Test.Client.assert_no_diagnostics diagnostics;

  (* Create another `foo` in foo_returns_int.php which returns int.
     For some reason, the foo_returns_string definition is still considered the winner. *)
  let env =
    Test.Client.setup_disk
      env
      [("foo_returns_int.php", "<?hh\nfunction foo() : int { return 1; }\n")]
  in
  let (env, diagnostics) = Test.Client.open_file env "foo_returns_int.php" in
  Test.Client.assert_diagnostics_string diagnostics "/foo_returns_int.php:\n";
  let (env, diagnostics) = Test.Client.open_file env "foo_returns_string.php" in
  Test.Client.assert_diagnostics_string diagnostics "/foo_returns_string.php:\n";
  let (env, diagnostics) = Test.Client.open_file env "bar_expects_int.php" in
  Test.Client.assert_diagnostics_string
    diagnostics
    bar_expects_int_but_got_string;
  let (env, diagnostics) = Test.Client.open_file env "bar_expects_string.php" in
  Test.Client.assert_diagnostics_string diagnostics "/bar_expects_string.php:\n";

  (* Erase foo_returns_string.php. Now everyone agrees that foo_returns_int.php is the winner. *)
  let env = Test.Client.setup_disk env [("foo_returns_string.php", "")] in
  let (env, diagnostics) = Test.Client.open_file env "foo_returns_int.php" in
  Test.Client.assert_diagnostics_string diagnostics "/foo_returns_int.php:\n";
  let (env, diagnostics) = Test.Client.open_file env "foo_returns_string.php" in
  Test.Client.assert_diagnostics_string diagnostics "/foo_returns_string.php:\n";
  let (env, diagnostics) = Test.Client.open_file env "bar_expects_int.php" in
  Test.Client.assert_diagnostics_string diagnostics "/bar_expects_int.php:\n";
  let (env, diagnostics) = Test.Client.open_file env "bar_expects_string.php" in
  Test.Client.assert_diagnostics_string
    diagnostics
    {|
/bar_expects_string.php:
File "/bar_expects_string.php", line 2, characters 40-44:
Invalid return type (Typing[4110])
  File "/bar_expects_string.php", line 2, characters 24-29:
  Expected `string`
  File "/foo_returns_int.php", line 2, characters 18-20:
  But got `int`
|};

  ignore env;
  ()
