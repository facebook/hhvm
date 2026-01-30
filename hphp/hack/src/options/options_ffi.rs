// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use config_file::ConfigFile;
use hh_config::HhConfig;
use ocamlrep_ocamlpool::ocaml_ffi;
use oxidized::experimental_features::FeatureName;
use oxidized::experimental_features::FeatureStatus;
use oxidized::global_options::GlobalOptions;
use oxidized::saved_state_rollouts::Flag;
use oxidized::saved_state_rollouts::SavedStateRollouts;

ocaml_ffi! {
fn get_current_rollout_flag(current_rolled_out_flag_idx: usize) -> Option<String> {
    Flag::get_current_rollout_flag(current_rolled_out_flag_idx as isize)
        .map(|f| f.flag_name().to_owned())
}

fn make_saved_state_rollouts(
    current_rolled_out_flag_idx: usize,
    deactivate_saved_state_rollout: bool,
    force_flag_value: Option<String>,
    default: bool,
) -> SavedStateRollouts {
    let get_default = move |_flag_name: &str| Ok(default);
    SavedStateRollouts::make(
        current_rolled_out_flag_idx.try_into().unwrap(),
        deactivate_saved_state_rollout,
        force_flag_value.as_deref(),
        get_default,
    )
    .unwrap()
}

fn default_saved_state_rollouts() -> SavedStateRollouts {
    SavedStateRollouts::default()
}

fn get_feature_status_deprecated(name: FeatureName) -> FeatureStatus {
    name.get_feature_status_deprecated()
}

// Parse configuration files using Rust parser and return GlobalOptions.
// Used for testing equivalence with OCaml parser.
fn rust_parse_config_to_global_options(
    hhconfig_bytes: Vec<u8>,
    hh_conf_bytes: Vec<u8>,
) -> Result<GlobalOptions, String> {
    let hhconfig = ConfigFile::from_slice(&hhconfig_bytes);
    let hh_conf = ConfigFile::from_slice(&hh_conf_bytes);

    let hh_config = HhConfig::from_configs(
        std::path::PathBuf::new(),
        hhconfig,
        hh_conf,
        Default::default(),
    ).map_err(|e| format!("Rust config parse error: {}", e))?;

    Ok(hh_config.opts)
}
}
