// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_pos::emit_pos;
use hhbc::FatalOp;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::pos::Pos;

pub fn emit_fatal(op: FatalOp, pos: &Pos, msg: impl AsRef<str>) -> InstrSeq {
    InstrSeq::gather(vec![
        emit_pos(pos),
        instr::string(msg.as_ref()),
        instr::fatal(op),
    ])
}

pub fn emit_fatal_runtime(pos: &Pos, msg: impl AsRef<str>) -> InstrSeq {
    emit_fatal(FatalOp::Runtime, pos, msg)
}

pub fn emit_fatal_runtimeomitframe(pos: &Pos, msg: impl AsRef<str>) -> InstrSeq {
    emit_fatal(FatalOp::RuntimeOmitFrame, pos, msg)
}

pub fn emit_fatal_for_break_continue(pos: &Pos) -> InstrSeq {
    emit_fatal_runtime(pos, "Cannot break/continue")
}
