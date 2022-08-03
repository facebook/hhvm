// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(box_patterns)]

mod hhas_xhp_attribute;
mod hhbc_ast;
mod hhbc_id;
mod opcodes;
mod typed_value;

pub mod decl_vars;
pub mod hackc_unit;
pub mod hhas_adata;
pub mod hhas_attribute;
pub mod hhas_body;
pub mod hhas_class;
pub mod hhas_coeffects;
pub mod hhas_constant;
pub mod hhas_function;
pub mod hhas_method;
pub mod hhas_module;
pub mod hhas_param;
pub mod hhas_pos;
pub mod hhas_property;
pub mod hhas_symbol_refs;
pub mod hhas_type;
pub mod hhas_type_const;
pub mod hhas_typedef;

pub use hhas_xhp_attribute::HhasXhpAttribute;
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
pub use opcodes::Opcode;
pub use typed_value::FloatBits;
pub use typed_value::TypedValue;
