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

/// An immediate constant value.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Immediate {
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

impl HasOperands for Immediate {
    fn operands(&self) -> &[ValueId] {
        // By definition constants don't have operands.
        &[]
    }

    fn operands_mut(&mut self) -> &mut [ValueId] {
        // By definition constants don't have operands.
        &mut []
    }
}

impl From<TypedValue> for Immediate {
    fn from(tv: TypedValue) -> Self {
        match tv {
            TypedValue::Bool(b) => Self::Bool(b),
            TypedValue::Float(f) => Self::Float(f),
            TypedValue::Int(i) => Self::Int(i),
            TypedValue::LazyClass(id) => Self::String(id.as_bytes_id()),
            TypedValue::Null => Self::Null,
            TypedValue::String(id) => Self::String(id),
            TypedValue::Uninit => Self::Uninit,
            TypedValue::Dict(_) | TypedValue::Keyset(_) | TypedValue::Vec(_) => {
                Self::Array(Arc::new(tv))
            }
        }
    }
}

#[derive(Debug)]
pub struct HackConstant {
    pub name: ConstName,
    pub value: Option<TypedValue>,
    pub attrs: Attr,
}
