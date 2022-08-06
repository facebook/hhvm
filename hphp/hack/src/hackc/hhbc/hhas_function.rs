// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use ffi::Slice;
use hhvm_types_ffi::ffi::Attr;
use serde::Serialize;

use crate::hhas_attribute::HhasAttribute;
use crate::hhas_body::HhasBody;
use crate::hhas_coeffects::HhasCoeffects;
use crate::hhas_param::HhasParam;
use crate::hhas_pos::HhasSpan;
use crate::FunctionName;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct HhasFunction<'arena> {
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub name: FunctionName<'arena>,
    pub body: HhasBody<'arena>,

    pub span: HhasSpan,
    pub coeffects: HhasCoeffects<'arena>,
    pub flags: HhasFunctionFlags,
    pub attrs: Attr,
}

bitflags! {
    #[derive(Serialize)]
    #[repr(C)]
    pub struct HhasFunctionFlags: u8 {
        const ASYNC =          1 << 0;
        const GENERATOR =      1 << 1;
        const PAIR_GENERATOR = 1 << 2;
        const MEMOIZE_IMPL =   1 << 3;
    }
}

impl<'arena> HhasFunction<'arena> {
    pub fn is_async(&self) -> bool {
        self.flags.contains(HhasFunctionFlags::ASYNC)
    }

    pub fn is_generator(&self) -> bool {
        self.flags.contains(HhasFunctionFlags::GENERATOR)
    }

    pub fn is_pair_generator(&self) -> bool {
        self.flags.contains(HhasFunctionFlags::PAIR_GENERATOR)
    }

    pub fn is_memoize_impl(&self) -> bool {
        self.flags.contains(HhasFunctionFlags::MEMOIZE_IMPL)
    }

    pub fn with_body<F, T>(&mut self, body: HhasBody<'arena>, f: F) -> T
    where
        F: FnOnce() -> T,
    {
        let old_body = std::mem::replace(&mut self.body, body);
        let ret = f();
        self.body = old_body;
        ret
    }

    pub fn params(&self) -> &[HhasParam<'arena>] {
        self.body.params.as_ref()
    }
}
