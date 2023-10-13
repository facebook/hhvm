// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::VecDeque;
use std::ffi::OsString;
use std::fs::File;
use std::sync::OnceLock;

use dep_graph_delta::DepGraphDelta;
use dep_graph_delta::HashSetDelta;
use depgraph_reader::BaseDepgraphTrait;
use depgraph_reader::Dep;
use depgraph_reader::DepGraph;
use hash::HashSet;
use itertools::Either;
use parking_lot::MappedRwLockReadGuard;
use parking_lot::MappedRwLockWriteGuard;
use parking_lot::RwLock;
use parking_lot::RwLockReadGuard;
use parking_lot::RwLockWriteGuard;
use rpds::HashTrieSet;
use typing_deps_hash::DepType;

use crate::RawTypingDepsMode;
use crate::TypingDepsMode;

pub struct DepGraphWithDelta<B: BaseDepgraphTrait> {
    /// A structure wrapping the memory-mapped dependency graph.
    /// Each worker will itself lazily (or eagerly upon request)
    /// open a memory-mapping to the dependency graph.
    ///
    /// It's an option, because custom mode might be enabled without
    /// an existing saved-state.
    base: Option<B>,
    /// The dependency graph delta.
    ///
    /// Even though this is only used in a single-threaded context (from OCaml)
    /// we wrap it in a `Mutex` to ensure safety.
    delta: DepGraphDelta,
}

impl<B: BaseDepgraphTrait> Default for DepGraphWithDelta<B> {
    fn default() -> Self {
        Self::new(None, DepGraphDelta::default())
    }
}

impl<B: BaseDepgraphTrait> DepGraphWithDelta<B> {
    pub fn new(base: Option<B>, delta: DepGraphDelta) -> Self {
        Self { base, delta }
    }

    pub fn delta(&self) -> &DepGraphDelta {
        &self.delta
    }

    pub fn delta_mut(&mut self) -> &mut DepGraphDelta {
        &mut self.delta
    }

    pub fn base(&self) -> Option<&B> {
        self.base.as_ref()
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
        let is_removed = |d: &Dep| removed.is_some_and(|removed| removed.contains(d));
        match base {
            None => f(&mut added_iter),
            Some(base) => {
                let base_iter = base.iter_dependents(dep);
                f(&mut added_iter.chain(base_iter.filter(|d| !is_removed(d))))
            }
        }
    }

    pub fn has_edge(&self, dependent: Dep, dependency: Dep) -> bool {
        let Self { base, delta } = self;
        let HashSetDelta { added, removed } = delta.get(dependency);
        added.is_some_and(|added| added.contains(&dependent))
            || (base.as_ref().map_or(false, |g| {
                g.dependent_dependency_edge_exists(dependent, dependency)
            }) && !removed.is_some_and(|removed| removed.contains(&dependent)))
    }

    /// Returns the recursive 'extends' dependents of a dep.
    /// Does not include the dep itself.
    pub fn get_extend_deps(
        &self,
        visited: &mut HashSet<Dep>,
        source_class: Dep,
        acc: &mut HashTrieSet<Dep>,
    ) {
        let mut queue = VecDeque::new();
        self.get_extend_deps_visit(visited, &mut queue, source_class, acc);
        while let Some(source_class) = queue.pop_front() {
            self.get_extend_deps_visit(visited, &mut queue, source_class, acc);
        }
    }

    /// Helper function to recursively get extend deps
    fn get_extend_deps_visit(
        &self,
        visited: &mut HashSet<Dep>,
        queue: &mut VecDeque<Dep>,
        source_class: Dep,
        acc: &mut HashTrieSet<Dep>,
    ) {
        if !visited.insert(source_class) {
            return;
        }
        if source_class.is_class() {
            let extends_hash = source_class.class_to_extends().unwrap();
            self.iter_dependents_with_duplicates(extends_hash, |iter| {
                iter.for_each(|dep: Dep| {
                    if dep.is_class() {
                        if !acc.contains(&dep) {
                            acc.insert_mut(dep);
                            queue.push_back(dep);
                        }
                    }
                })
            });
            let require_extends_hash = source_class.class_to_require_extends();
            self.iter_dependents_with_duplicates(require_extends_hash, |iter| {
                iter.for_each(|dep: Dep| {
                    if !acc.contains(&dep) {
                        acc.insert_mut(dep);
                        queue.push_back(dep);
                    }
                })
            });
        }
    }

