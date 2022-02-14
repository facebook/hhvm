// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;

#[derive(Default, Debug, Clone, Copy, PartialEq, Eq)]
#[repr(C)]
pub struct IterId {
    /// 0-based index into HHBC stack frame iterators
    pub idx: u32,
}

impl fmt::Display for IterId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.idx)
    }
}

#[derive(Default, Debug, Clone)]
pub struct Iter {
    pub next: IterId,
    count: u32,
}

impl Iter {
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
