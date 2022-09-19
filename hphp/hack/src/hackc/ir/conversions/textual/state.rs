// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::HashSet;

pub(crate) struct UnitState<'a> {
    pub(crate) external_funcs: HashSet<String>,
    pub(crate) unit: &'a ir::Unit<'a>,
}

impl<'a> UnitState<'a> {
    pub(crate) fn new(unit: &'a ir::Unit<'a>) -> Self {
        Self {
            external_funcs: Default::default(),
            unit,
        }
    }
}
