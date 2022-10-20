// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use std::sync::Arc;

use ffi::Str;
pub use hhbc::CollectionType;
pub use hhbc::ConstName;
pub use hhbc::FloatBits;

use crate::instr::HasOperands;
use crate::TypedValue;
use crate::ValueId;

/// A constant value.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Constant<'a> {
    Array(Arc<TypedValue<'a>>),
    Bool(bool),
    Dir,
    Double(FloatBits),
    File,
    FuncCred,
    Int(i64),
    Method,
    Named(ConstName<'a>),
    NewCol(CollectionType),
    Null,
    String(Str<'a>),
    Uninit,
}

impl HasOperands for Constant<'_> {
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
