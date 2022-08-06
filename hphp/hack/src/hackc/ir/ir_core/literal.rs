// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use ffi::Str;
pub use hhbc::AdataId;
pub use hhbc::CollectionType;
pub use hhbc::ConstName;
pub use hhbc::FloatBits;

use crate::instr::HasOperands;
use crate::TypedValue;
use crate::ValueId;

/// A literal value. Note that since these refer to a LocId they're only valid
/// in relation to a specific Func.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Literal<'a> {
    Bool(bool),
    Dict(AdataId<'a>),
    Dir,
    Double(FloatBits),
    File,
    FuncCred,
    Int(i64),
    Keyset(AdataId<'a>),
    Method,
    Named(ConstName<'a>),
    NewCol(CollectionType),
    Null,
    String(Str<'a>),
    Uninit,
    Vec(AdataId<'a>),
}

impl HasOperands for Literal<'_> {
    fn operands(&self) -> &[ValueId] {
        // By definition constants don't have operands.
        &[]
    }

    fn operands_mut(&mut self) -> &mut [ValueId] {
        // By definition constants don't have operands.
        &mut []
    }
}

#[derive(Debug)]
pub struct HackConstant<'a> {
    pub name: ConstName<'a>,
    pub value: Option<TypedValue<'a>>,
    pub is_abstract: bool,
}
