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

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename "timeout = 737";
  let (config, local_config) =
    ServerConfig.load
      ~silent:false
      ~from:""
      ~cli_config_overrides:
        [("timeout", "747"); ("informant_min_distance_restart", "711")]
  in
  let timeout =
    TypecheckerOptions.timeout (ServerConfig.typechecker_options config)
  in
  if not (timeout = 747) then Test.fail "Global config value not overridden!";

  let informant_min_distance_restart =
    local_config.ServerLocalConfig.informant_min_distance_restart
  in
  if not (informant_min_distance_restart = 711) then
    Test.fail "Local config value not overridden!"