    /// Returns the union of the provided dep set and their recursive 'extends' dependents.
    pub fn add_extend_deps(&self, acc: &mut HashTrieSet<Dep>) {
        let mut visited = HashSet::default();
        let mut queue = VecDeque::new();
        for source_class in acc.iter() {
            queue.push_back(*source_class);
        }
        while let Some(source_class) = queue.pop_front() {
            self.get_extend_deps_visit(&mut visited, &mut queue, source_class, acc);
        }
    }

    /// The fanout of a member `m` in type `A` contains:
    /// - the members `m` in descendants of `A` down to the first members `m` which are declared.
    /// - the dependents of those members `m` in descendants,
    ///   but excluding dependents of declared members.
    ///
    /// We also include `A::m` itself in the result.
    /// The computed fanout is added to the provided fanout accumulator.
    pub fn get_member_fanout(
        &self,
        class_dep: Dep,
        member_type: DepType,
        member_name: &str,
        fanout_acc: &mut HashTrieSet<Dep>,
    ) {
        let mut queue = VecDeque::new();
        let mut visited = HashSet::default();

        self.visit_class_dep_for_member_fanout(
            (class_dep, false),
            member_type,
            member_name,
            &mut visited,
            &mut queue,
            fanout_acc,
            false,
        );

        while let Some(class_dep) = queue.pop_front() {
            self.visit_class_dep_for_member_fanout(
                class_dep,
                member_type,
                member_name,
                &mut visited,
                &mut queue,
                fanout_acc,
                true,
            );
        }
    }

    fn visit_class_dep_for_member_fanout(
        &self,
        (class_dep, via_req): (Dep, bool),
        member_type: DepType,
        member_name: &str,
        visited: &mut HashSet<(Dep, bool)>,
        queue: &mut VecDeque<(Dep, bool)>,
        fanout_acc: &mut HashTrieSet<Dep>,
        stop_if_declared: bool,
    ) {
        if !visited.insert((class_dep, via_req)) {
            return;
        }
        if !via_req {
            fanout_acc.insert_mut(class_dep);
        }
        let member_dep = class_dep.member(member_type, member_name);
        let mut member_is_declared = false;
        let mut member_deps = HashSet::default();
        self.iter_dependents_with_duplicates(member_dep, |deps| {
            for dep in deps {
                if dep.is_declares() {
                    member_is_declared = true;
                    if stop_if_declared {
                        break;
                    }
                } else {
                    member_deps.insert(dep);
                }
            }
        });
        if !(stop_if_declared && member_is_declared) {
            member_deps
                .into_iter()
                .for_each(|dep| fanout_acc.insert_mut(dep));
            let extends_dep_for_class = class_dep.class_to_extends().unwrap();
            self.iter_dependents_with_duplicates(extends_dep_for_class, |iter| {
                iter.for_each(|dep| queue.push_back((dep, via_req)));
            });
            let requires_extends_dep_for_class = class_dep.class_to_require_extends();
            self.iter_dependents_with_duplicates(requires_extends_dep_for_class, |iter| {
                iter.for_each(|dep| queue.push_back((dep, true)));
            });
        }
    }

    pub fn load_delta(&mut self, source: OsString) -> usize {
        let f = File::open(source).unwrap();
        let mut r = std::io::BufReader::new(f);

        let Self { base, delta } = self;
        let result = match base {
            Some(base) => {
                delta.read_added_edges_from(&mut r, |dependent, dependency| {
                    // Only add when it's not already in
                    // the graph!
                    !base.dependent_dependency_edge_exists(dependent, dependency)
                })
            }
            None => delta.read_added_edges_from(&mut r, |_, _| true),
        };
        result.unwrap()
    }

