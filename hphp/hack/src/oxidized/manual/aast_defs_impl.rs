// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast_defs::*;

impl Lid {
    pub fn new(p: Pos, s: String) -> Self {
        Self(p, (0, s))
    }
}
