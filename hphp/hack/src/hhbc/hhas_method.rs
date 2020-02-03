// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhas_attribute_rust::HhasAttribute;
use hhas_body_rust::HhasBody;
use hhas_pos_rust as hhas_pos;
use hhbc_id_rust::method;
use oxidized::aast_defs::Visibility;
use rx_rust as rx;

extern crate bitflags;
use bitflags::bitflags;

pub struct HhasMethod<'id> {
    pub attributes: Vec<HhasAttribute>,
    pub visibility: Visibility,
    pub name: method::Type<'id>,
    pub body: HhasBody,
    pub span: hhas_pos::Span,
    pub rx_level: rx::Level,
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
