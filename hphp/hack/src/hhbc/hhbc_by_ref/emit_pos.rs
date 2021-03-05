// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_instruction_sequence::{instr, InstrSeq};
use oxidized::pos::Pos;
use std::convert::TryInto;

pub fn emit_pos<'a>(alloc: &'a bumpalo::Bump, pos: &Pos) -> InstrSeq<'a> {
    if !pos.is_none() {
        let (line_begin, line_end, col_begin, col_end) = pos.info_pos_extended();
        instr::srcloc(
            alloc,
            line_begin.try_into().unwrap(),
            line_end.try_into().unwrap(),
            col_begin.try_into().unwrap(),
            col_end.try_into().unwrap(),
        )
    } else {
        instr::empty(alloc)
    }
}

pub fn emit_pos_then<'a>(
    alloc: &'a bumpalo::Bump,
    pos: &Pos,
    instrs: InstrSeq<'a>,
) -> InstrSeq<'a> {
    InstrSeq::gather(alloc, vec![emit_pos(alloc, pos), instrs])
}
