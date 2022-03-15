// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use dashmap::{mapref::entry::Entry, DashMap};
use hackrs::dependency_registrar::{DeclName, DependencyName, DependencyRegistrar, Result};
use std::collections::HashSet;

type DeclNameSet = HashSet<DeclName>;

#[derive(Debug, Clone)]
pub struct DependencyMap {
    // e.g. Extends(Bar) <- {Type(Foo), ...}
    deps: DashMap<DependencyName, DeclNameSet>,
}

impl DependencyMap {
    #[allow(dead_code)] // TODO: remove on first use
    pub fn new() -> Self {
        Self {
            deps: DashMap::new(),
        }
    }
}

impl DependencyRegistrar for DependencyMap {
    fn add_dependency(&mut self, dependency: DependencyName, dependent: DeclName) -> Result<()> {
        match self.deps.entry(dependency) {
            Entry::Vacant(e) => {
                let mut dependents: DeclNameSet = DeclNameSet::new();
                dependents.insert(dependent);
                e.insert(dependents);
            }
            Entry::Occupied(mut e) => {
                e.get_mut().insert(dependent);
            }
        }
        Ok(())
    }
}
