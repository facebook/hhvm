// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::convert::From;
use std::hash::Hash;
use std::ops::Deref;

use crate::hcons::Consed;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Symbol(Consed<String>);

impl Deref for Symbol {
    type Target = String;

    fn deref(&self) -> &String {
        &self.0
    }
}

impl From<Consed<String>> for Symbol {
    fn from(symbol: Consed<String>) -> Symbol {
        Symbol(symbol)
    }
}
