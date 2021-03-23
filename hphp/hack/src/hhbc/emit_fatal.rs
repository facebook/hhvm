// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_pos_rust::emit_pos;
use hhbc_ast_rust::FatalOp;
use instruction_sequence::{instr, Error, InstrSeq};
use oxidized::pos::Pos;

pub fn raise_fatal_runtime(pos: &Pos, msg: impl Into<String>) -> Error {
    Error::IncludeTimeFatalException(FatalOp::Runtime, pos.clone(), msg.into())
}

pub fn raise_fatal_parse(pos: &Pos, msg: impl Into<String>) -> Error {
    Error::IncludeTimeFatalException(FatalOp::Parse, pos.clone(), msg.into())
}

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

pub fn emit_fatal_for_break_continue(pos: &Pos, level: usize) -> InstrSeq {
    let suffix = if level == 1 { "" } else { "s" };
    let msg = format!("Cannot break/continue {} level{}", level, suffix);
    emit_fatal_runtime(pos, msg)
}
