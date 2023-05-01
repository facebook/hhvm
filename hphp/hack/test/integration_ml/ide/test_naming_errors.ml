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

let foo_returns_int_name = "foo_returns_int.php"

let foo_returns_string_name = "foo_returns_string_name.php"

let foo_contents =
  Printf.sprintf "<?hh // strict
/* HH_FIXME[4336] */
function foo() : %s {

}
"

let foo_returns_int_contents = foo_contents "int"

let foo_returns_string_contents = foo_contents "string"

let bar_expects_int_name = "bar_expects_int.php"

let bar_expects_string_name = "bar_expects_string.php"

let bar_contents x =
  Printf.sprintf
    "<?hh // strict

function bar_%s(): %s {
  return foo();
}
"
    x
    x

let foo_unbound_diagnostics =
  SMap.of_list
    [
      ( "/bar_expects_int.php",
        SSet.of_list
          [
            "File \"/bar_expects_int.php\", line 4, characters 10-12:
Unbound name: `foo` (a global function) (Naming[2049])";
            "File \"/bar_expects_int.php\", line 4, characters 10-12:
Unbound name (typing): `foo` (Typing[4107])";
          ] );
      ( "/bar_expects_string.php",
        SSet.of_list
          [
            "File \"/bar_expects_string.php\", line 4, characters 10-12:
Unbound name: `foo` (a global function) (Naming[2049])";
            "File \"/bar_expects_string.php\", line 4, characters 10-12:
Unbound name (typing): `foo` (Typing[4107])";
          ] );
    ]

let foo_returns_string_diagnostics =
  "
/bar_expects_int.php:
File \"/bar_expects_int.php\", line 4, characters 10-14:
Invalid return type (Typing[4110])
  File \"/bar_expects_int.php\", line 3, characters 21-23:
  Expected `int`
  File \"/foo_returns_string_name.php\", line 3, characters 18-23:
  But got `string`

/bar_expects_string.php:
"

let foo_duplicate_diagnostics =
  "
/foo_returns_int.php:
File \"/foo_returns_int.php\", line 3, characters 10-12:
Name already bound: `foo` (Naming[2012])
  File \"/foo_returns_string_name.php\", line 3, characters 10-12:
  Previous definition is here
"

let foo_returns_int_diagnostics =
  "
/bar_expects_int.php:
/bar_expects_string.php:
File \"/bar_expects_string.php\", line 4, characters 10-14:
Invalid return type (Typing[4110])
  File \"/bar_expects_string.php\", line 3, characters 24-29:
  Expected `string`
  File \"/foo_returns_int.php\", line 3, characters 18-20:
  But got `int`

/foo_returns_int.php:
"

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents =
  "
allowed_fixme_codes_strict = 4336
allowed_decl_fixme_codes = 4336
"

let load_config () =
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) = ServerConfig.load ~silent:false options in
  custom_config

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let custom_config = load_config () in
  let env = Test.setup_server ~custom_config () in
  let env = Test.connect_persistent_client env in

  (* Two bar files use `foo` which is still unbound. *)
  let env =
    Test.open_file env bar_expects_int_name ~contents:(bar_contents "int")
  in
  let env =
    Test.open_file env bar_expects_string_name ~contents:(bar_contents "string")
  in
  let env = Test.wait env in
  let (env, loop_outputs) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics loop_outputs foo_unbound_diagnostics;

  (* Now create `foo` in foo_string.php which returns string. *)
  let env =
    Test.open_file
      env
      foo_returns_string_name
      ~contents:foo_returns_string_contents
  in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_string loop_output foo_returns_string_diagnostics;

  (* Create another `foo` in foo_int.php which returns int. *)
  let env =
    Test.open_file env foo_returns_int_name ~contents:foo_returns_int_contents
  in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_in
    loop_output
    ~filename:foo_returns_int_name
    foo_duplicate_diagnostics;

  (* Erase foo_string.php. *)
  let env = Test.open_file env foo_returns_string_name ~contents:"" in
  let env = Test.wait env in
  let (_, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_diagnostics_string loop_output foo_returns_int_diagnostics
