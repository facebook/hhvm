// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_trait::Arena;

pub use crate::typing_continuations::{CMap, TypingContKey};
use crate::typing_env_types::LocalId;
use crate::typing_local_types::{Local, TypingLocalTypes};

// TODO(hrust): Should this really be copy? Or should we keep a map of pointers.
#[derive(Debug, Clone, Copy)]
pub struct PerContEntry<'a> {
    pub local_types: TypingLocalTypes<'a>,
    // TODO(hrust): fake_members
    // TODO(hrust): tpenv
}

impl<'a> PerContEntry<'a> {
    pub fn empty() -> Self {
        PerContEntry {
            local_types: TypingLocalTypes::empty(),
        }
    }
}

#[derive(Debug, Copy, Clone)]
pub struct TypingPerContEnv<'a>(CMap<'a, PerContEntry<'a>>);

impl<'a> TypingPerContEnv<'a> {
    pub fn initial_locals<A: Arena>(arena: &'a A, entry: PerContEntry<'a>) -> Self {
        let m = CMap::empty();
        let m = m.add(arena, TypingContKey::Next, entry);
        TypingPerContEnv(m)
    }

    pub fn get_cont_option(self, name: TypingContKey<'a>) -> Option<&'a PerContEntry<'a>> {
        self.0.find_ref(&name)
    }

    pub fn add_to_cont<A: Arena>(
        self,
        arena: &'a A,
        name: TypingContKey<'a>,
        key: LocalId<'a>,
        value: Local<'a>,
    ) -> Self {
        let m = self.0;
        let m = match m.find(&name) {
            None => m,
            Some(mut cont) => {
                cont.local_types = cont.local_types.add(arena, key, value);
                m.add(arena, name, cont)
            }
        };
        TypingPerContEnv(m)
    }
}
