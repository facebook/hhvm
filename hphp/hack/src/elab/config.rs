// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use oxidized::typechecker_options::TypecheckerOptions;

bitflags! {
    struct Flags: u8 {
        const SOFT_AS_LIKE = 1 << 0;
        const HKT_ENABLED = 1 << 1;
        const IS_HHI = 1 << 2;
        const IS_SYSTEMLIB = 1 << 3;
        const LIKE_TYPE_HINTS_ENABLED = 1 << 4;
        const CONST_ATTRIBUTE = 1 << 5;
    }
}

impl Flags {
    pub fn new(tco: &TypecheckerOptions, is_hhi: bool) -> Self {
        let mut flags: Self = Flags::empty();

        flags.set(Self::IS_HHI, is_hhi);
        flags.set(
            Self::SOFT_AS_LIKE,
            tco.po_interpret_soft_types_as_like_types,
        );
        flags.set(Self::HKT_ENABLED, tco.tco_higher_kinded_types);
        flags.set(Self::IS_SYSTEMLIB, tco.tco_is_systemlib);
        flags.set(Self::LIKE_TYPE_HINTS_ENABLED, tco.tco_like_type_hints);
        flags.set(Self::CONST_ATTRIBUTE, tco.tco_const_attribute);
        flags
    }
}

#[derive(Debug, Clone)]
pub struct Config {
    flags: Flags,
    pub consistent_ctor_level: isize,
}

impl Default for Config {
    fn default() -> Self {
        Self::new(&TypecheckerOptions::default(), false)
    }
}

impl Config {
    pub fn new(tco: &TypecheckerOptions, is_hhi: bool) -> Self {
        Self {
            flags: Flags::new(tco, is_hhi),
            consistent_ctor_level: tco.tco_explicit_consistent_constructors,
        }
    }

    pub fn soft_as_like(&self) -> bool {
        self.flags.contains(Flags::SOFT_AS_LIKE)
    }

    pub fn hkt_enabled(&self) -> bool {
        self.flags.contains(Flags::HKT_ENABLED)
    }

    pub fn is_systemlib(&self) -> bool {
        self.flags.contains(Flags::IS_SYSTEMLIB)
    }

    pub fn like_type_hints_enabled(&self) -> bool {
        self.flags.contains(Flags::LIKE_TYPE_HINTS_ENABLED)
    }

    pub fn is_hhi(&self) -> bool {
        self.flags.contains(Flags::IS_HHI)
    }

    pub fn const_attribute(&self) -> bool {
        self.flags.contains(Flags::CONST_ATTRIBUTE)
    }
}
