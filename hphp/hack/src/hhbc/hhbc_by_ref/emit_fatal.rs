// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_emit_pos::emit_pos;
use hhbc_by_ref_hhbc_ast::FatalOp;
use hhbc_by_ref_instruction_sequence::{instr, Error, InstrSeq};
use oxidized::pos::Pos;

pub fn raise_fatal_runtime(pos: &Pos, msg: impl Into<String>) -> Error {
    Error::IncludeTimeFatalException(FatalOp::Runtime, pos.clone(), msg.into())
}

pub fn raise_fatal_parse(pos: &Pos, msg: impl Into<String>) -> Error {
    Error::IncludeTimeFatalException(FatalOp::Parse, pos.clone(), msg.into())
}

pub fn emit_fatal<'arena>(
    alloc: &'arena bumpalo::Bump,
    op: FatalOp,
    pos: &Pos,
    msg: impl AsRef<str>,
) -> InstrSeq<'arena> {
    InstrSeq::gather(
        alloc,
        vec![
            emit_pos(alloc, pos),
            instr::string(alloc, msg.as_ref()),
            instr::fatal(alloc, op),
        ],
    )
}

pub fn emit_fatal_runtime<'arena>(
    alloc: &'arena bumpalo::Bump,
    pos: &Pos,
    msg: impl AsRef<str>,
) -> InstrSeq<'arena> {
    emit_fatal(alloc, FatalOp::Runtime, pos, msg)
}

pub fn emit_fatal_runtimeomitframe<'arena>(
    alloc: &'arena bumpalo::Bump,
    pos: &Pos,
    msg: impl AsRef<str>,
) -> InstrSeq<'arena> {
    emit_fatal(alloc, FatalOp::RuntimeOmitFrame, pos, msg)
}

pub fn emit_fatal_for_break_continue<'arena>(
    alloc: &'arena bumpalo::Bump,
    pos: &Pos,
    level: usize,
) -> InstrSeq<'arena> {
    let suffix = if level == 1 { "" } else { "s" };
    let msg = format!("Cannot break/continue {} level{}", level, suffix);
    emit_fatal_runtime(alloc, pos, msg)
}