    pub fn remove(&mut self, dependent: Dep, dependency: Dep) {
        self.delta.remove(dependent, dependency)
    }
}

impl DepGraphWithDelta<DepGraph> {
    /// Load the graph using the given mode.
    ///
    /// The mode is only used on the first call, to establish some global state, and
    /// then ignored for future calls.
    fn load_base(&mut self, mode: RawTypingDepsMode) -> Result<(), String> {
        if self.base.is_none() {
            self.base = Self::base_dep_graph_from_mode(mode)?;
        }

        Ok(())
    }

    /// Override the loaded dep graph.
    ///
    /// # Panics
    ///
    /// Panics if the graph is not loaded, and custom mode was not enabled.
    ///
    /// Panics if the graph is not yet loaded, and opening
    /// the graph results in an error.
    ///
    /// # Safety
    ///
    /// The pointer to the dependency graph mode should still be pointing
    /// to a valid OCaml object.
    pub fn replace_dep_graph(&mut self, mode: RawTypingDepsMode) -> Result<(), String> {
        self.base = Self::base_dep_graph_from_mode(mode)?;
        Ok(())
    }

    fn base_dep_graph_from_mode(mode: RawTypingDepsMode) -> Result<Option<DepGraph>, String> {
        // # Safety
        //
        // The pointer to the dependency graph mode should still be pointing
        // to a valid OCaml object.
        let mode = unsafe { mode.to_rust().unwrap() };

        match mode {
            TypingDepsMode::InMemoryMode(None)
            | TypingDepsMode::SaveToDiskMode {
                graph: None,
                new_edges_dir: _,
                human_readable_dep_map_dir: _,
            } => {
                // Enabled, but we don't have a saved-state, so we can't open it
                Ok(None)
            }
            TypingDepsMode::InMemoryMode(Some(depgraph_fn))
            | TypingDepsMode::SaveToDiskMode {
                graph: Some(depgraph_fn),
                new_edges_dir: _,
                human_readable_dep_map_dir: _,
            } => {
                // We are opening and intializing the dep graph while holding onto the mutex...
                // Which typically isn't great, but since ocaml is single threaded, it's ok.
                let depgraph = DepGraph::from_path(depgraph_fn)
                    .map_err(|err| format!("could not open dep graph file: {:?}", err))?;
                Ok(Some(depgraph))
            }
            TypingDepsMode::HhFanoutRustMode { hh_fanout: _ } => {
                // HhFanoutRustMode doesn't load the dep graph this way.
                // This path shouldn't be reached.
                unimplemented!()
            }
        }
    }
}

pub struct LockedDepgraphWithDelta(RwLock<OnceLock<DepGraphWithDelta<DepGraph>>>);

impl LockedDepgraphWithDelta {
    pub const fn new() -> Self {
        Self(RwLock::new(OnceLock::new()))
    }

