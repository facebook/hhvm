// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::aast;
use oxidized::pos::Pos;

#[derive(Debug)]
pub struct StatementState<'arena> {
    pub verify_return: Option<aast::Hint>,
    pub default_return_value: InstrSeq<'arena>,
    pub default_dropthrough: Option<InstrSeq<'arena>>,
    pub verify_out: InstrSeq<'arena>,
    pub function_pos: Pos,
    pub num_out: usize,
}

impl<'arena> StatementState<'arena> {
    pub fn init() -> Self {
        StatementState {
            verify_return: None,
            default_return_value: instr::null(),
            default_dropthrough: None,
            verify_out: instr::empty(),
            num_out: 0,
            function_pos: Pos::NONE,
        }
    }
}
