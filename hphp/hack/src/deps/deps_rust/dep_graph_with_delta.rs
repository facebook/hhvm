// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use dep_graph_delta::DepGraphDelta;
use dep_graph_delta::HashSetDelta;
use depgraph_reader::Dep;
use depgraph_reader::DepGraph;
use itertools::Either;

pub struct DepGraphWithDelta<'a> {
    base: Option<&'a DepGraph>,
    delta: &'a DepGraphDelta,
}

impl<'a> DepGraphWithDelta<'a> {
    pub fn new(base: Option<&'a DepGraph>, delta: &'a DepGraphDelta) -> Self {
        Self { base, delta }
    }

    /// Iterates over all dependents of a dependency, with possibly duplicate dependents.
    pub fn iter_dependents_with_duplicates<F, R>(&self, dep: Dep, mut f: F) -> R
    where
        F: FnMut(&mut dyn Iterator<Item = Dep>) -> R,
    {
        let Self { base, delta } = self;
        let HashSetDelta { added, removed } = delta.get(dep);
        let mut added_iter = added
            .map(|s| s.iter().copied())
            .map_or(Either::Right(std::iter::empty::<Dep>()), Either::Left);
        let is_removed = |d: &Dep| match removed {
            None => false,
            Some(removed) => removed.contains(d),
        };
        match base {
            None => f(&mut added_iter),
            Some(base) => {
                let hashes = base.hash_list_for(dep);
                match hashes {
                    None => f(&mut added_iter),
                    Some(hashes) => {
                        let base_iter = base.hash_list_hashes(hashes);
                        f(&mut added_iter.chain(base_iter.filter(|d| !is_removed(d))))
                    }
                }
            }
        }
    }

    /// Returns true if we know for sure that the depgraph has the edge, false
    /// if we don't know.
    pub fn has_edge_for_sure(&self, dependent: Dep, dependency: Dep) -> bool {
        let Self { base, delta } = self;
        base.map_or(false, |g| {
            g.dependent_dependency_edge_exists(dependent, dependency)
        }) && !delta.edge_is_removed(dependent, dependency)
    }
}
