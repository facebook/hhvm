// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io;
use std::io::Read;
use std::io::Write;

use dep::Dep;
use hash::HashMap;
use hash::HashSet;
use serde::Deserialize;
use serde::Serialize;

#[derive(Clone, Debug, Default, Eq, PartialEq, Serialize, Deserialize)]
struct Graph {
    map: HashMap<Dep, HashSet<Dep>>,

    /// Total number of edges. Tracks the sum:
    /// `map.values().map(|set|set.len()).sum()`
    num_edges: usize,
}

impl Graph {
    // The high bit of a value being set distinguishes dependency from dependent.
    const DEPENDENCY_TAG: u64 = 1 << 63;

    fn insert(&mut self, key: Dep, value: Dep) {
        if self.map.entry(key).or_default().insert(value) {
            self.num_edges += 1;
        }
    }

    fn remove(&mut self, key: Dep, value: Dep) {
        let mut removed = false;
        let mut is_empty = false;
        self.map.entry(key).and_modify(|values| {
            removed = values.remove(&value);
            is_empty = values.is_empty();
        });
        if removed {
            self.num_edges -= 1;
        }
        if is_empty {
            self.map.remove(&key);
        }
    }

    fn extend(&mut self, other: &Self) {
        for (key, other_values) in &other.map {
            let values = self.map.entry(*key).or_default();
            for value in other_values {
                if values.insert(*value) {
                    self.num_edges += 1;
                }
            }
        }
    }

    fn remove_many(&mut self, other: &Self) {
        for (key, other_values) in &other.map {
            let values = self.map.entry(*key).or_default();
            for value in other_values {
                if values.remove(value) {
                    self.num_edges -= 1;
                }
            }
            if values.is_empty() {
                self.map.remove(key);
            }
        }
    }

    fn get(&self, key: &Dep) -> Option<&HashSet<Dep>> {
        self.map.get(key)
    }

    fn len(&self) -> usize {
        self.num_edges
    }

    pub fn is_empty(&self) -> bool {
        self.num_edges == 0
    }

    fn clear(&mut self) {
        self.map.clear();
        self.num_edges = 0;
    }

