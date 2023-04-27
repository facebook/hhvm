// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::file_pos_large::FilePosLarge;

#[derive(Copy, Clone, Eq, PartialEq)]
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
