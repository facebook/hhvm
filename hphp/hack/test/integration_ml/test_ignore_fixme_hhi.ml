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

function f(): float {
  /* HH_FIXME[4110] */
  return 4;
}
"

let hhi_contents = "<?hh

/* HH_FIXME[4110] */
const float C = 4.0;
"

let hhconfig_contents = ""

let errors =
  {|
ERROR: File "/f.php", line 5, characters 10-10:
Invalid return type (Typing[4110])
  File "/f.php", line 3, characters 15-19:
  Expected `float`
  File "/f.php", line 5, characters 10-10:
  But got `int`

ERROR: File "/f.php", line 4, characters 3-22:
You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error 4110 (Typing[4110])
|}

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let (config, _) =
    ServerConfig.load ~silent:false ~from:"" ~cli_config_overrides:[]
  in
  let env =
    Test.setup_server
      ~custom_config:config
      ~hhi_files:[(hhi_name, hhi_contents)]
      ()
  in
  let env = Test.setup_disk env [(file_name, file_contents)] in
  Test.assert_env_diagnostics env errors
