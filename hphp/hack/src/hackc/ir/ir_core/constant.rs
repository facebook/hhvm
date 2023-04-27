// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use std::sync::Arc;

use crate::instr::HasOperands;
use crate::CollectionType;
use crate::ConstId;
use crate::ConstName;
use crate::FloatBits;
use crate::TypedValue;
use crate::UnitBytesId;
use crate::ValueId;

/// A constant value.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Constant<'a> {
    Array(Arc<TypedValue>),
    Bool(bool),
    Dir,
    File,
    Float(FloatBits),
    FuncCred,
    Int(i64),
    Method,
    Named(ConstName<'a>),
    NewCol(CollectionType),
    Null,
    String(UnitBytesId),
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
pub struct HackConstant {
    pub name: ConstId,
    pub value: Option<TypedValue>,
    pub is_abstract: bool,
}
