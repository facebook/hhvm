// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod hhbc_ast;

pub mod adata_state;
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
pub mod hhas_xhp_attribute;
pub mod hhbc_id;
pub mod opcodes;
pub mod typed_value;

pub use hhbc_ast::{
    AdataId, BareThisOp, ClassId, ClassNum, ClassishKind, CollectionType, ConstId, ContCheckOp,
    FCallArgs, FCallArgsFlags, FatalOp, FunctionId, HasGenericsOp, IncDecOp, InitPropOp, Instruct,
    IsLogAsDynamicCallOp, IsTypeOp, IterArgs, IterId, Label, Local, LocalRange, MOpMode, MemberKey,
    MethodId, NumParams, OODeclExistsOp, ObjMethodOp, Opcode, ParamId, PropId, Pseudo, QueryMOp,
    ReadonlyOp, SetOpOp, SetRangeOp, SilenceOp, SpecialClsRef, SrcLoc, StackIndex, SwitchKind,
    TypeStructResolveOp, Visibility,
};
