// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Clone, Copy, Debug, Hash, PartialEq, Eq, PartialOrd, Ord)]
pub struct Dep(u64);

impl Dep {
    pub fn new(x: u64) -> Self {
        Dep(x)
    }

    pub fn is_class(self) -> bool {
        self.0 & (1 << 62) != 0
    }

    pub fn class_to_extends(self) -> Option<Self> {
        if !self.is_class() {
            None
        } else {
            Some(Dep(self.0 & ((1 << 62) - 1)))
        }
    }
}

impl Into<u64> for Dep {
    fn into(self) -> u64 {
        self.0
    }
}
