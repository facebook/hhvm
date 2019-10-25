// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhas_attribute_rust::HhasAttribute;
use hhbc_id_rust as hhbc_id;
use instruction_sequence_rust as instruction_sequence;
use oxidized::{aast_defs::Visibility, doc_comment::DocComment};

extern crate bitflags;
use bitflags::bitflags;

bitflags! {
    pub struct HhasPropertyFlags: u16 {
        const IS_ABSTRACT =          0b0000_0000_0001;
        const IS_STATIC =            0b0000_0000_0010;
        const IS_DEEP_INIT =         0b0000_0000_0100;
        const IS_CONST =             0b0000_0000_1000;
        const IS_LSB =               0b0000_0001_0000;
        const IS_NO_BAD_REDECLARE =  0b0000_0010_0000;
        const HAS_SYSTEM_INITIAL =   0b0000_0100_0000;
        const NO_IMPLICIT_NULL =     0b0000_1000_0000;
        const INITIAL_SATISFIES_TC = 0b0001_0000_0000;
        const IS_LATE_INIT =         0b0100_0000_0000;
    }
}

pub struct HhasProperty<'id> {
    pub name: hhbc_id::prop::Type<'id>,
    pub flags: HhasPropertyFlags,
    pub attributes: Vec<HhasAttribute>,
    pub visibility: Visibility,
    pub initial_value: runtime::TypedValue,
    pub initializer_instrs: instruction_sequence::Instr,
    pub type_info: hhas_type::Info,
    pub doc_comment: Option<DocComment>,
}
impl<'a> HhasProperty<'a> {
    pub fn is_private(&self) -> bool {
        self.visibility == Visibility::Private
    }
    pub fn is_protected(&self) -> bool {
        self.visibility == Visibility::Protected
    }
    pub fn is_public(&self) -> bool {
        self.visibility == Visibility::Public
    }
}
