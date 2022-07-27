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

pub use self::block::Block;
pub use self::class::Class;
pub use self::coeffects::Coeffects;
pub use self::coeffects::CtxConstant;
pub use self::common::Attr;
pub use self::common::Attribute;
pub use self::common::TypedValue;
pub use self::func::ExFrameId;
pub use self::func::Func;
pub use self::func::Function;
pub use self::func::FunctionName;
pub use self::func::Method;
pub use self::func::MethodName;
pub use self::func::Param;
pub use self::func::TParamBounds;
pub use self::func::TryCatchId;
pub use self::func::Visibility;
pub use self::func_builder::FuncBuilder;
pub use self::instr::Call;
pub use self::instr::HasEdges;
pub use self::instr::Instr;
pub use self::instr::LocalId;
pub use self::instr::Predicate;
pub use self::instr::SrcLoc;
pub use self::instr::UnnamedLocalId;
pub use self::literal::HackConstant;
pub use self::literal::Literal;
pub use self::module::Module;
pub use self::newtype::BlockId;
pub use self::newtype::BlockIdMap;
pub use self::newtype::BlockIdSet;
pub use self::newtype::ClassId;
pub use self::newtype::ClassIdMap;
pub use self::newtype::ConstId;
pub use self::newtype::FullInstrId;
pub use self::newtype::FunctionId;
pub use self::newtype::InstrId;
pub use self::newtype::InstrIdMap;
pub use self::newtype::InstrIdSet;
pub use self::newtype::LiteralId;
pub use self::newtype::LocId;
pub use self::newtype::MethodId;
pub use self::newtype::PropId;
pub use self::newtype::ValueId;
pub use self::newtype::ValueIdMap;
pub use self::newtype::ValueIdSet;
pub use self::newtype::VarId;
pub use self::string_intern::StringInterner;
pub use self::string_intern::UnitStringId;
pub use self::type_const::TypeConstant;
pub use self::types::Type;
pub use self::unit::FatalOp;
pub use self::unit::Unit;
