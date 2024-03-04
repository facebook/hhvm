// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::pos::Pos;

pub fn emit_pos(pos: &Pos) -> InstrSeq {
    if !pos.is_none() {
        let (line_begin, line_end, col_begin, col_end) = pos.info_pos_extended();
        instr::srcloc(
            line_begin.try_into().unwrap(),
            line_end.try_into().unwrap(),
            col_begin.try_into().unwrap(),
            col_end.try_into().unwrap(),
        )
    } else {
        instr::empty()
    }
}

pub fn emit_pos_then(pos: &Pos, instrs: InstrSeq) -> InstrSeq {
    InstrSeq::gather(vec![emit_pos(pos), instrs])
}
