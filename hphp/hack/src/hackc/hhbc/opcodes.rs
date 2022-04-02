// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    AdataId, BareThisOp, BumpSliceMut, ClassId, ClassNum, CollectionType, ConstId, ContCheckOp,
    FCallArgs, FatalOp, FunctionId, IncDecOp, InitPropOp, IsLogAsDynamicCallOp, IsTypeOp, IterArgs,
    IterId, Label, Local, LocalRange, MOpMode, MemberKey, MethodId, NumParams, OODeclExistsOp,
    ObjMethodOp, ParamId, PropId, QueryMOp, ReadonlyOp, RepoAuthType, SetOpOp, SetRangeOp,
    SilenceOp, Slice, SpecialClsRef, StackIndex, Str, SwitchKind, Targets, TypeStructResolveOp,
};
use emit_opcodes_macro::Targets;

#[emit_opcodes_macro::emit_opcodes]
#[derive(Clone, Debug, Targets)]
#[repr(C)]
pub enum Opcode<'arena> {
    // This is filled in by the emit_opcodes macro.  It can be printed using the
    // "//hphp/hack/src/hackc/hhbc:dump-opcodes" binary.
}
