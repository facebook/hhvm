// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io;
use std::io::Read;
use std::io::Write;

use depgraph::reader::Dep;
use hash::HashMap;
use hash::HashSet;

/// Structure to keep track of the dependency graph delta.
#[derive(Default, Debug, Clone, PartialEq, Eq)]
pub struct DepGraphDelta {
    /// Maps each dependency to a set of dependents
    rdeps: HashMap<Dep, HashSet<Dep>>,

    /// Total number of edges. Tracks the sum:
    /// `rdeps.values().map(|set|set.len()).sum()`
    num_edges: usize,
}

impl DepGraphDelta {
    pub fn insert(&mut self, dependent: Dep, dependency: Dep) {
        if (self.rdeps.entry(dependency))
            .or_default()
            .insert(dependent)
        {
            self.num_edges += 1;
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

    /// Return the number of edges in the dep graph delta.
    pub fn num_edges(&self) -> usize {
        self.num_edges
    }

    /// Write all edges in the delta to the writer in a custom format.
    ///
    /// The format is as follows. Each dependency hash can be followed by
    /// an arbitrary number of accompanying dependent hashes. To distinguish
    /// between dependency and dependent hashes, we make use of the fact that
    /// hashes are 63-bit (due to the OCaml limitation). We set the MSB for
    /// dependent hashes.
    pub fn write_to<W: Write>(&self, w: &mut W) -> io::Result<usize> {
        let mut edges_added = 0;
        for (&dependency, dependents) in self.rdeps.iter() {
            if dependents.is_empty() {
                continue;
            }

            let dependency: u64 = dependency.into();
            w.write_all(&dependency.to_be_bytes())?;
            for &dependent in dependents.iter() {
                let dependent: u64 = dependent.into();

                // Hashes are 63-bits, so we have one bit left to distinguish
                // between dependencies and dependents. Dependents have their
                // MSB set.
                let dependent = dependent | (1 << 63);
                w.write_all(&dependent.to_be_bytes())?;
                edges_added += 1;
            }
        }

        Ok(edges_added)
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
        r: &mut R,
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

            let hash = u64::from_be_bytes(bytes);
            if (hash & (1 << 63)) == 0 {
                // This is a dependency hash.
                dependency = Some(Dep::new(hash));
            } else {
                // This is a dependent hash.
                let hash = hash & !(1 << 63);
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
