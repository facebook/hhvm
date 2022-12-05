// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use depgraph_api::DeclName;
use depgraph_api::DepGraphReader;
use depgraph_api::DepGraphWriter;
use depgraph_api::DependencyName;
use depgraph_api::Result;
use hash::DashMap;
use hash::HashSet;

#[derive(Debug, Clone, Default)]
pub struct DependencyGraph {
    // We store the dependency edges in "reverse dependency" fashion (rdeps)
    // e.g.
    // ```
    //   class A {}
    //   class B extends A {}
    // ```
    // would result in
    //   Extends(A) : {Type(B)}
    pub rdeps: DashMap<DependencyName, HashSet<DeclName>>,
}

impl DepGraphReader for DependencyGraph {
    fn get_dependents(
        &self,
        dependency: DependencyName,
    ) -> Box<dyn Iterator<Item = dep::Dep> + '_> {
        Box::new(
            self.rdeps
                .get(&dependency)
                .into_iter()
                .flat_map(|e| Vec::from_iter(e.value().iter().map(|n| n.to_dep()))),
        )
    }
}

impl DepGraphWriter for DependencyGraph {
    fn add_dependency(&self, dependent: DeclName, dependency: DependencyName) -> Result<()> {
        self.rdeps.entry(dependency).or_default().insert(dependent);
        Ok(())
    }
}
