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
  let root = "/" in
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set (Filename.concat root ".hhconfig") "";
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) = ServerConfig.load ~silent:false options in
  let env = Test.setup_server ~custom_config () in
  let env = Test.connect_persistent_client env in

  (* Two bar files use `foo` which is still unbound. *)
  let env =
    Test.open_file
      env
      "bar_expects_int.php"
      ~contents:"<?hh\nfunction bar_int(): int { return foo(); }\n"
  in
  let env =
    Test.open_file
      env
      "bar_expects_string.php"
      ~contents:"<?hh\nfunction bar_string(): string { return foo(); }\n"
  in
  let env = Test.wait env in
  let (env, loop_outputs) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics
    loop_outputs
    (SMap.of_list
       [
         ( "/bar_expects_int.php",
           SSet.of_list
             [
               "File \"/bar_expects_int.php\", line 2, characters 34-36:\nUnbound name: `foo` (a global function) (Naming[2049])";
               "File \"/bar_expects_int.php\", line 2, characters 34-36:\nUnbound name (typing): `foo` (Typing[4107])";
             ] );
         ( "/bar_expects_string.php",
           SSet.of_list
             [
               "File \"/bar_expects_string.php\", line 2, characters 40-42:\nUnbound name: `foo` (a global function) (Naming[2049])";
               "File \"/bar_expects_string.php\", line 2, characters 40-42:\nUnbound name (typing): `foo` (Typing[4107])";
             ] );
       ]);

  (* Now create `foo` in foo_string.php which returns string. *)
  let env =
    Test.open_file
      env
      "foo_returns_string_name.php"
      ~contents:"<?hh\nfunction foo() : string { return \"\"; }\n"
  in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_string
    loop_output
    "
/bar_expects_int.php:
File \"/bar_expects_int.php\", line 2, characters 34-38:\nInvalid return type (Typing[4110])
  File \"/bar_expects_int.php\", line 2, characters 21-23:\n  Expected `int`
  File \"/foo_returns_string_name.php\", line 2, characters 18-23:\n  But got `string`

/bar_expects_string.php:
";

  (* Create another `foo` in foo_int.php which returns int. *)
  let env =
    Test.open_file
      env
      "foo_returns_int.php"
      ~contents:"<?hh\nfunction foo() : int { return 1; }\n"
  in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_in
    loop_output
    ~filename:"foo_returns_int.php"
    "
/foo_returns_int.php:
File \"/foo_returns_int.php\", line 2, characters 10-12:\nName already bound: `foo` (Naming[2012])
  File \"/foo_returns_string_name.php\", line 2, characters 10-12:\n  Previous definition is here
";

  (* Erase foo_string.php. *)
  let env = Test.open_file env "foo_returns_string_name.php" ~contents:"" in
  let env = Test.wait env in
  let (_, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_string
    loop_output
    "
/bar_expects_int.php:
/bar_expects_string.php:
File \"/bar_expects_string.php\", line 2, characters 40-44:\nInvalid return type (Typing[4110])
  File \"/bar_expects_string.php\", line 2, characters 24-29:\n  Expected `string`
  File \"/foo_returns_int.php\", line 2, characters 18-20:\n  But got `int`

/foo_returns_int.php:
"
