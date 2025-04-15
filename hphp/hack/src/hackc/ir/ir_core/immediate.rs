// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use crate::ArcVec;
use crate::BytesId;
use crate::ClassName;
use crate::CollectionType;
use crate::ConstName;
use crate::DictEntry;
use crate::FloatBits;
use crate::TypedValue;
use crate::ValueId;
use crate::instr::HasOperands;

/// An immediate constant value.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Immediate {
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
    Vec(ArcVec<TypedValue>),
    Dict(ArcVec<DictEntry>),
    Keyset(ArcVec<TypedValue>),
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
            TypedValue::LazyClass(id) => Self::LazyClass(id),
            TypedValue::Null => Self::Null,
            TypedValue::String(id) => Self::String(id),
            TypedValue::Uninit => Self::Uninit,
            TypedValue::Vec(v) => Self::Vec(v),
            TypedValue::Dict(v) => Self::Dict(v),
            TypedValue::Keyset(v) => Self::Keyset(v),
        }
    }
}

impl TryFrom<Immediate> for TypedValue {
    type Error = Immediate;

    /// Convert Immediate->TypedValue for all the TypedValue variants,
    /// otherwise return the original Immediate.
    fn try_from(imm: Immediate) -> Result<Self, Self::Error> {
        match imm {
            Immediate::Bool(b) => Ok(Self::Bool(b)),
            Immediate::Float(f) => Ok(Self::Float(f)),
            Immediate::Int(i) => Ok(Self::Int(i)),
            Immediate::LazyClass(id) => Ok(Self::LazyClass(id)),
            Immediate::Null => Ok(Self::Null),
            Immediate::String(id) => Ok(Self::String(id)),
            Immediate::Uninit => Ok(Self::Uninit),
            Immediate::Vec(v) => Ok(Self::Vec(v)),
            Immediate::Dict(v) => Ok(Self::Dict(v)),
            Immediate::Keyset(v) => Ok(Self::Keyset(v)),

            imm @ (Immediate::Dir
            | Immediate::EnumClassLabel(_)
            | Immediate::File
            | Immediate::FuncCred
            | Immediate::Method
            | Immediate::Named(_)
            | Immediate::NewCol(_)) => Err(imm),
        }
    }
}
