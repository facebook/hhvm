// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::HashSet;
use ir::StringInterner;

pub(crate) struct UnitState {
    pub(crate) external_funcs: HashSet<String>,
    pub(crate) strings: StringInterner,
}

impl UnitState {
    pub(crate) fn new(strings: StringInterner) -> Self {
        Self {
            external_funcs: Default::default(),
            strings,
        }
    }
}
