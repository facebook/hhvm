// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::BitAnd;
use std::ops::BitOr;
use std::ops::BitOrAssign;

#[allow(unreachable_patterns)]
#[cxx::bridge(namespace = "HPHP")]
pub mod ffi {
    // This is not a real definition. Cxx just adds static asserts on each of
    // these enum variants that they match the definition in
    // fcall-args-flags.h.
    #[repr(u16)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
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

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum IsTypeOp {
        Null,
        Bool,
        Int,
        Dbl,
        Str,
        Obj,
        Res,
        /// Int or Dbl or Str or Bool
        Scalar,
        Keyset,
        Dict,
        Vec,
        /// Arr or Vec or Dict or Keyset
        ArrLike,
        ClsMeth,
        Func,
        LegacyArrLike,
        Class,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum FatalOp {
        Runtime,
        Parse,
        RuntimeOmitFrame,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum InitPropOp {
        Static,
        NonStatic,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum SpecialClsRef {
        SelfCls,
        LateBoundCls,
        ParentCls,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum MOpMode {
        None,
        Warn,
        Define,
        Unset,
        InOut,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum QueryMOp {
        CGet,
        CGetQuiet,
        Isset,
        InOut,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum TypeStructResolveOp {
        Resolve,
        DontResolve,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum TypeStructEnforceKind {
        Deep,
        Shallow,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum AsTypeStructExceptionKind {
        Error,
        Typehint,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum IsLogAsDynamicCallOp {
        LogAsDynamicCall,
        DontLogAsDynamicCall,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum ReadonlyOp {
        Any,
        Readonly,
        Mutable,
        CheckROCOW,
        CheckMutROCOW,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum SetRangeOp {
        Forward,
        Reverse,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum ContCheckOp {
        IgnoreStarted,
        CheckStarted,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum SwitchKind {
        Unbounded,
        Bounded,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum ObjMethodOp {
        NullThrows,
        NullSafe,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum SilenceOp {
        Start,
        End,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum BareThisOp {
        Notice,
        NoNotice,
        NeverNull,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum IncDecOp {
        PreInc,
        PostInc,
        PreDec,
        PostDec,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum CollectionType {
        Vector = 0x12,
        Map = 0x13,
        Set = 0x14,
        Pair = 0x15,
        ImmVector = 0x16,
        ImmMap = 0x17,
        ImmSet = 0x18,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum SetOpOp {
        PlusEqual,
        MinusEqual,
        MulEqual,
        ConcatEqual,
        DivEqual,
        PowEqual,
        ModEqual,
        AndEqual,
        OrEqual,
        XorEqual,
        SlEqual,
        SrEqual,
    }

    #[repr(u8)]
    #[derive(Debug, Copy, Clone, Hash, Eq, PartialEq, Serialize)]
    enum OODeclExistsOp {
        Class,
        Interface,
        Trait,
    }

    unsafe extern "C++" {
        include!("hphp/hack/src/hackc/hhvm_cxx/hhvm_hhbc_defs/as-hhbc-ffi.h");
        type BareThisOp;
        type CollectionType;
        type ContCheckOp;
        type FatalOp;
        type FCallArgsFlags;
        type IncDecOp;
        type InitPropOp;
        type IsLogAsDynamicCallOp;
        type IsTypeOp;
        type MOpMode;
        type ObjMethodOp;
        type OODeclExistsOp;
        type QueryMOp;
        type ReadonlyOp;
        type SetOpOp;
        type SetRangeOp;
        type SilenceOp;
        type SpecialClsRef;
        type SwitchKind;
        type TypeStructResolveOp;
        type TypeStructEnforceKind;
        type AsTypeStructExceptionKind;
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
