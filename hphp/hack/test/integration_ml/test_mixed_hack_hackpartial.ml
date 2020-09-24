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

let f_name = "f.hackpartial"

let g_name = "g.hack"

let f_contents =
  "
  function f1($m): int {}

  function f2(): string {
    /* HH_FIXME[4110] */
    return 4;
  }
"

let g_contents =
  "
  function g1(mixed $m): void {
    /* HH_FIXME[4063] */
    $m[3];
  }

  function g2(): string {
    /* HH_FIXME[4110] */
    return 4;
  }
"

let hhconfig_contents =
  "
allowed_fixme_codes_strict = 4063
allowed_fixme_codes_partial = 4110
codes_not_raised_partial = 4336
"

let errors =
  {|
File "/g.hack", line 9, characters 12-12:
Invalid return type (Typing[4110])
  File "/g.hack", line 7, characters 18-23:
  Expected `string`
  File "/g.hack", line 9, characters 12-12:
  But got `int`

File "/g.hack", line 8, characters 5-23:
You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error 4110 (Typing[4110])
|}

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let hhconfig_path =
    Relative_path.create Relative_path.Root hhconfig_filename
  in
  let options = ServerArgs.default_options ~root in
  let (config, _) = ServerConfig.load ~silent:false hhconfig_path options in
  let env = Test.setup_server ~custom_config:config () in
  let env = Test.setup_disk env [(f_name, f_contents); (g_name, g_contents)] in
  Test.assert_env_errors env errors
