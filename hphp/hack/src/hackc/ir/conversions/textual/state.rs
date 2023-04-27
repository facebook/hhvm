// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ir::StringInterner;

pub(crate) struct UnitState {
    pub(crate) strings: Arc<StringInterner>,
}

impl UnitState {
    pub(crate) fn new(strings: Arc<StringInterner>) -> Self {
        Self { strings }
    }
}
