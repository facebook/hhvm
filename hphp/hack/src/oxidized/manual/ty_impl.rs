// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::{typing_defs::*, typing_reason::Reason};
use crate::pos::Pos;

// Helper methods on types
impl Ty {
    pub fn get_node(&self) -> &Ty_ {
        let Ty(_, t) = self;
        &*t
    }
    pub fn get_reason(&self) -> &Reason {
        let Ty(r, _) = self;
        r
    }
    pub fn get_pos(&self) -> &Pos {
        self.get_reason().get_pos()
    }
}
