// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast_defs::*;
use crate::pos::Pos;
use std::boxed::Box;

impl Lid {
    pub fn new(p: Pos, s: String) -> Self {
        Self(p, (0, s))
    }

    pub fn from_counter(p: Pos, counter: isize, s: &str) -> Self {
        Self(p, (counter, String::from(s)))
    }
}

impl Hint {
    pub fn new(p: Pos, h: Hint_) -> Self {
        Self(p, Box::new(h))
    }
}