    /// Return an iterator over this graph
    fn iter(&self) -> impl Iterator<Item = (Dep, Dep)> + '_ {
        self.map.iter().flat_map(|(&dependency, dependents_set)| {
            dependents_set
                .iter()
                .map(move |&dependent| (dependent, dependency))
        })
    }

    fn into_iter(self) -> impl Iterator<Item = (Dep, HashSet<Dep>)> {
        self.map.into_iter()
    }

    /// Write one (dependency, dependents) list.
    ///
    /// The format is as follows. Each dependency hash can be followed by
    /// an arbitrary number of accompanying dependent hashes. To distinguish
    /// between dependency and dependent hashes, we make use of the fact that
    /// hashes are 63-bit (due to the OCaml limitation). We set the MSB for
    /// dependency hashes.
    fn write_list<W: Write>(
        mut w: W,
        dependency: Dep,
        dependents: impl Iterator<Item = Dep> + ExactSizeIterator,
    ) -> io::Result<()> {
        if dependents.len() != 0 {
            let dependency: u64 = dependency.into();
            w.write_all(&(dependency | Self::DEPENDENCY_TAG).to_ne_bytes())?;

            for dependent in dependents {
                let dependent: u64 = dependent.into();
                w.write_all(&dependent.to_ne_bytes())?;
            }
        }

        Ok(())
    }

    /// Write all edges in the delta to the writer in a custom format.
    ///
    /// The output is deterministic if the insertion order is deterministic,
    /// but is arbitrary since we're iterating HashMap & HashSet.
    pub fn write_to<W: Write>(&self, mut w: W) -> io::Result<usize> {
        let mut edges_added = 0;
        for (&dependency, dependents) in self.map.iter() {
            Self::write_list(&mut w, dependency, dependents.iter().copied())?;
            edges_added += dependents.len();
        }

        Ok(edges_added)
    }

    /// Write all edges in the delta to the writer in a custom format.
    ///
    /// The output is deterministic sorted order.
    pub fn write_sorted<W: Write>(&self, mut w: W) -> io::Result<()> {
        let mut dependencies: Vec<_> = self.map.iter().collect();
        dependencies.sort_unstable_by_key(|(dep, _)| *dep);
        for (&dependency, dependents) in dependencies {
            let mut dependents: Vec<Dep> = dependents.iter().copied().collect();
            dependents.sort_unstable();
            Self::write_list(&mut w, dependency, dependents.into_iter())?;
        }
        Ok(())
    }

    /// Load all edges into the delta.
    ///
    /// The predicate determines whether or not to add a loaded edge to the delta.
    /// If the predicate returns true for a given dependent-dependency edge
    /// (in that order), the edge is added.
    ///
    /// Returns the number of edges actually read.
    ///
    /// See write_to() for details about the file format.
    pub fn read_from<R: Read>(
        &mut self,
        mut r: R,
        f: impl Fn(Dep, Dep) -> bool,
    ) -> io::Result<usize> {
        let mut edges_read = 0;
        let mut dependency: Option<Dep> = None;
        loop {
            let mut bytes: [u8; 8] = [0; 8];
            match r.read_exact(&mut bytes) {
                Err(err) if err.kind() == io::ErrorKind::UnexpectedEof => {
                    break;
                }
                r => r?,
            };

            let hash = u64::from_ne_bytes(bytes);
            if (hash & Self::DEPENDENCY_TAG) != 0 {
                // This is a dependency hash.
                let hash = hash & !Self::DEPENDENCY_TAG;
                dependency = Some(Dep::new(hash));
            } else {
                // This is a dependent hash.
                let dependent = Dep::new(hash);
                let dependency =
                    dependency.expect("Expected a dependent hash before a dependency hash");

                if f(dependent, dependency) {
                    self.insert(dependency, dependent);
                    edges_read += 1;
                }
            }
        }

        Ok(edges_read)
    }
}

pub struct HashSetDelta<'a, T> {
    pub added: Option<&'a HashSet<T>>,
    pub removed: Option<&'a HashSet<T>>,
}

/// Structure to keep track of the dependency graph delta.
///
/// Assuming this delta is applied to a graph with edge set B (for base),
/// and A is the set of added edges and R the set of removed edges,
/// then the resulting set of edges will be
///
///    B + A - R
///
/// with invariant A /\ R = 0 (A and R are disjoint).
#[derive(Default, Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct DepGraphDelta {
    /// Maps each dependency to a set of dependents
    added_edges: Graph,

    /// Edges to be removed from the base dependency graph.
    /// Invariant: `removed_edges` and `added_edges` are disjoint.
    removed_edges: Graph,
}

impl DepGraphDelta {
    pub fn insert(&mut self, dependent: Dep, dependency: Dep) {
        self.added_edges.insert(dependency, dependent);
        self.removed_edges.remove(dependency, dependent);
    }

    pub fn remove(&mut self, dependent: Dep, dependency: Dep) {
        self.removed_edges.insert(dependency, dependent);
        self.added_edges.remove(dependency, dependent);
    }

    pub fn extend(&mut self, mut other: Self) {
        self.added_edges.remove_many(&other.removed_edges);
        other.added_edges.remove_many(&self.removed_edges);
        self.added_edges.extend(&other.added_edges);
        self.removed_edges.extend(&other.removed_edges);
    }

