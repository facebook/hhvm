// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_hhas_body::HhasBody;
use hhbc_by_ref_hhas_coeffects::HhasCoeffects;
use hhbc_by_ref_hhas_pos as hhas_pos;
use hhbc_by_ref_hhbc_id::method;
use oxidized::aast_defs::Visibility;

use bitflags::bitflags;

#[derive(Debug)]
pub struct HhasMethod<'arena> {
    pub attributes: Vec<HhasAttribute<'arena>>,
    pub visibility: Visibility,
    pub name: method::Type<'arena>,
    pub body: HhasBody<'arena>,
    pub span: hhas_pos::Span,
    pub coeffects: HhasCoeffects,
    pub flags: HhasMethodFlags,
}

bitflags! {
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
        const RX_DISABLED = 1 << 10;
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
