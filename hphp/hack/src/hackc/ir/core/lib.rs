// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! # The HackC IR
//!
//! This is an IR representation of the Hack code.
//!
//! A Hack program is organized into "units".  Each `Unit` (see `Unit` in
//! unit.rs) represents a single Hack source file - although there is no reason
//! multiple units couldn't be merged together to form a larger unit.
//!
//! Units are composed of the information needed to represent the program:
//! classes, constants, functions, typedefs.
//!
//! Each class (see `Class` in class.rs) has a class description and a list of
//! methods.
//!
//! Methods (see `Method` in func.rs) and functions (see `Function` in func.rs)
//! are backed by a common `Func` structure (see `Func` in func.rs) which is the
//! basic representation of a Hack callable.
//!
//! Funcs are composed of instructions (see `Instr` in instr.rs) and basic
//! blocks (see `Block` in block.rs).
//!

pub mod block;
pub mod class;
pub mod coeffects;
pub mod common;
pub mod func;
pub mod func_builder;
pub mod instr;
pub mod literal;
pub mod module;
pub mod newtype;
pub mod string_intern;
pub mod type_const;
pub mod types;
pub mod unit;

pub use self::{
    block::Block,
    class::Class,
    coeffects::{Coeffects, CtxConstant},
    common::{Attr, Attribute, TypedValue},
    func::{
        ExFrameId, Func, Function, FunctionName, Method, MethodName, Param, TryCatchId, Visibility,
    },
    func_builder::FuncBuilder,
    instr::{Call, HasEdges, Instr, LocalId, Predicate, SrcLoc, UnnamedLocalId},
    literal::{HackConstant, Literal},
    module::Module,
    newtype::{
        BlockId, BlockIdMap, BlockIdSet, ClassId, ConstId, FullInstrId, FunctionId, InstrId,
        InstrIdMap, InstrIdSet, LiteralId, LocId, MethodId, PropId, ValueId, ValueIdMap,
        ValueIdSet,
    },
    string_intern::{StringInterner, UnitStringId},
    type_const::TypeConstant,
    types::Type,
    unit::{FatalOp, Unit},
};