    pub fn get<'a>(&'a self, dependency: Dep) -> HashSetDelta<'a, Dep> {
        HashSetDelta {
            added: self.added_edges.get(&dependency),
            removed: self.removed_edges.get(&dependency),
        }
    }

    /// Return the number of added edges in the dep graph delta. Ignore removed edges.
    pub fn added_edges_count(&self) -> usize {
        self.added_edges.len()
    }

    pub fn is_empty(&self) -> bool {
        self.added_edges.is_empty() && self.removed_edges.is_empty()
    }

    /// Return an iterator over this dependency graph delta.
    ///
    /// Iterates over (dependent, dependency) pairs
    pub fn iter_added_edges(&self) -> impl Iterator<Item = (Dep, Dep)> + '_ {
        self.added_edges.iter()
    }

    pub fn into_iter_added_edges(self) -> impl Iterator<Item = (Dep, HashSet<Dep>)> {
        self.added_edges.into_iter()
    }

    /// Write all edges in the delta to the writer in a custom format.
    ///
    /// The output is deterministic if the insertion order is deterministic,
    /// but is arbitrary since we're iterating HashMap & HashSet.
    pub fn write_added_edges_to<W: Write>(&self, w: W) -> io::Result<usize> {
        self.added_edges.write_to(w)
    }

    /// Write all edges in the delta to the writer in a custom format.
    ///
    /// The output is deterministic sorted order.
    pub fn write_sorted_added_edges<W: Write>(&self, w: W) -> io::Result<()> {
        self.added_edges.write_sorted(w)
    }

    /// Load all edges into the delta.
    ///
    /// The predicate determines whether or not to add a loaded edge to the delta.
    /// If the predicate returns true for a given dependent-dependency edge
    /// (in that order), the edge is added.
    ///
    /// Returns the number of edges actually read.
    ///
    /// See write_to() for details about the file format.
    pub fn read_added_edges_from<R: Read>(
        &mut self,
        r: R,
        f: impl Fn(Dep, Dep) -> bool,
    ) -> io::Result<usize> {
        self.added_edges.read_from(r, f)
    }

    pub fn clear(&mut self) {
        self.added_edges.clear();
        self.removed_edges.clear();
    }
}

/// An iterator that yields a sequence of (dependency, dependents) pairs
/// from a DepGraphDelta file.
///
/// It does so in a zero-copy way, simply pointing into the DepGraphDelta
/// representation (e.g. a memory-mapped file).
pub struct DepGraphDeltaIterator<'a> {
    raw_data: &'a [u64],
}

impl<'a> DepGraphDeltaIterator<'a> {
    pub fn new(raw_data: &'a [u64]) -> Self {
        Self { raw_data }
    }
}

impl<'a> Iterator for DepGraphDeltaIterator<'a> {
    type Item = (Dep, &'a [Dep]);

    fn next(&mut self) -> Option<Self::Item> {
        self.raw_data.split_first().map(|(&first, rest)| {
            // Find the next dependency hash, which is indicated by the high
            // bit being set. Everything in between is a dependent, and we can
            // just point to them directly.
            let end = rest
                .iter()
                .position(|&x| (x & Graph::DEPENDENCY_TAG) != 0)
                .unwrap_or(rest.len());

            // Advance the iterator to the next edge list (if any).
            let (dependents, rest) = rest.split_at(end);
            self.raw_data = rest;

            debug_assert_ne!(first & Graph::DEPENDENCY_TAG, 0);
            let dependency = Dep::new(first & !Graph::DEPENDENCY_TAG);
            let dependents = Dep::from_u64_slice(dependents);
            (dependency, dependents)
        })
    }
}

impl<'a> std::iter::FusedIterator for DepGraphDeltaIterator<'a> {}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_insert_remove() {
        let mut delta = DepGraphDelta::default();
        delta.insert(Dep::new(10), Dep::new(1));
        delta.insert(Dep::new(20), Dep::new(1));
        delta.remove(Dep::new(10), Dep::new(1));

        delta.insert(Dep::new(30), Dep::new(3));
        delta.remove(Dep::new(30), Dep::new(3));

        let HashSetDelta { added, removed } = delta.get(Dep::new(1));
        assert_set_equals(added, [Dep::new(20)]);
        assert_set_equals(removed, [Dep::new(10)]);

        let HashSetDelta { added, removed } = delta.get(Dep::new(3));
        assert!(is_empty(added));
        assert_set_equals(removed, [Dep::new(30)]);
        assert_eq!(delta.added_edges_count(), 1);

        // Re-add removed edge
        delta.insert(Dep::new(30), Dep::new(3));
        let HashSetDelta { added, removed } = delta.get(Dep::new(3));
        assert_set_equals(added, [Dep::new(30)]);
        assert!(is_empty(removed));
        assert_eq!(delta.added_edges_count(), 2);
    }

