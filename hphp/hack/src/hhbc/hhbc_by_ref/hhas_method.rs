// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Slice;
use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_hhas_body::HhasBody;
use hhbc_by_ref_hhas_coeffects::HhasCoeffects;
use hhbc_by_ref_hhas_pos::HhasSpan;
use hhbc_by_ref_hhbc_ast::Visibility;
use hhbc_by_ref_hhbc_id::method::MethodType;

use bitflags::bitflags;

#[derive(Debug)]
pub struct HhasMethod<'arena> {
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub visibility: Visibility,
    pub name: MethodType<'arena>,
    pub body: HhasBody<'arena>,
    pub span: HhasSpan,
    pub coeffects: HhasCoeffects, //TODO(SF, 2021-08-10): Fix when Steve's `HhasCoeffect`'s (`repr(C)`)work lands
    pub flags: HhasMethodFlags,
}

bitflags! {
    #[repr(C)]
    pub struct HhasMethodFlags: u16 {
        const IS_STATIC = 1 << 1;
        const IS_FINAL = 1 << 2;
        const IS_ABSTRACT = 1 << 3;
        const IS_ASYNC = 1 << 4;
        const IS_GENERATOR = 1 << 5;
        const IS_PAIR_GENERATOR = 1 << 6;
        const IS_CLOSURE_BODY = 1 << 7;
        const IS_INTERCEPTABLE = 1 << 8;
        const IS_MEMOIZE_IMPL = 1 << 9;
        const IS_READONLY_RETURN = 1 << 10;
        const NO_INJECTION = 1 << 11;
    }
}

impl<'a, 'arena> HhasMethod<'arena> {
    pub fn is_closure_body(&self) -> bool {
        self.flags.contains(HhasMethodFlags::IS_CLOSURE_BODY)
    }

    pub fn is_no_injection(&self) -> bool {
        self.flags.contains(HhasMethodFlags::NO_INJECTION)
    }

    pub fn is_memoize_impl(&self) -> bool {
        self.flags.contains(HhasMethodFlags::IS_MEMOIZE_IMPL)
    }

    pub fn is_interceptable(&self) -> bool {
        self.flags.contains(HhasMethodFlags::IS_INTERCEPTABLE)
    }
}
