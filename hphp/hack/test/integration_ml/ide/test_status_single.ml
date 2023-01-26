(*
 * Copyright (c) 2016, Facebook, Inc.
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

let bar_name = "bar.php"

let baz_name = "baz.php"

let foo_contents =
  {|<?hh //strict
function foo(): int {
  /* HH_FIXME[4110] */
  return "4";
}
|}

let bar_contents = {|<?hh
function bar(): int {
  return "4";
}
|}

let baz_contents = {|<?hh
function baz(): bool {
  return "4";
}
|}

let bar_error =
  {|
/bar.php:
File "/bar.php", line 3, characters 10-12:
Invalid return type (Typing[4110])
  File "/bar.php", line 2, characters 17-19:
  Expected `int`
  File "/bar.php", line 3, characters 10-12:
  But got `string`
|}

let baz_error =
  {|
/baz.php:
File "/baz.php", line 3, characters 10-12:
Invalid return type (Typing[4110])
  File "/baz.php", line 2, characters 17-20:
  Expected `bool`
  File "/baz.php", line 3, characters 10-12:
  But got `string`
|}

let status_single_request_foo =
  ServerCommandTypes.(
    STATUS_SINGLE { file_names = [FileName "/foo.php"]; max_errors = None })

let check_status_single_response_foo = function
  | None -> Test.fail "Expected STATUS_SINGLE response"
  | Some ([], _) -> ()
  | Some _ -> Test.fail "Expected no errors"

let status_single_request_bar_and_baz =
  ServerCommandTypes.(
    STATUS_SINGLE
      {
        file_names = [FileName "/bar.php"; FileName "/baz.php"];
        max_errors = None;
      })

let check_status_single_response_bar_and_baz loop_outputs =
  Test.assert_diagnostics_in loop_outputs ~filename:bar_name bar_error;
  Test.assert_diagnostics_in loop_outputs ~filename:baz_name baz_error

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents = "
allowed_fixme_codes_strict = 4110
"

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) = ServerConfig.load ~silent:false options in
  let env = Test.setup_server ~custom_config () in
  let env = Test.setup_disk env [(foo_name, foo_contents)] in
  let env = Test.connect_persistent_client env in
  Test.assert_no_errors env;

  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          new_client = Some (RequestResponse status_single_request_foo);
        })
  in
  check_status_single_response_foo loop_output.new_client_response;

  let env = Test.open_file env bar_name ~contents:bar_contents in
  let env = Test.open_file env baz_name ~contents:baz_contents in
  let env = Test.wait env in
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          new_client = Some (RequestResponse status_single_request_bar_and_baz);
        })
  in
  check_status_single_response_bar_and_baz loop_output;
  ignore env;

  ()
