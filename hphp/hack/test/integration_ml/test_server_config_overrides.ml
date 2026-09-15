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

let test_cli_overrides () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  Test_disk.set hhconfig_filename "timeout = 737";
  let (config, local_config) =
    Server_config.load
      ~silent:false
      ~from:""
      ~cli_config_overrides:
        [("timeout", "747"); ("informant_min_distance_restart", "711")]
  in
  let timeout =
    Typechecker_options.timeout (Server_config.typechecker_options config)
  in
  if not (timeout = 747) then Test.fail "Global config value not overridden!";

  let informant_min_distance_restart =
    local_config.Server_local_config.informant_min_distance_restart
  in
  if not (informant_min_distance_restart = 711) then
    Test.fail "Local config value not overridden!"

let run_override_pipeline overrides =
  let key = "test_override_precedence" in
  let get_value config =
    Config_file.Getters.string_ key ~default:"missing" config
  in
  let observed_inputs = ref [] in
  let apply source config =
    observed_inputs := (source, get_value config) :: !observed_inputs;
    Config_file.apply_overrides
      ~config
      ~overrides:(Config_file.of_list [(key, source)])
      ~log_reason:None
  in
  let apply_experiments_config_overrides config =
    ("gatekeeper metadata", apply "gatekeeper" config)
  in
  let (experiments_meta, config) =
    Server_local_config_load.For_test.apply_overrides_in_order
      ~silent:true
      ~config:(Config_file.of_list [(key, "hh.conf")])
      ~overrides
      ~apply_justknobs_overrides:(apply "justknobs")
      ~apply_dynamic_overrides:(apply "qe")
      ~apply_sandbox_experiment_overrides:(apply "sandbox_experiment")
      ~apply_experiments_config_overrides
  in
  (experiments_meta, get_value config, List.rev !observed_inputs)

let test_override_precedence () =
  let (experiments_meta, final_value, observed_inputs) =
    run_override_pipeline (Config_file.empty ())
  in
  let expected_inputs =
    [
      ("justknobs", "hh.conf");
      ("qe", "justknobs");
      ("sandbox_experiment", "qe");
      ("gatekeeper", "sandbox_experiment");
    ]
  in
  if observed_inputs <> expected_inputs then
    Test.fail "Configuration override sources were applied out of order";
  if not (String.equal experiments_meta "gatekeeper metadata") then
    Test.fail "Experiments metadata was not preserved";
  if not (String.equal final_value "gatekeeper") then
    Test.fail "GateKeeper override did not take precedence"

let test_cli_override_precedence () =
  let (experiments_meta, final_value, observed_inputs) =
    run_override_pipeline
      (Config_file.of_list [("test_override_precedence", "cli")])
  in
  let expected_inputs =
    [
      ("justknobs", "cli");
      ("qe", "justknobs");
      ("sandbox_experiment", "qe");
      ("gatekeeper", "sandbox_experiment");
    ]
  in
  if observed_inputs <> expected_inputs then
    Test.fail "CLI override was not visible before dynamic overrides";
  if not (String.equal experiments_meta "gatekeeper metadata") then
    Test.fail "Experiments metadata was not preserved";
  if not (String.equal final_value "cli") then
    Test.fail "CLI override did not take final precedence"

let test () =
  test_cli_overrides ();
  test_override_precedence ();
  test_cli_override_precedence ()
