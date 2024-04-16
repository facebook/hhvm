// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use serde::Serialize;

use crate::BodyImpl;
use crate::FunctionName;
use crate::ParamEntry;

pub type Function = FunctionImpl<crate::BcRepr>;

/// A top-level Hack function.
#[derive(Debug, Clone, Serialize)]
#[repr(C)]
pub struct FunctionImpl<R> {
    pub name: FunctionName,
    pub body: BodyImpl<R>,
    pub flags: FunctionFlags,
}

bitflags! {
    #[derive(Default, Serialize, PartialEq, Eq, PartialOrd, Ord, Hash, Debug, Clone, Copy)]
    #[repr(C)]
    pub struct FunctionFlags: u8 {
        const ASYNC =          1 << 0;
        const GENERATOR =      1 << 1;
        const PAIR_GENERATOR = 1 << 2;
        const MEMOIZE_IMPL =   1 << 3;
    }
}

impl Function {
    pub fn is_async(&self) -> bool {
        self.flags.contains(FunctionFlags::ASYNC)
    }

    pub fn is_generator(&self) -> bool {
        self.flags.contains(FunctionFlags::GENERATOR)
    }

    pub fn is_pair_generator(&self) -> bool {
        self.flags.contains(FunctionFlags::PAIR_GENERATOR)
    }

    pub fn is_memoize_impl(&self) -> bool {
        self.flags.contains(FunctionFlags::MEMOIZE_IMPL)
    }

    pub fn params(&self) -> &[ParamEntry] {
        self.body.repr.params.as_ref()
    }
}
