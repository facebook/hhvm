// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::ident::Ident;

pub trait New {
    fn new() -> Self;
}

impl New for Ident {
    fn new() -> Self {
        // TODO(hrust)
        0
    }
}
