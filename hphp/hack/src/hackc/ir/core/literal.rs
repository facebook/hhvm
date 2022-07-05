// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use crate::instr::HasLoc;
use crate::instr::HasOperands;
use crate::LocId;
use crate::TypedValue;
use crate::ValueId;
use ffi::Str;

pub use hhbc::AdataId;
pub use hhbc::CollectionType;
pub use hhbc::ConstName;
pub use hhbc::FloatBits;

/// A literal value. Note that since these refer to a LocId they're only valid
/// in relation to a specific Func.
#[derive(Debug)]
pub enum Literal<'a> {
    Bool(bool, LocId),
    Dict(AdataId<'a>, LocId),
    Dir(LocId),
    Double(FloatBits, LocId),
    File(LocId),
    FuncCred(LocId),
    Int(i64, LocId),
    Keyset(AdataId<'a>, LocId),
    Method(LocId),
    Named(ConstName<'a>, LocId),
    NewCol(CollectionType, LocId),
    Null(LocId),
    NullUninit(LocId),
    String(Str<'a>, LocId),
    Vec(AdataId<'a>, LocId),
}

impl HasLoc for Literal<'_> {
    fn loc_id(&self) -> LocId {
        match self {
            Literal::Bool(_, loc)
            | Literal::Dict(_, loc)
            | Literal::Dir(loc)
            | Literal::Double(_, loc)
            | Literal::File(loc)
            | Literal::FuncCred(loc)
            | Literal::Int(_, loc)
            | Literal::Keyset(_, loc)
            | Literal::Method(loc)
            | Literal::Named(_, loc)
            | Literal::NewCol(_, loc)
            | Literal::Null(loc)
            | Literal::NullUninit(loc)
            | Literal::String(_, loc)
            | Literal::Vec(_, loc) => *loc,
        }
    }
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

impl Literal<'_> {
    pub fn boolean<'a>(value: bool, loc: LocId) -> Literal<'a> {
        Literal::Bool(value, loc)
    }

    pub fn dict<'a>(name: AdataId<'a>, loc: LocId) -> Literal<'a> {
        Literal::Dict(name, loc)
    }

    pub fn double<'a>(d: f64, loc: LocId) -> Literal<'a> {
        Literal::Double(d.into(), loc)
    }

    pub fn int<'a>(i: i64, loc: LocId) -> Literal<'a> {
        Literal::Int(i, loc)
    }

    pub fn keyset<'a>(name: AdataId<'a>, loc: LocId) -> Literal<'a> {
        Literal::Keyset(name, loc)
    }

    pub fn named<'a>(name: ConstName<'a>, loc: LocId) -> Literal<'a> {
        Literal::Named(name, loc)
    }

    pub fn new_col<'a>(kind: CollectionType, loc: LocId) -> Literal<'a> {
        Literal::NewCol(kind, loc)
    }

    pub fn null<'a>(loc: LocId) -> Literal<'a> {
        Literal::Null(loc)
    }

    pub fn null_uninit<'a>(loc: LocId) -> Literal<'a> {
        Literal::NullUninit(loc)
    }

    pub fn string<'a>(s: Str<'a>, loc: LocId) -> Literal<'a> {
        Literal::String(s, loc)
    }

    pub fn vec<'a>(name: AdataId<'a>, loc: LocId) -> Literal<'a> {
        Literal::Vec(name, loc)
    }
}

#[derive(Debug)]
pub struct HackConstant<'a> {
    pub name: ConstName<'a>,
    pub value: Option<TypedValue<'a>>,
    pub is_abstract: bool,
}
