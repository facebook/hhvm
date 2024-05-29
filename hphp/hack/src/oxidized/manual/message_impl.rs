// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use rc_pos::with_erased_lines::WithErasedLines;

use crate::message::Message;

impl<P: WithErasedLines> Message<P> {
    pub fn with_erased_lines(self) -> Self {
        let Message(pos, m) = self;
        Message(pos.with_erased_lines(), m)
    }
}
