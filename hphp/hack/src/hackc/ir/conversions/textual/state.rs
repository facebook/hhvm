// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ir::StringInterner;

pub(crate) struct UnitState {
    pub(crate) strings: Arc<StringInterner>,
    pub(crate) experimental_self_parent_in_trait: bool,
}

impl UnitState {
    pub(crate) fn new(
        strings: Arc<StringInterner>,
        experimental_self_parent_in_trait: bool,
    ) -> Self {
        Self {
            strings,
            experimental_self_parent_in_trait,
        }
    }
}
