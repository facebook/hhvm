// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::saved_state_rollouts::SavedStateRollouts;

pub const DEFAULT_SAVED_STATE_ROLLOUTS: SavedStateRollouts = SavedStateRollouts {
    one: false,
    two: false,
    three: false,
};
