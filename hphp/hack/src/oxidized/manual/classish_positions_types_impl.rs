// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use rc_pos::with_erased_lines::WithErasedLines;

use crate::classish_positions_types;

impl<P: WithErasedLines> WithErasedLines for classish_positions_types::Pos<P> {
    fn with_erased_lines(self) -> Self {
        match self {
            Self::ClassishClosingBrace(s) => Self::ClassishClosingBrace(s),
            Self::ClassishEndOfBody(s) => Self::ClassishEndOfBody(s),
            Self::ClassishStartOfBody(s) => Self::ClassishStartOfBody(s),
            Self::Precomputed(p) => Self::Precomputed(p.with_erased_lines()),
        }
    }
}
