// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    AdataId, BareThisOp, ClassName, ClassNum, CollectionType, ConstName, ContCheckOp, FCallArgs,
    FatalOp, FunctionName, IncDecOp, InitPropOp, IsLogAsDynamicCallOp, IsTypeOp, IterArgs, IterId,
    Label, Local, LocalRange, MOpMode, MemberKey, MethodName, NumParams, OODeclExistsOp,
    ObjMethodOp, ParamName, PropName, QueryMOp, ReadonlyOp, RepoAuthType, SetOpOp, SetRangeOp,
    SilenceOp, SpecialClsRef, StackIndex, SwitchKind, Targets, TypeStructResolveOp,
};
use emit_opcodes_macro::Targets;
use ffi::{BumpSliceMut, Slice, Str};

#[emit_opcodes_macro::emit_opcodes]
#[derive(Clone, Debug, Targets, Hash, Eq, PartialEq)]
#[repr(C)]
pub enum Opcode<'arena> {
    // This is filled in by the emit_opcodes macro.  It can be printed using the
    // "//hphp/hack/src/hackc/hhbc:dump-opcodes" binary.
}
