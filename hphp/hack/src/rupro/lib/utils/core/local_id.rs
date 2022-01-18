// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::pos::Symbol;

#[derive(Debug, Clone, Eq, Hash, PartialEq)]
pub struct LocalId(u64, Symbol);

impl LocalId {
    pub fn new_unscoped(x: Symbol) -> Self {
        Self(0, x)
    }

    pub fn to_int(&self) -> u64 {
        self.0
    }

    pub fn to_string(&self) -> &Symbol {
        &self.1
    }
}
