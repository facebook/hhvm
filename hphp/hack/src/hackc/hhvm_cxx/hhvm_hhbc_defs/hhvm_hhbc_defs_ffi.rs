// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::{BitAnd, BitOr, BitOrAssign};

#[allow(unreachable_patterns)]
#[cxx::bridge(namespace = "HPHP")]
pub mod ffi {
    // This is not a real definition. Cxx just adds static asserts on each of
    // these enum variants that they match the definition in
    // fcall-args-flags.h.
    #[repr(u16)]
    #[derive(Debug, Copy, Clone)]
    enum FCallArgsFlags {
        // This maps to a non-class enum in C++ so the variant names are chosen
        // to avoid naming collisions.
        FCANone = 0x0,
        HasUnpack = 0x1,
        HasGenerics = 0x2,
        LockWhileUnwinding = 0x4,
        SkipRepack = 0x8,
        SkipCoeffectsCheck = 0x10,
        EnforceMutableReturn = 0x20,
        EnforceReadonlyThis = 0x40,
        ExplicitContext = 0x80,
        HasInOut = 0x100,
        EnforceInOut = 0x200,
        EnforceReadonly = 0x400,
        HasAsyncEagerOffset = 0x800,
        NumArgsStart = 0x1000,
    }

    unsafe extern "C++" {
        include!("hphp/hack/src/hackc/hhvm_cxx/hhvm_hhbc_defs/as-hhbc-ffi.h");
        type FCallArgsFlags;
        fn fcall_flags_to_string_ffi(flags: FCallArgsFlags) -> String;
    }
}

use ffi::FCallArgsFlags;

impl FCallArgsFlags {
    pub fn add(&mut self, flag: Self) {
        self.repr = *self | flag
    }

    pub fn set(&mut self, flag: Self, b: bool) {
        if b {
            self.add(flag)
        }
    }

    pub fn contains(&self, flag: Self) -> bool {
        (*self & flag) != 0
    }
}

impl BitOr for FCallArgsFlags {
    type Output = u16;

    fn bitor(self, other: Self) -> u16 {
        self.repr | other.repr
    }
}

impl BitOrAssign for FCallArgsFlags {
    fn bitor_assign(&mut self, rhs: Self) {
        self.repr |= rhs.repr;
    }
}

impl BitAnd for FCallArgsFlags {
    type Output = u16;

    fn bitand(self, other: Self) -> u16 {
        self.repr & other.repr
    }
}

impl Default for FCallArgsFlags {
    fn default() -> Self {
        Self::FCANone
    }
}
