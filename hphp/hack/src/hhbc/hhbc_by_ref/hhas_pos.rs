// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::pos::Pos;

/// Span, emitted as prefix to classes and functions
#[derive(Clone, Copy, Debug, Default)]
pub struct Span(pub usize, pub usize);

impl Span {
    pub fn from_pos(pos: &Pos) -> Span {
        let (line_begin, line_end, _, _) = pos.info_pos_extended();
        Span(line_begin, line_end)
    }
}
