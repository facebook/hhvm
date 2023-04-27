// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_pos::emit_pos;
use hhbc::FatalOp;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::pos::Pos;

pub fn emit_fatal<'arena>(
    alloc: &'arena bumpalo::Bump,
    op: FatalOp,
    pos: &Pos,
    msg: impl AsRef<str>,
) -> InstrSeq<'arena> {
    InstrSeq::gather(vec![
        emit_pos(pos),
        instr::string(alloc, msg.as_ref()),
        instr::fatal(op),
    ])
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
) -> InstrSeq<'arena> {
    emit_fatal_runtime(alloc, pos, "Cannot break/continue")
}
