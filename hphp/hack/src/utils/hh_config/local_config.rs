// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Result;
use config_file::ConfigFile;
use oxidized::global_options::SavedState;
use oxidized::saved_state_rollouts::SavedStateRollouts;

/// A port of just enough of Server_local_config.t to support settings that are
/// copied into GlobalOptions.
#[derive(Debug, Default, Clone)]
pub struct LocalConfig {
    /// Allows unstable features to be enabled within a file via the
    /// '__EnableUnstableFeatures' attribute
    pub allow_unstable_features: bool,

    /// Use the Rust implementation of naming elaboration and NAST checks.
    pub rust_elab: bool,

    pub saved_state: SavedState,
}

impl LocalConfig {
    /// Construct from an hh.conf file with CLI overrides already applied.
    pub fn from_config(
        current_version: Option<&str>,
        current_rolled_out_flag_idx: isize,
        config: ConfigFile,
    ) -> Result<Self> {
        let mut lc = Self::default();
        lc.saved_state.rollouts = SavedStateRollouts::make(
            current_rolled_out_flag_idx,
            config.get_str("ss_force"),
            |flag_name| config.get_bool(flag_name).unwrap_or(Ok(false)),
        )?;
        if let Some(b) = config.get_bool("project_metadata_w_flags") {
            lc.saved_state.project_metadata_w_flags = b?;
        }
        if let Some(b) = config.bool_if_min_version("allow_unstable_features", current_version) {
            lc.allow_unstable_features = b?;
        }
        if let Some(b) = config.bool_if_min_version("rust_elab", current_version) {
            lc.rust_elab = b?;
        }
        Ok(lc)
    }
}
