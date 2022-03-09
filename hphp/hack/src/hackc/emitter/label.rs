// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// HHBC encodes bytecode offsets as i32 (HPHP::Offset) so u32
/// is plenty of range for label ids.
#[derive(Debug, Default, Clone, Copy, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[repr(transparent)]
pub struct Label(pub u32);

impl std::fmt::Display for Label {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

impl Label {
    pub const INVALID: Label = Label(u32::MAX);
}

#[derive(Default, Debug)]
pub struct Gen {
    next: Label,
}

impl Gen {
    pub fn next_regular(&mut self) -> Label {
        let curr = self.next;
        self.next.0 += 1;
        curr
    }

    pub fn reset(&mut self) {
        *self = Default::default();
    }
}
