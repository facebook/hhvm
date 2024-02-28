// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc::IterId;
use hhbc::Local;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;

#[derive(Debug, Clone)]
enum IterKind {
    Iter,
    LIter(Local),
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

    fn gen(&mut self, kind: IterKind) -> IterId {
        self.iters.push(kind);
        self.count = std::cmp::max(self.count, self.iters.len() as u32);
        IterId {
            idx: (self.iters.len() - 1) as u32,
        }
    }

    pub fn gen_iter(&mut self) -> IterId {
        self.gen(IterKind::Iter)
    }

    pub fn gen_liter(&mut self, loc: Local) -> IterId {
        self.gen(IterKind::LIter(loc))
    }

    pub fn free<'a>(&mut self, count: usize) -> InstrSeq<'a> {
        let total = self.iters.len();
        assert!(count <= total);
        InstrSeq::gather(
            self.iters
                .drain(total - count..)
                .rev()
                .enumerate()
                .map(|(i, it)| {
                    let id = IterId {
                        idx: (total - i - 1) as u32,
                    };
                    match it {
                        IterKind::Iter => instr::iter_free(id),
                        IterKind::LIter(loc) => instr::l_iter_free(id, loc),
                    }
                })
                .collect(),
        )
    }

    pub fn reset(&mut self) {
        *self = Self::default();
    }
}
