// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::file_pos_large::FilePosLarge;
use crate::with_erased_lines::WithErasedLines;

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub struct PosSpanRaw {
    pub start: FilePosLarge,
    pub end: FilePosLarge,
}

const DUMMY: PosSpanRaw = PosSpanRaw {
    start: FilePosLarge::make_dummy(),
    end: FilePosLarge::make_dummy(),
};

impl PosSpanRaw {
    #[inline]
    pub const fn make_dummy() -> Self {
        DUMMY
    }

    #[inline]
    pub fn is_dummy(self) -> bool {
        self == DUMMY
    }
}

impl WithErasedLines for PosSpanRaw {
    fn with_erased_lines(self) -> Self {
        let Self { start, end } = self;
        let (start, end) = (start, end).with_erased_lines();
        Self { start, end }
    }
}
