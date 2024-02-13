// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_opcodes_macro::Flow;
use emit_opcodes_macro::Locals;
use emit_opcodes_macro::Targets;
use ffi::Str;
use ffi::Vector;
use serde::Serialize;

use crate::AdataId;
use crate::AsTypeStructExceptionKind;
use crate::BareThisOp;
use crate::ClassGetCMode;
use crate::ClassName;
use crate::CollectionType;
use crate::ConstName;
use crate::ContCheckOp;
use crate::Dummy;
use crate::FCallArgs;
use crate::FatalOp;
use crate::FloatBits;
use crate::Flow;
use crate::FunctionName;
use crate::IncDecOp;
use crate::InitPropOp;
use crate::IsLogAsDynamicCallOp;
use crate::IsTypeOp;
use crate::IterArgs;
use crate::IterId;
use crate::Label;
use crate::Local;
use crate::LocalRange;
use crate::Locals;
use crate::MOpMode;
use crate::MemberKey;
use crate::MethodName;
use crate::NumParams;
use crate::OODeclExistsOp;
use crate::ObjMethodOp;
use crate::PropName;
use crate::QueryMOp;
use crate::ReadonlyOp;
use crate::RepoAuthType;
use crate::SetOpOp;
use crate::SetRangeOp;
use crate::SilenceOp;
use crate::SpecialClsRef;
use crate::StackIndex;
use crate::SwitchKind;
use crate::Targets;
use crate::TypeStructEnforceKind;
use crate::TypeStructResolveOp;
use crate::NUM_ACT_REC_CELLS;

#[emit_opcodes_macro::emit_opcodes]
#[derive(Clone, Debug, Targets, Hash, Eq, PartialEq, Serialize, Flow, Locals)]
#[repr(C)]
pub enum Opcode<'arena> {
    // This is filled in by the emit_opcodes macro.  It can be printed using the
    // "//hphp/hack/src/hackc/hhbc:dump-opcodes" binary.
}

// The macro also generates:
// impl Opcode<'arena> {
//   pub fn variant_name(&self) -> &'static str;
//   pub fn variant_index(&self) -> usize;
//   pub fn num_inputs(&self) -> usize;
// }
// impl Targets for Opcode<'arena>;
