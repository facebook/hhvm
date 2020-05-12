// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_trait::Arena;
use oxidized::ToOxidized;
use std::collections::{BTreeMap, BTreeSet};
use typing_collections_rust::Map;

pub use crate::typing_continuations::TypingContKey;
use crate::typing_env_types::LocalId;
use crate::typing_local_types::{Local, TypingLocalTypes};

// TODO(hrust): Should this really be copy? Or should we keep a map of pointers.
#[derive(Debug, Clone, Copy)]
pub struct PerContEntry<'a> {
    pub local_types: TypingLocalTypes<'a>,
    // TODO(hrust): fake_members
    // TODO(hrust): tpenv
}

impl<'a> ToOxidized for PerContEntry<'a> {
    type Target = oxidized::typing_per_cont_env::PerContEntry;

    fn to_oxidized(&self) -> Self::Target {
        let PerContEntry { local_types } = self;
        oxidized::typing_per_cont_env::PerContEntry {
            local_types: local_types.to_oxidized(),
            fake_members: oxidized::typing_fake_members::TypingFakeMembers::Valid(BTreeSet::new()),
            tpenv: oxidized::type_parameter_env::TypeParameterEnv {
                tparams: BTreeMap::new(),
                consistent: true,
            },
        }
    }
}

impl<'a> PerContEntry<'a> {
    pub fn empty() -> Self {
        PerContEntry {
            local_types: TypingLocalTypes::empty(),
        }
    }

    pub fn with_local_types<A: Arena>(
        &self,
        arena: &'a A,
        local_types: TypingLocalTypes<'a>,
    ) -> &'a Self {
        let mut entry = self.clone();
        entry.local_types = local_types;
        arena.alloc(entry)
    }
}

#[derive(Debug, Copy, Clone)]
pub struct TypingPerContEnv<'a>(Map<'a, TypingContKey<'a>, &'a PerContEntry<'a>>);

impl<'a> ToOxidized for TypingPerContEnv<'a> {
    type Target = oxidized::typing_per_cont_env::TypingPerContEnv;

    fn to_oxidized(&self) -> Self::Target {
        let TypingPerContEnv(map) = self;
        map.to_oxidized()
    }
}

impl<'a> TypingPerContEnv<'a> {
    pub fn initial_locals<A: Arena>(arena: &'a A, entry: PerContEntry<'a>) -> Self {
        let m = Map::empty();
        let entry: &'a PerContEntry<'a> = arena.alloc(entry);
        let m = m.add(arena, TypingContKey::Next, entry);
        TypingPerContEnv(m)
    }

    pub fn get_cont_option(self, name: TypingContKey<'a>) -> Option<&'a PerContEntry<'a>> {
        self.0.find(&name)
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
            Some(cont) => {
                let value = arena.alloc(value);
                let local_types = cont.local_types.add(arena, key, value);
                let cont = cont.with_local_types(arena, local_types);
                m.add(arena, name, cont)
            }
        };
        TypingPerContEnv(m)
    }
}
