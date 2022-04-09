// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

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
pub use hhbc_ast::{
    AdataId, ClassNum, ClassishKind, FCallArgs, HasGenericsOp, Instruct, IterArgs, IterId, Label,
    Local, LocalRange, MemberKey, NumParams, ParamName, Pseudo, RepoAuthType, SrcLoc, StackIndex,
    Targets, Visibility,
};
pub use hhbc_id::{ClassName, ConstName, FunctionName, MethodName, PropName};
pub use hhvm_hhbc_defs_ffi::ffi::{
    BareThisOp, CollectionType, ContCheckOp, FCallArgsFlags, FatalOp, IncDecOp, InitPropOp,
    IsLogAsDynamicCallOp, IsTypeOp, MOpMode, OODeclExistsOp, ObjMethodOp, QueryMOp, ReadonlyOp,
    SetOpOp, SetRangeOp, SilenceOp, SpecialClsRef, SwitchKind, TypeStructResolveOp,
};
pub use opcodes::Opcode;
pub use typed_value::TypedValue;
