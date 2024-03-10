// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use std::sync::Arc;

use hhvm_types_ffi::Attr;

use crate::instr::HasOperands;
use crate::BytesId;
use crate::ClassName;
use crate::CollectionType;
use crate::ConstName;
use crate::FloatBits;
use crate::TypedValue;
use crate::ValueId;

/// A constant value.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Constant {
    Array(Arc<TypedValue>),
    Bool(bool),
    Dir,
    EnumClassLabel(BytesId),
    File,
    Float(FloatBits),
    FuncCred,
    Int(i64),
    LazyClass(ClassName),
    Method,
    Named(ConstName),
    NewCol(CollectionType),
    Null,
    String(BytesId),
    Uninit,
}

impl HasOperands for Constant {
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
    pub name: ConstName,
    pub value: Option<TypedValue>,
    pub attrs: Attr,
}
