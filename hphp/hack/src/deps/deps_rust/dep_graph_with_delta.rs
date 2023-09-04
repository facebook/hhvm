// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use dep_graph_delta::DepGraphDelta;
use dep_graph_delta::HashSetDelta;
use depgraph_reader::BaseDepgraphTrait;
use depgraph_reader::Dep;
use itertools::Either;

pub struct DepGraphWithDelta<'a, B> {
    base: Option<&'a B>,
    delta: &'a DepGraphDelta,
}

impl<'a, B: BaseDepgraphTrait> DepGraphWithDelta<'a, B> {
    pub fn new(base: Option<&'a B>, delta: &'a DepGraphDelta) -> Self {
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
                let base_iter = base.iter_dependents(dep);
                f(&mut added_iter.chain(base_iter.filter(|d| !is_removed(d))))
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

#[cfg(test)]
mod tests {
    use std::collections::HashMap;
    use std::collections::HashSet;

    use super::*;

    pub struct SimpleDepGraph {
        graph: HashMap<Dep, HashSet<Dep>>,
    }

    impl<const N: usize> From<[(Dep, HashSet<Dep>); N]> for SimpleDepGraph {
        fn from(arr: [(Dep, HashSet<Dep>); N]) -> Self {
            Self {
                graph: arr.into_iter().collect(),
            }
        }
    }

    impl BaseDepgraphTrait for SimpleDepGraph {
        fn iter_dependents(&self, dep: Dep) -> Box<dyn Iterator<Item = Dep> + '_> {
            Box::new(
                self.graph
                    .get(&dep)
                    .map_or(Either::Left(std::iter::empty()), |set| {
                        Either::Right(set.iter().copied())
                    }),
            )
        }

        fn dependent_dependency_edge_exists(&self, dependent: Dep, dependency: Dep) -> bool {
            self.graph
                .get(&dependency)
                .map_or(false, |set| set.contains(&dependent))
        }
    }

    #[test]
    fn test_some_base() {
        let base = SimpleDepGraph::from([
            (Dep::new(0), HashSet::from([Dep::new(1), Dep::new(2)])),
            (Dep::new(3), HashSet::from([Dep::new(2)])),
        ]);
        let mut delta = DepGraphDelta::default();
        delta.insert(Dep::new(4), Dep::new(3));
        delta.insert(Dep::new(3), Dep::new(0));
        delta.remove(Dep::new(2), Dep::new(0));
        let dg = DepGraphWithDelta::new(Some(&base), &delta);

        assert_eq!(
            dg.iter_dependents_with_duplicates(Dep::new(0), |iter| iter.collect::<HashSet<_>>()),
            HashSet::from([Dep::new(1), Dep::new(3)])
        );
        assert_eq!(
            dg.iter_dependents_with_duplicates(Dep::new(3), |iter| iter.collect::<HashSet<_>>()),
            HashSet::from([Dep::new(2), Dep::new(4)])
        );
        assert!(!dg.has_edge_for_sure(Dep::new(2), Dep::new(0)));
        assert!(dg.has_edge_for_sure(Dep::new(2), Dep::new(3)));
    }

    #[test]
    fn test_no_base() {
        let mut delta = DepGraphDelta::default();
        delta.insert(Dep::new(4), Dep::new(3));
        delta.insert(Dep::new(3), Dep::new(0));
        delta.remove(Dep::new(2), Dep::new(0));
        let dg = DepGraphWithDelta::<SimpleDepGraph>::new(None, &delta);

        assert_eq!(
            dg.iter_dependents_with_duplicates(Dep::new(0), |iter| iter.collect::<HashSet<_>>()),
            HashSet::from([Dep::new(3)])
        );
        assert_eq!(
            dg.iter_dependents_with_duplicates(Dep::new(3), |iter| iter.collect::<HashSet<_>>()),
            HashSet::from([Dep::new(4)])
        );
        assert!(!dg.has_edge_for_sure(Dep::new(2), Dep::new(0)));
        assert!(!dg.has_edge_for_sure(Dep::new(2), Dep::new(3)));
    }
}
