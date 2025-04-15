// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc::IterId;
use instruction_sequence::InstrSeq;
use instruction_sequence::instr;

#[derive(Debug, Clone)]
enum IterKind {
    Iter,
}

#[derive(Default, Debug, Clone)]
pub struct IterGen {
    iters: Vec<IterKind>,
    count: u32,
}

impl IterGen {
    pub fn count(&self) -> usize {
        self.count as usize
    }

    pub fn next(&self) -> u32 {
        self.iters.len() as u32
    }

    pub fn gen_iter(&mut self) -> IterId {
        self.iters.push(IterKind::Iter);
        self.count = std::cmp::max(self.count, self.iters.len() as u32);
        IterId::new(self.iters.len() - 1)
    }

    pub fn free(&mut self, count: usize) -> InstrSeq {
        let total = self.iters.len();
        assert!(count <= total);
        InstrSeq::gather(
            self.iters
                .drain(total - count..)
                .rev()
                .enumerate()
                .map(|(i, _)| {
                    let id = IterId::new(total - i - 1);
                    instr::iter_free(id)
                })
                .collect(),
        )
    }

    pub fn reset(&mut self) {
        *self = Self::default();
    }
}
