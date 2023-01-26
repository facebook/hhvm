(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Test = Integration_test_base

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let file_name = "f.php"

let hhi_name = "g.hhi"

let file_contents =
  "<?hh // strict

function f(): string {
  /* HH_FIXME[4110] */
  return 4;
}
"

let hhi_contents = "<?hh

/* HH_FIXME[4110] */
const string C = 4;
"

let hhconfig_contents = ""

let errors =
  {|
File "/f.php", line 5, characters 10-10:
Invalid return type (Typing[4110])
  File "/f.php", line 3, characters 15-20:
  Expected `string`
  File "/f.php", line 5, characters 10-10:
  But got `int`

File "/f.php", line 4, characters 3-22:
You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error 4110 (Typing[4110])
|}

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let options = ServerArgs.default_options ~root in
  let (config, _) = ServerConfig.load ~silent:false options in
  let env =
    Test.setup_server
      ~custom_config:config
      ~hhi_files:[(hhi_name, hhi_contents)]
      ()
  in
  let env = Test.setup_disk env [(file_name, file_contents)] in
  Test.assert_env_errors env errors
