// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(const_option)]

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

pub mod block;
pub mod class;
pub mod common;
pub mod constant;
pub mod func;
pub mod func_builder;
pub mod func_builder_ex;
pub mod instr;
pub mod module;
pub mod newtype;
pub mod string_intern;
pub mod type_const;
pub mod type_struct;
pub mod typed_value;
pub mod types;
pub mod unit;

// Re-export some types in from hhbc so users of `ir` don't have to figure out
// which random stuff to get from `ir` and which to get elsewhere.
pub use hhbc::intern;
pub use hhbc::intern_bytes;
pub use hhbc::string_id;
pub use hhbc::AsTypeStructExceptionKind;
pub use hhbc::BareThisOp;
pub use hhbc::BytesId;
pub use hhbc::CcParam;
pub use hhbc::CcReified;
pub use hhbc::CcThis;
pub use hhbc::ClassGetCMode;
pub use hhbc::ClassName;
pub use hhbc::ClassNameMap;
pub use hhbc::ClassishKind;
pub use hhbc::Coeffects;
pub use hhbc::CollectionType;
pub use hhbc::ConstName;
pub use hhbc::ContCheckOp;
pub use hhbc::CtxConstant;
pub use hhbc::FCallArgsFlags;
pub use hhbc::FatalOp;
pub use hhbc::FloatBits;
pub use hhbc::FunctionFlags;
pub use hhbc::FunctionName;
pub use hhbc::IncDecOp;
pub use hhbc::IncludePath;
pub use hhbc::InitPropOp;
pub use hhbc::IsLogAsDynamicCallOp;
pub use hhbc::IsTypeOp;
pub use hhbc::IterId;
pub use hhbc::MOpMode;
pub use hhbc::MethodFlags;
pub use hhbc::MethodName;
pub use hhbc::ModuleName;
pub use hhbc::OODeclExistsOp;
pub use hhbc::ObjMethodOp;
pub use hhbc::PropName;
pub use hhbc::QueryMOp;
pub use hhbc::ReadonlyOp;
pub use hhbc::Requirement;
pub use hhbc::SetOpOp;
pub use hhbc::SetRangeOp;
pub use hhbc::SilenceOp;
pub use hhbc::Span;
pub use hhbc::SpecialClsRef;
pub use hhbc::StringId;
pub use hhbc::SwitchKind;
pub use hhbc::SymbolRefs;
pub use hhbc::TraitReqKind;
pub use hhbc::TypeStructEnforceKind;
pub use hhbc::TypeStructResolveOp;
pub use hhbc::Visibility;
pub use hhvm_types_ffi::ffi::Attr;
pub use hhvm_types_ffi::ffi::TypeConstraintFlags;
pub use hhvm_types_ffi::ffi::TypeStructureKind;
pub use naming_special_names_rust::coeffects::Ctx;

pub use self::block::Block;
pub use self::class::Class;
pub use self::class::Property;
pub use self::common::Attribute;
pub use self::constant::Constant;
pub use self::constant::HackConstant;
pub use self::func::ExFrameId;
pub use self::func::Filename;
pub use self::func::Func;
pub use self::func::Function;
pub use self::func::Method;
pub use self::func::Param;
pub use self::func::SrcLoc;
pub use self::func::TParamBounds;
pub use self::func::TryCatchId;
pub use self::func_builder::FuncBuilder;
pub use self::func_builder::MemberOpBuilder;
pub use self::func_builder_ex::FuncBuilderEx;
pub use self::instr::Call;
pub use self::instr::HasEdges;
pub use self::instr::Instr;
pub use self::instr::LocalId;
pub use self::instr::Predicate;
pub use self::instr::UnnamedLocalId;
pub use self::module::Module;
pub use self::newtype::BlockId;
pub use self::newtype::BlockIdMap;
pub use self::newtype::BlockIdSet;
pub use self::newtype::ConstantId;
pub use self::newtype::FullInstrId;
pub use self::newtype::GlobalId;
pub use self::newtype::InstrId;
pub use self::newtype::InstrIdMap;
pub use self::newtype::InstrIdSet;
pub use self::newtype::LocId;
pub use self::newtype::ValueId;
pub use self::newtype::ValueIdMap;
pub use self::newtype::ValueIdSet;
pub use self::newtype::VarId;
pub use self::string_intern::StringInterner;
pub use self::string_intern::UnitBytesId;
pub use self::type_const::TypeConstant;
pub use self::typed_value::ArrayKey;
pub use self::typed_value::DictValue;
pub use self::typed_value::KeysetValue;
pub use self::typed_value::TypedValue;
pub use self::types::BaseType;
pub use self::types::EnforceableType;
pub use self::types::TypeInfo;
pub use self::types::Typedef;
pub use self::unit::Fatal;
pub use self::unit::Unit;
