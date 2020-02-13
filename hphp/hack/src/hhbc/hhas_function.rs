// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use closure_convert_rust::HoistKind;
use hhas_attribute_rust::HhasAttribute;
use hhas_body_rust::HhasBody;
use hhas_param_rust::HhasParam;

use hhas_pos_rust as hhas_pos;
use hhbc_id_rust as hhbc_id;
use rx_rust as rx;

extern crate bitflags;
use bitflags::bitflags;

#[derive(Debug)]
pub struct HhasFunction<'a> {
    pub attributes: Vec<HhasAttribute>,
    pub name: hhbc_id::function::Type<'a>,
    pub body: HhasBody<'a>,
    pub span: hhas_pos::Span,
    pub rx_level: rx::Level,
    pub hoisted: HoistKind,
    pub flags: Flags,
}

bitflags! {
    pub struct Flags: u8 {
        const ASYNC =          1 << 1;
        const GENERATOR =      1 << 2;
        const PAIR_GENERATOR = 1 << 3;
        const NO_INJECTION =   1 << 4;
        const INTERCEPTABLE =  1 << 5;
        const MEMOIZE_IMPL =   1 << 6;
        const RX_DISABLED =    1 << 7;
    }
}

impl<'a> HhasFunction<'a> {
    pub fn is_top(&self) -> bool {
        match self.hoisted {
            HoistKind::TopLevel => true,
            HoistKind::Hoisted => false,
        }
    }

    pub fn is_async(&self) -> bool {
        self.flags.contains(Flags::ASYNC)
    }

    pub fn is_generator(&self) -> bool {
        self.flags.contains(Flags::GENERATOR)
    }

    pub fn is_pair_generator(&self) -> bool {
        self.flags.contains(Flags::PAIR_GENERATOR)
    }

    pub fn is_interceptable(&self) -> bool {
        self.flags.contains(Flags::INTERCEPTABLE)
    }

    pub fn is_no_injection(&self) -> bool {
        self.flags.contains(Flags::NO_INJECTION)
    }

    pub fn is_memoize_impl(&self) -> bool {
        self.flags.contains(Flags::MEMOIZE_IMPL)
    }

    pub fn rx_disabled(&self) -> bool {
        self.flags.contains(Flags::RX_DISABLED)
    }

    pub fn with_body<F, T>(&mut self, body: HhasBody<'a>, f: F) -> T
    where
        F: FnOnce() -> T,
    {
        let old_body = std::mem::replace(&mut self.body, body);
        let ret = f();
        std::mem::replace(&mut self.body, old_body);
        ret
    }

    pub fn params(&self) -> &[HhasParam] {
        self.body.params.as_slice()
    }
}
