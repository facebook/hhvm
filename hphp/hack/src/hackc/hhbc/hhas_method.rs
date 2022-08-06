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
use crate::hhas_pos::HhasSpan;
use crate::hhbc_ast::Visibility;
use crate::MethodName;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct HhasMethod<'arena> {
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub visibility: Visibility,
    pub name: MethodName<'arena>,
    pub body: HhasBody<'arena>,
    pub span: HhasSpan,
    pub coeffects: HhasCoeffects<'arena>,
    pub flags: HhasMethodFlags,
    pub attrs: Attr,
}

bitflags! {
    #[derive(Serialize)]
    #[repr(C)]
    pub struct HhasMethodFlags: u16 {
        const IS_ASYNC = 1 << 0;
        const IS_GENERATOR = 1 << 1;
        const IS_PAIR_GENERATOR = 1 << 2;
        const IS_CLOSURE_BODY = 1 << 3;
    }
}

impl<'a, 'arena> HhasMethod<'arena> {
    pub fn is_closure_body(&self) -> bool {
        self.flags.contains(HhasMethodFlags::IS_CLOSURE_BODY)
    }
}
