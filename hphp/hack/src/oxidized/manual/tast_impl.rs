// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::tast::FuncBodyAnn;
use crate::typing_defs_core::Reactivity;

impl Default for Reactivity {
    fn default() -> Self {
        Reactivity::Nonreactive
    }
}

impl Default for FuncBodyAnn {
    fn default() -> Self {
        FuncBodyAnn::NoUnsafeBlocks
    }
}
