// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::Serialize;

#[derive(Copy, Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub struct LocalId<'a>(isize, &'a str);

impl<'a> LocalId<'a> {
    pub fn new_unscoped(name: &'a str) -> Self {
        Self(0, name)
    }

    pub fn name(self) -> &'a str {
        self.1
    }
}

pub mod map {
    pub type Map<'a, T> = arena_collections::SortedAssocList<'a, super::LocalId<'a>, T>;
}

pub mod set {
    pub type Set<'a> = arena_collections::SortedSet<'a, super::LocalId<'a>>;
}
