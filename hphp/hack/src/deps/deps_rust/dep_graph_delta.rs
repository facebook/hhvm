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

/// Structure to keep track of the dependency graph delta.
#[derive(Default, Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct DepGraphDelta {
    /// Maps each dependency to a set of dependents
    rdeps: HashMap<Dep, HashSet<Dep>>,

    /// Total number of edges. Tracks the sum:
    /// `rdeps.values().map(|set|set.len()).sum()`
    num_edges: usize,
}

impl DepGraphDelta {
    // The high bit of a value being set distinguishes dependency from dependent.
    const DEPENDENCY_TAG: u64 = 1 << 63;

    pub fn insert(&mut self, dependent: Dep, dependency: Dep) {
        if (self.rdeps.entry(dependency))
            .or_default()
            .insert(dependent)
        {
            self.num_edges += 1;
        }
    }

    pub fn extend(&mut self, other: Self) {
        use std::collections::hash_map::Entry::*;
        for (dependency, dependents) in other.rdeps {
            match self.rdeps.entry(dependency) {
                Occupied(e) => {
                    let ds = e.into_mut();
                    let n = ds.len();
                    ds.extend(dependents);
                    self.num_edges += ds.len() - n;
                }
                Vacant(e) => {
                    self.num_edges += dependents.len();
                    e.insert(dependents);
                }
            }
        }
    }

    pub fn get(&self, dependency: Dep) -> Option<&HashSet<Dep>> {
        self.rdeps.get(&dependency)
    }

    /// Return an iterator over this dependency graph delta.
    ///
    /// Iterates over (dependent, dependency) pairs
    pub fn iter(&self) -> impl Iterator<Item = (Dep, Dep)> + '_ {
        self.rdeps.iter().flat_map(|(&dependency, dependents_set)| {
            dependents_set
                .iter()
                .map(move |&dependent| (dependent, dependency))
        })
    }

    pub fn into_rdeps(self) -> impl Iterator<Item = (Dep, HashSet<Dep>)> {
        self.rdeps.into_iter()
    }

    /// Return the number of edges in the dep graph delta.
    pub fn len(&self) -> usize {
        self.num_edges
    }

    pub fn is_empty(&self) -> bool {
        self.num_edges == 0
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
        for (&dependency, dependents) in self.rdeps.iter() {
            Self::write_list(&mut w, dependency, dependents.iter().copied())?;
            edges_added += dependents.len();
        }

        Ok(edges_added)
    }

    /// Write all edges in the delta to the writer in a custom format.
    ///
    /// The output is deterministic sorted order.
    pub fn write_sorted<W: Write>(&self, mut w: W) -> io::Result<()> {
        let mut dependencies: Vec<_> = self.rdeps.iter().collect();
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
                    self.insert(dependent, dependency);
                    edges_read += 1;
                }
            }
        }

        Ok(edges_read)
    }

    pub fn clear(&mut self) {
        self.rdeps.clear();
        self.num_edges = 0;
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
                .position(|&x| (x & DepGraphDelta::DEPENDENCY_TAG) != 0)
                .unwrap_or(rest.len());

            // Advance the iterator to the next edge list (if any).
            let (dependents, rest) = rest.split_at(end);
            self.raw_data = rest;

            debug_assert_ne!(first & DepGraphDelta::DEPENDENCY_TAG, 0);
            let dependency = Dep::new(first & !DepGraphDelta::DEPENDENCY_TAG);
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
    fn test_dep_graph_delta_serialize_empty() {
        let x = DepGraphDelta::default();
        let mut bytes = Vec::new();
        x.write_to(&mut bytes).unwrap();

        let mut y = DepGraphDelta::default();
        let mut bytes_read: &[u8] = &bytes;
        let num_loaded = y.read_from(&mut bytes_read, |_, _| true).unwrap();

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
        x.write_to(&mut bytes).unwrap();

        let mut y = DepGraphDelta::default();
        let mut bytes_read: &[u8] = &bytes;
        let num_loaded = y.read_from(&mut bytes_read, |_, _| true).unwrap();

        assert_eq!(num_loaded, 4);
        assert_eq!(x, y);
    }

    #[test]
    fn test_dep_graph_delta_iter_empty() {
        let x = DepGraphDelta::default();
        let v: Vec<_> = x.iter().collect();
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
        let mut v: Vec<_> = x.iter().collect();
        v.sort();
        assert_eq!(v, edges);
    }
}
