// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_ocamlpool::ocaml_ffi;
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
}
