// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;

#[derive(Debug, Clone)]
pub struct Id(usize);

impl fmt::Display for Id {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Id({})", self.0)
    }
}

#[derive(Debug, Clone)]
pub struct Iter {
    next: Id,
    count: usize,
}
impl Iter {
    pub fn count(&self) -> usize {
        self.count
    }

    pub fn get(&mut self) -> Id {
        let curr = self.next.0;
        self.next.0 = curr + 1;
        self.count = if self.count > self.next.0 {
            self.count
        } else {
            self.next.0
        };
        Id(curr)
    }

    pub fn free(&mut self) {
        self.next.0 -= 1;
    }

    pub fn reset(&mut self) {
        *self = Self::default();
    }
}
impl Default for Iter {
    fn default() -> Self {
        Iter {
            next: Id(0),
            count: 0,
        }
    }
}
