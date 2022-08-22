// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(box_patterns)]

mod hhbc_ast;
mod hhbc_id;
mod opcodes;
mod typed_value;
mod xhp_attribute;

mod adata;
mod attribute;
mod body;
mod class;
mod coeffects;
mod constant;
pub mod decl_vars;
mod function;
mod method;
mod module;
mod param;
mod pos;
mod property;
mod symbol_refs;
mod type_const;
mod typedef;
mod types;
mod unit;

pub use adata::*;
pub use attribute::*;
pub use body::*;
pub use class::*;
pub use coeffects::*;
pub use constant::*;
pub use function::*;
pub use hhbc_ast::AdataId;
pub use hhbc_ast::ClassNum;
pub use hhbc_ast::ClassishKind;
pub use hhbc_ast::Dummy;
pub use hhbc_ast::FCallArgs;
pub use hhbc_ast::HasGenericsOp;
pub use hhbc_ast::Instruct;
pub use hhbc_ast::IterArgs;
pub use hhbc_ast::IterId;
pub use hhbc_ast::Label;
pub use hhbc_ast::Local;
pub use hhbc_ast::LocalRange;
pub use hhbc_ast::MemberKey;
pub use hhbc_ast::NumParams;
pub use hhbc_ast::Pseudo;
pub use hhbc_ast::RepoAuthType;
pub use hhbc_ast::SrcLoc;
pub use hhbc_ast::StackIndex;
pub use hhbc_ast::Targets;
pub use hhbc_ast::Visibility;
pub use hhbc_id::ClassName;
pub use hhbc_id::ConstName;
pub use hhbc_id::FunctionName;
pub use hhbc_id::MethodName;
pub use hhbc_id::PropName;
pub use hhvm_hhbc_defs_ffi::ffi::BareThisOp;
pub use hhvm_hhbc_defs_ffi::ffi::CollectionType;
pub use hhvm_hhbc_defs_ffi::ffi::ContCheckOp;
pub use hhvm_hhbc_defs_ffi::ffi::FCallArgsFlags;
pub use hhvm_hhbc_defs_ffi::ffi::FatalOp;
pub use hhvm_hhbc_defs_ffi::ffi::IncDecOp;
pub use hhvm_hhbc_defs_ffi::ffi::InitPropOp;
pub use hhvm_hhbc_defs_ffi::ffi::IsLogAsDynamicCallOp;
pub use hhvm_hhbc_defs_ffi::ffi::IsTypeOp;
pub use hhvm_hhbc_defs_ffi::ffi::MOpMode;
pub use hhvm_hhbc_defs_ffi::ffi::OODeclExistsOp;
pub use hhvm_hhbc_defs_ffi::ffi::ObjMethodOp;
pub use hhvm_hhbc_defs_ffi::ffi::QueryMOp;
pub use hhvm_hhbc_defs_ffi::ffi::ReadonlyOp;
pub use hhvm_hhbc_defs_ffi::ffi::SetOpOp;
pub use hhvm_hhbc_defs_ffi::ffi::SetRangeOp;
pub use hhvm_hhbc_defs_ffi::ffi::SilenceOp;
pub use hhvm_hhbc_defs_ffi::ffi::SpecialClsRef;
pub use hhvm_hhbc_defs_ffi::ffi::SwitchKind;
pub use hhvm_hhbc_defs_ffi::ffi::TypeStructResolveOp;
pub use method::*;
pub use module::*;
pub use opcodes::Opcode;
pub use param::*;
pub use pos::*;
pub use property::*;
pub use symbol_refs::*;
pub use type_const::*;
pub use typed_value::FloatBits;
pub use typed_value::TypedValue;
pub use typedef::*;
pub use types::*;
pub use unit::*;
pub use xhp_attribute::XhpAttribute;
