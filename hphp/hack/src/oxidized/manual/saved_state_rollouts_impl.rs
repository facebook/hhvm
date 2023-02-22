// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::saved_state_rollouts::SavedStateRollouts;

impl Default for SavedStateRollouts {
    fn default() -> Self {
        Self {
            one: false,
            two: false,
            three: false,
        }
    }
}
