// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_pos_rust::emit_pos;
use env::emitter::Emitter;
use hhbc_ast_rust::FatalOp;
use instruction_sequence_rust::Instr;
use oxidized::pos::Pos;

#[derive(Debug)]
pub enum Error {
    IncludeTimeFatalException(FatalOp, String),
}

pub fn raise_fatal_runtime(pos: &Pos, msg: String) -> Error {
    Error::IncludeTimeFatalException(FatalOp::Runtime, format!("{:?}: {}", pos, msg))
}

pub fn raise_fatal_parse(pos: &Pos, msg: String) -> Error {
    Error::IncludeTimeFatalException(FatalOp::Parse, format!("{:?}: {}", pos, msg))
}

pub fn emit_fatal(emitter: &Emitter, op: FatalOp, pos: &Pos, msg: String) -> Instr {
    Instr::gather(vec![
        emit_pos(emitter, pos),
        Instr::make_instr_string(&msg),
        Instr::make_instr_fatal(op),
    ])
}

pub fn emit_fatal_runtime(emitter: &Emitter, pos: &Pos, msg: String) -> Instr {
    emit_fatal(emitter, FatalOp::Runtime, pos, msg)
}

pub fn emit_fatal_runtimeomitframe(emitter: &Emitter, pos: &Pos, msg: String) -> Instr {
    emit_fatal(emitter, FatalOp::RuntimeOmitFrame, pos, msg)
}

pub fn emit_fatal_for_break_continue(emitter: &Emitter, pos: &Pos, level: usize) -> Instr {
    let suffix = if level == 1 { "" } else { "s" };
    let msg = format!("Cannot break/continue {} level{}", level, suffix);
    emit_fatal_runtime(emitter, pos, msg)
}
