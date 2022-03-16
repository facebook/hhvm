// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use dashmap::{mapref::entry::Entry, DashMap};
use hackrs::dependency_registrar::{DeclName, DependencyName, DependencyRegistrar, Result};
use std::collections::HashSet;

type DeclNameSet = HashSet<DeclName>;

#[derive(Debug, Clone)]
pub struct DependencyGraph {
    // We store the dependency edges in "reverse dependency" fashion (rdeps)
    // e.g.
    // ```
    //   class A {}
    //   class B extends A {}
    // ```
    // would result in
    //   Extends(A) : {Type(B)}
    rdeps: DashMap<DependencyName, DeclNameSet>,
}

impl DependencyGraph {
    pub fn new() -> Self {
        Self {
            rdeps: DashMap::new(),
        }
    }
}

impl DependencyRegistrar for DependencyGraph {
    fn add_dependency(&self, dependent: DeclName, dependency: DependencyName) -> Result<()> {
        match self.rdeps.entry(dependency) {
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
