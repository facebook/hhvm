// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc_ast::IterId;

#[derive(Default, Debug, Clone)]
pub struct IterGen {
    pub next: IterId,
    count: u32,
}

impl IterGen {
    pub fn count(&self) -> usize {
        self.count as usize
    }

    pub fn get(&mut self) -> IterId {
        let curr = self.next;
        self.next.idx += 1;
        self.count = std::cmp::max(self.count, self.next.idx);
        curr
    }

    pub fn free(&mut self) {
        self.next.idx -= 1;
    }

    pub fn reset(&mut self) {
        *self = Self::default();
    }
}
