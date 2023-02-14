// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bitflags::bitflags;

bitflags! {
    #[derive(Default)]
    pub struct Config: u8 {
        const SOFT_AS_LIKE= 1 << 0;
        const HKT_ENABLED = 1 << 1;
        const IS_HHI = 1 << 2;
        const IS_SYSTEMLIB= 1 << 3;
        const LIKE_TYPE_HINTS_ENABLED = 1 << 4;
    }
}

impl Config {
    pub fn soft_as_like(&self) -> bool {
        self.contains(Self::SOFT_AS_LIKE)
    }

    pub fn hkt_enabled(&self) -> bool {
        self.contains(Self::HKT_ENABLED)
    }

    pub fn is_hhi(&self) -> bool {
        self.contains(Self::IS_HHI)
    }

    pub fn is_systemlib(&self) -> bool {
        self.contains(Self::IS_SYSTEMLIB)
    }

    pub fn like_type_hints_enabled(&self) -> bool {
        self.contains(Self::LIKE_TYPE_HINTS_ENABLED)
    }
}
