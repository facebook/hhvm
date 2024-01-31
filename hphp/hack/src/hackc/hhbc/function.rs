// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use ffi::Slice;
use hhvm_types_ffi::ffi::Attr;
use serde::Serialize;

use crate::Attribute;
use crate::Body;
use crate::Coeffects;
use crate::FunctionName;
use crate::Param;
use crate::Span;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Function<'arena> {
    pub attributes: Slice<'arena, Attribute<'arena>>,
    pub name: FunctionName<'arena>,
    pub body: Body<'arena>,

    pub span: Span,
    pub coeffects: Coeffects<'arena>,
    pub flags: FunctionFlags,
    pub attrs: Attr,
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

impl<'arena> Function<'arena> {
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

    pub fn with_body<F, T>(&mut self, body: Body<'arena>, f: F) -> T
    where
        F: FnOnce() -> T,
    {
        let old_body = std::mem::replace(&mut self.body, body);
        let ret = f();
        self.body = old_body;
        ret
    }

    pub fn params(&self) -> &[Param<'arena>] {
        self.body.params.as_ref()
    }
}
