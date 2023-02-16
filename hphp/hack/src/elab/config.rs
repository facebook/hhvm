// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bitflags::bitflags;

bitflags! {
    #[derive(Default)]
    pub struct Flags: u8 {
        const SOFT_AS_LIKE= 1 << 0;
        const HKT_ENABLED = 1 << 1;
        const IS_HHI = 1 << 2;
        const IS_SYSTEMLIB= 1 << 3;
        const LIKE_TYPE_HINTS_ENABLED = 1 << 4;
    }
}

#[derive(Default)]
pub struct Config {
    pub flags: Flags,
    pub consistent_ctor_level: isize,
}

impl Config {
    pub fn soft_as_like(&self) -> bool {
        self.flags.contains(Flags::SOFT_AS_LIKE)
    }

    pub fn hkt_enabled(&self) -> bool {
        self.flags.contains(Flags::HKT_ENABLED)
    }

    pub fn is_hhi(&self) -> bool {
        self.flags.contains(Flags::IS_HHI)
    }

    pub fn is_systemlib(&self) -> bool {
        self.flags.contains(Flags::IS_SYSTEMLIB)
    }

    pub fn like_type_hints_enabled(&self) -> bool {
        self.flags.contains(Flags::LIKE_TYPE_HINTS_ENABLED)
    }
}