    #[test]
    fn test_extend() {
        let mut delta1 = DepGraphDelta::default();
        delta1.insert(Dep::new(1), Dep::new(10));
        delta1.insert(Dep::new(1), Dep::new(20));
        delta1.insert(Dep::new(3), Dep::new(30));
        delta1.remove(Dep::new(2), Dep::new(10));
        delta1.remove(Dep::new(5), Dep::new(10));

        let mut delta2 = DepGraphDelta::default();
        delta2.insert(Dep::new(2), Dep::new(10));
        delta2.insert(Dep::new(1), Dep::new(20));
        delta2.insert(Dep::new(4), Dep::new(30));
        delta2.remove(Dep::new(3), Dep::new(30));
        delta2.remove(Dep::new(1), Dep::new(10));
        delta1.remove(Dep::new(5), Dep::new(10));

        delta1.extend(delta2);

        let mut expected = DepGraphDelta::default();
        expected.insert(Dep::new(1), Dep::new(20));
        expected.insert(Dep::new(4), Dep::new(30));
        expected.remove(Dep::new(2), Dep::new(10));
        expected.remove(Dep::new(3), Dep::new(30));
        expected.remove(Dep::new(1), Dep::new(10));
        expected.remove(Dep::new(5), Dep::new(10));

        assert_eq!(delta1, expected);
        assert_eq!(delta1.added_edges_count(), 2);
    }

    fn is_empty<T>(set: Option<&HashSet<T>>) -> bool {
        set.is_none() || set.is_some_and(|set| set.is_empty())
    }

    fn assert_set_equals<const N: usize>(actual: Option<&HashSet<Dep>>, expected: [Dep; N]) {
        let mut expected_set = HashSet::default();
        for d in expected {
            expected_set.insert(d);
        }
        assert_eq!(actual, Some(&expected_set))
    }

    #[test]
    fn test_dep_graph_delta_serialize_empty() {
        let x = DepGraphDelta::default();
        let mut bytes = Vec::new();
        x.write_added_edges_to(&mut bytes).unwrap();

        let mut y = DepGraphDelta::default();
        let mut bytes_read: &[u8] = &bytes;
        let num_loaded = y
            .read_added_edges_from(&mut bytes_read, |_, _| true)
            .unwrap();

        assert_eq!(num_loaded, 0);
        assert_eq!(x, y);
    }

    #[test]
    fn test_dep_graph_delta_serialize_non_empty() {
        let mut x = DepGraphDelta::default();
        x.insert(Dep::new(10), Dep::new(1));
        x.insert(Dep::new(10), Dep::new(2));
        x.insert(Dep::new(11), Dep::new(2));
        x.insert(Dep::new(12), Dep::new(3));
        let mut bytes = Vec::new();
        x.write_added_edges_to(&mut bytes).unwrap();

        let mut y = DepGraphDelta::default();
        let mut bytes_read: &[u8] = &bytes;
        let num_loaded = y
            .read_added_edges_from(&mut bytes_read, |_, _| true)
            .unwrap();

        assert_eq!(num_loaded, 4);
        assert_eq!(x, y);
    }

    #[test]
    fn test_dep_graph_delta_iter_empty() {
        let x = DepGraphDelta::default();
        let v: Vec<_> = x.iter_added_edges().collect();
        assert_eq!(v.len(), 0);
    }

    #[test]
    fn test_dep_graph_delta_iter_non_empty() {
        let mut x = DepGraphDelta::default();
        let edges = vec![
            (Dep::new(10), Dep::new(1)),
            (Dep::new(10), Dep::new(2)),
            (Dep::new(11), Dep::new(2)),
            (Dep::new(12), Dep::new(3)),
        ];
        for (dependency, dependent) in edges.iter() {
            x.insert(*dependency, *dependent)
        }
        let mut v: Vec<_> = x.iter_added_edges().collect();
        v.sort();
        assert_eq!(v, edges);
    }
}
