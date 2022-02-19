// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// HHBC encodes bytecode offsets as i32 (HPHP::Offset) so u32
/// is plenty of range for label ids.
#[derive(Debug, Default, Clone, Copy, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[repr(transparent)]
pub struct LabelId(pub u32);

impl std::fmt::Display for LabelId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, std::cmp::Ord, std::cmp::PartialOrd)]
#[repr(C)]
pub enum Label {
    Regular(LabelId),
    DefaultArg(LabelId),
}
impl Label {
    pub fn id(&self) -> LabelId {
        match self {
            Label::Regular(id) => *id,
            Label::DefaultArg(id) => *id,
        }
    }

    pub fn map<F: FnOnce(&LabelId) -> LabelId>(&self, f: F) -> Self {
        match self {
            Label::Regular(id) => Label::Regular(f(id)),
            Label::DefaultArg(id) => Label::DefaultArg(f(id)),
        }
    }
}

#[derive(Default, Debug)]
pub struct Gen {
    next_id: LabelId,
}

impl Gen {
    pub fn next_regular(&mut self) -> Label {
        Label::Regular(self.next())
    }

    pub fn next_default_arg(&mut self) -> Label {
        Label::DefaultArg(self.next())
    }

    fn next(&mut self) -> LabelId {
        let curr_id = self.next_id;
        self.next_id.0 += 1;
        curr_id
    }

    pub fn reset(&mut self) {
        *self = Default::default();
    }
}