    fn read_init(&self) -> MappedRwLockReadGuard<'_, DepGraphWithDelta<DepGraph>> {
        RwLockReadGuard::map(self.0.read(), |c| c.get_or_init(DepGraphWithDelta::default))
    }

    fn write_init(&self) -> MappedRwLockWriteGuard<'_, DepGraphWithDelta<DepGraph>> {
        RwLockWriteGuard::map(self.0.write(), |c| {
            c.get_or_init(DepGraphWithDelta::default);
            c.get_mut().unwrap()
        })
    }

    /// Locks the depgraph delta for reading, which also maintains a lock on the depgraph
    ///
    /// # Panics
    ///
    /// When another reference to delta is still active, but that
    /// isn't likely, given that we only have one thread, and the
    /// `with`/`with_mut` auxiliary functions disallow the reference
    /// to escape.
    pub fn read_delta(&self) -> MappedRwLockReadGuard<'_, DepGraphDelta> {
        MappedRwLockReadGuard::map(self.read_init(), |g| g.delta())
    }

    /// Locks the depgraph delta for writing, which also maintains a lock on the depgraph
    pub fn write_delta(&self) -> MappedRwLockWriteGuard<'_, DepGraphDelta> {
        MappedRwLockWriteGuard::map(self.write_init(), |g| g.delta_mut())
    }

    /// Locks the depgraph for reading.
    ///
    /// The mode is only used on the first call, to establish some global state, and
    /// then ignored for future calls.
    ///
    /// # Safety
    ///
    /// The pointer to the dependency graph mode should still be pointing
    /// to a valid OCaml object.
    pub fn read(
        &self,
        mode: RawTypingDepsMode,
    ) -> MappedRwLockReadGuard<'_, DepGraphWithDelta<DepGraph>> {
        self.load_base(mode).unwrap();
        self.read_init()
    }

    /// Locks the depgraph for writing.
    ///
    /// The mode is only used on the first call, to establish some global state, and
    /// then ignored for future calls.
    ///
    /// # Safety
    ///
    /// The pointer to the dependency graph mode should still be pointing
    /// to a valid OCaml object.
    pub fn write(
        &self,
        mode: RawTypingDepsMode,
    ) -> MappedRwLockWriteGuard<'_, DepGraphWithDelta<DepGraph>> {
        self.load_base(mode).unwrap();
        self.write_init()
    }

    /// Load the graph using the given mode.
    ///
    /// The mode is only used on the first call, to establish some global state, and
    /// then ignored for future calls.
    ///
    /// # Safety
    ///
    /// The pointer to the dependency graph mode should still be pointing
    /// to a valid OCaml object.
    fn load_base(&self, mode: RawTypingDepsMode) -> Result<(), String> {
        self.write_init().load_base(mode)
    }
}

#[cfg(test)]
mod tests {
    use std::collections::HashMap;
    use std::collections::HashSet;

    use itertools::Itertools;

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
        delta.insert(Dep::new(4), Dep::new(0));
        delta.remove(Dep::new(2), Dep::new(0));
        delta.remove(Dep::new(4), Dep::new(0));
        let dg = DepGraphWithDelta::new(Some(base), delta);

