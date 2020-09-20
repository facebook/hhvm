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

let foo_contents =
  {|<?hh //strict
function foo(): int {
  /* HH_FIXME[4110] */
  return "4";
}
|}

let status_single_request =
  ServerCommandTypes.(STATUS_SINGLE (FileName "/foo.php", None))

let check_status_single_response = function
  | None -> Test.fail "Expected STATUS_SINGLE response"
  | Some ([], _) -> ()
  | Some _ -> Test.fail "Expected no errors"

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents = "
allowed_fixme_codes_strict = 4110
"

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let hhconfig_path =
    Relative_path.create Relative_path.Root hhconfig_filename
  in
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) =
    ServerConfig.load ~silent:false hhconfig_path options
  in
  let env = Test.setup_server ~custom_config () in
  let env = Test.setup_disk env [(foo_name, foo_contents)] in
  Test.assert_no_errors env;

  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          new_client = Some (RequestResponse status_single_request);
        })
  in
  check_status_single_response loop_output.new_client_response;
  ignore env;
  ()
