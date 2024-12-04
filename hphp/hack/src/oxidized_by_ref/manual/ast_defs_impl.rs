// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ast_defs::*;
use crate::pos::Pos;

impl<'a> Id<'a> {
    pub fn pos(&self) -> &'a Pos<'_> {
        self.0
    }

    pub fn name(&self) -> &'a str {
        self.1
    }
}

impl std::fmt::Debug for Id<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Id({:?}, {:?})", self.pos(), self.name())
    }
}