        assert_eq!(
            dg.iter_dependents_with_duplicates(Dep::new(0), |iter| iter.collect::<HashSet<_>>()),
            HashSet::from([Dep::new(1), Dep::new(3)])
        );
        assert_eq!(
            dg.iter_dependents_with_duplicates(Dep::new(3), |iter| iter.collect::<HashSet<_>>()),
            HashSet::from([Dep::new(2), Dep::new(4)])
        );
        assert!(dg.has_edge(Dep::new(1), Dep::new(0)));
        assert!(!dg.has_edge(Dep::new(2), Dep::new(0)));
        assert!(dg.has_edge(Dep::new(2), Dep::new(3)));
        assert!(dg.has_edge(Dep::new(4), Dep::new(3)));
        assert!(dg.has_edge(Dep::new(3), Dep::new(0)));
        assert!(!dg.has_edge(Dep::new(4), Dep::new(0)));
        assert!(!dg.has_edge(Dep::new(10), Dep::new(11)));
    }

    #[test]
    fn test_no_base() {
        let mut delta = DepGraphDelta::default();
        delta.insert(Dep::new(4), Dep::new(3));
        delta.insert(Dep::new(3), Dep::new(0));
        delta.remove(Dep::new(2), Dep::new(0));
        let dg = DepGraphWithDelta::<SimpleDepGraph>::new(None, delta);

        assert_eq!(
            dg.iter_dependents_with_duplicates(Dep::new(0), |iter| iter.collect::<HashSet<_>>()),
            HashSet::from([Dep::new(3)])
        );
        assert_eq!(
            dg.iter_dependents_with_duplicates(Dep::new(3), |iter| iter.collect::<HashSet<_>>()),
            HashSet::from([Dep::new(4)])
        );
        assert!(dg.has_edge(Dep::new(4), Dep::new(3)));
        assert!(dg.has_edge(Dep::new(3), Dep::new(0)));
        assert!(!dg.has_edge(Dep::new(2), Dep::new(0)));
        assert!(!dg.has_edge(Dep::new(2), Dep::new(3)));
    }

    #[test]
    fn test_extends_deps() {
        fn class_dep(n: u64) -> Dep {
            Dep::new((n << 1) + 1)
        }
        let class_a = class_dep(0);
        let class_b = class_dep(1);
        let class_c = class_dep(2);
        let class_d = class_dep(3);
        let class_e = class_dep(4);
        let class_f = class_dep(5);
        let class_k = class_dep(10);
        let class_l = class_dep(11);
        let class_m = class_dep(12);

        let dg = DepGraphWithDelta::new(
            Some(SimpleDepGraph::from([
                // 'Extends' edges
                (
                    class_a.class_to_extends().unwrap(),
                    HashSet::from([class_b, class_c]),
                ),
                (
                    class_b.class_to_extends().unwrap(),
                    HashSet::from([class_d]),
                ),
                (
                    class_c.class_to_extends().unwrap(),
                    HashSet::from([class_e, class_f]),
                ),
                // Regular edges
                (class_a, HashSet::from([class_k])),
                (class_c, HashSet::from([class_l, class_m])),
            ])),
            DepGraphDelta::default(),
        );
        let mut visited = hash::HashSet::<Dep>::default();
        let mut acc = HashTrieSet::new();
        dg.get_extend_deps(&mut visited, class_a, &mut acc);
        assert_eq!(
            HashSet::from_iter(acc.iter().copied()),
            HashSet::from([class_b, class_c, class_d, class_e, class_f])
        );
        assert_eq!(
            HashSet::from_iter(visited.iter().copied()),
            HashSet::from([class_a, class_b, class_c, class_d, class_e, class_f])
        );
    }

    #[test]
    fn test_add_extends_deps() {
        fn class_dep(n: u64) -> Dep {
            Dep::new((n << 1) + 1)
        }
        let class_a = class_dep(0);
        let class_b = class_dep(1);
        let class_c = class_dep(2);
        let class_d = class_dep(3);
        let class_e = class_dep(4);
        let class_f = class_dep(5);
        let class_g = class_dep(6);
        let class_h = class_dep(7);
        let class_i = class_dep(8);
        let class_j = class_dep(9);
        let class_k = class_dep(10);
        let class_l = class_dep(11);
        let class_m = class_dep(12);
        let class_n = class_dep(13);
        let class_o = class_dep(14);
        let class_p = class_dep(15);
        let class_q = class_dep(16);
        let class_r = class_dep(17);

        let dg = DepGraphWithDelta::new(
            Some(SimpleDepGraph::from([
                // 'Extends' edges
                (
                    class_a.class_to_extends().unwrap(),
                    HashSet::from([class_b, class_c]),
                ),
                (
                    class_b.class_to_extends().unwrap(),
                    HashSet::from([class_d]),
                ),
                (
                    class_c.class_to_extends().unwrap(),
                    HashSet::from([class_e, class_f]),
                ),
                (
                    class_f.class_to_extends().unwrap(),
                    HashSet::from([class_g, class_h]),
                ),
                (
                    class_i.class_to_extends().unwrap(),
                    HashSet::from([class_j, class_f]),
                ),
                (
                    class_p.class_to_extends().unwrap(),
                    HashSet::from([class_q, class_r]),
                ),
                (
                    class_r.class_to_extends().unwrap(),
                    HashSet::from([class_c, class_i]),
                ),
                // Regular edges
                (class_a, HashSet::from([class_k])),
                (class_c, HashSet::from([class_l, class_m])),
                (class_i, HashSet::from([class_n])),
                (class_j, HashSet::from([class_o])),
            ])),
            DepGraphDelta::default(),
        );
        let mut acc = HashTrieSet::new().insert(class_a).insert(class_i);
        dg.add_extend_deps(&mut acc);
        assert_eq!(
            HashSet::from_iter(acc.iter().copied()),
            HashSet::from([
                class_a, class_b, class_c, class_d, class_e, class_f, class_g, class_h, class_i,
                class_j
            ])
        );
    }

    #[test]
    fn test_member_fanout() {
        fn class_dep(n: u64) -> Dep {
            Dep::new((n << 1) + 1)
        }
        let class_a = class_dep(0);
        let class_b = class_dep(1);
        let class_c = class_dep(2);
        let class_d = class_dep(3);
        let class_e = class_dep(4);
        let class_f = class_dep(5);
        let class_g = class_dep(6);
        let class_h = class_dep(7);
        let class_i = class_dep(8);
        let class_j = class_dep(9);
        let a_m = class_a.member(DepType::Method, "m");
        let b_m = class_b.member(DepType::Method, "m");
        let c_m = class_c.member(DepType::Method, "m");
        let d_m = class_d.member(DepType::Method, "m");
        let e_m = class_e.member(DepType::Method, "m");
        let f_m = class_f.member(DepType::Method, "m");
        let g_m = class_g.member(DepType::Method, "m");
        let h_m = class_h.member(DepType::Method, "m");
        let d0 = Dep::new(100);
        let d1 = Dep::new(101);
        let d2 = Dep::new(102);
        let d3 = Dep::new(103);
        let d4 = Dep::new(104);
        let d5 = Dep::new(105);
        let d6 = Dep::new(106);
        let d7 = Dep::new(107);
        let d8 = Dep::new(108);
        let d9 = Dep::new(109);
        let d10 = Dep::new(110);
        let d11 = Dep::new(111);
        let d12 = Dep::new(112);
        let d13 = Dep::new(113);
        let d14 = Dep::new(114);
        let d15 = Dep::new(115);
        let d16 = Dep::new(116);
        let d17 = Dep::new(117);
        let d18 = Dep::new(118);
        let d19 = Dep::new(119);
        let d20 = Dep::new(120);

        let dg = DepGraphWithDelta::new(
            Some(SimpleDepGraph::from([
                // 'Extends' edges
                (
                    class_a.class_to_extends().unwrap(),
                    HashSet::from([class_b]),
                ),
                (
                    class_b.class_to_extends().unwrap(),
                    HashSet::from([class_c, class_f, class_j]),
                ),
                (
                    class_c.class_to_extends().unwrap(),
                    HashSet::from([class_d, class_g, class_i]),
                ),
                (
                    class_d.class_to_extends().unwrap(),
                    HashSet::from([class_e]),
                ),
                (
                    class_g.class_to_extends().unwrap(),
                    HashSet::from([class_h]),
                ),
                (
                    class_i.class_to_extends().unwrap(),
                    HashSet::from([class_f]), // makes the graph a DAG instead of just a tree
                ),
                // Let's introduce a cycle
                (
                    class_j.class_to_extends().unwrap(),
                    HashSet::from([class_b]),
                ),
                // member deps
                (a_m, HashSet::from([d0])),
                (b_m, HashSet::from([Dep::declares(), d1])),
                (c_m, HashSet::from([d2])),
                (d_m, HashSet::from([d3, Dep::declares(), d8])),
                (e_m, HashSet::from([d4, d10])),
                (f_m, HashSet::from([d5, Dep::declares()])),
                (g_m, HashSet::from([d6, d11])),
                (h_m, HashSet::from([d7, Dep::declares(), d9])),
                // I::m and J::m does not appear in the graph
                // Other class deps
                (class_a, HashSet::from([d12])),
                (class_b, HashSet::from([d13])),
                (class_c, HashSet::from([d14])),
                (class_d, HashSet::from([d15])),
                (class_e, HashSet::from([d16])),
                (class_f, HashSet::from([d17])),
                (class_g, HashSet::from([d18])),
                (class_h, HashSet::from([d19])),
                (class_i, HashSet::from([d20])),
            ])),
            DepGraphDelta::default(),
        );
        let mut fanout = HashTrieSet::new();
        dg.get_member_fanout(class_b, DepType::Method, "m", &mut fanout);
        assert_eq!(
            fanout.into_iter().copied().sorted().collect::<Vec<_>>(),
            [
                class_b, class_c, class_d, class_f, class_g, class_h, class_i, class_j, d1, d2, d6,
                d11
            ]
            .into_iter()
            .sorted()
            .collect::<Vec<_>>()
        );
    }
}
