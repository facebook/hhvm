// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc::Label;

#[derive(Debug)]
pub struct LabelGen {
    next: Label,
}

impl LabelGen {
    pub fn new() -> Self {
        Self { next: Label::ZERO }
    }

    pub fn next_regular(&mut self) -> Label {
        let curr = self.next;
        self.next.0 += 1;
        curr
    }

    pub fn reset(&mut self) {
        *self = Self::new();
    }
}
