// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::collections::HashSet;
use std::io::Write;
use std::sync::Mutex;

use dep_graph_delta::DepGraphDelta;
pub use depgraph::reader::Dep;
use depgraph::reader::DepGraph;
use depgraph::reader::DepGraphOpener;
use ocamlrep::FromError;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use ocamlrep::Value;
use ocamlrep_custom::caml_serialize_default_impls;
use ocamlrep_custom::CamlSerialize;
use once_cell::sync::OnceCell;
use rpds::HashTrieSet;

fn _static_assert() {
    // The use of 64-bit (actually 63-bit) dependency hashes requires that we
    // are compiling for a 64-bit architecture. Let's assert that at compile time.
    //
    // OCaml only supports unboxed integers of WORD SIZE - 1 bits. We don't want to
    // be boxing dependency hashes, so we require a 64-bit word size.
    //
    // If this check fails, it would be impossible to correctly convert back and
    // forth between OCaml's native integer type and Rust's u64.
    let _ = [(); 0 - (!(8 == std::mem::size_of::<usize>()) as usize)];
}

/// A structure wrapping the memory-mapped dependency graph.
/// Each worker will itself lazily (or eagerly upon request)
/// open a memory-mapping to the dependency graph.
///
/// It's an option, because custom mode might be enabled without
/// an existing saved-state.
static DEP_GRAPH: OnceCell<Option<UnsafeDepGraph>> = OnceCell::new();

/// The dependency graph delta.
///
/// Even though this is only used in a single-threaded context (from OCaml)
/// we wrap it in a `Mutex` to ensure safety.
static DEP_GRAPH_DELTA: OnceCell<Mutex<DepGraphDelta>> = OnceCell::new();

/// Which dependency graph format are we using?
#[derive(FromOcamlRep, ToOcamlRep)]
#[repr(C, u8)]
// CAUTION: This must be kept in sync with typing_deps_mode.ml
pub enum TypingDepsMode {
    /// Keep track of newly discovered edges in an in-memory delta.
    ///
    /// Optionally, the in-memory delta is backed by a pre-computed
    /// dependency graph stored using a custom file format.
    InMemoryMode(Option<String>),
    /// Mode that writes newly discovered edges to binary files on disk
    /// (one file per disk). Those binary files can then be post-processed
    /// using a tool of choice.
    ///
    /// The first parameter is (optionally) a path to an existing custom 64-bit
    /// dependency graph. If it is present, only new edges will be written,
    /// of not, all edges will be written.
    SaveToDiskMode {
        graph: Option<String>,
        new_edges_dir: String,
        human_readable_dep_map_dir: Option<String>,
    },
    /// Mode that keeps track of edges via hh_fanout's Rust API
    HhFanoutRustMode {
        hh_fanout: ocamlrep_custom::Custom<hh_fanout_rust_ffi::HhFanoutRustFfi>,
    },
}

/// A raw OCaml pointer to the dependency mode.
///
/// We use this raw pointer because we don't want to constantly
/// convert between the OCaml and Rust value (which involves copying)
/// when its not needed. Rather, we only convert when we first open the
/// dependency graph.
#[derive(Debug, Clone, Copy)]
pub struct RawTypingDepsMode(usize);

impl FromOcamlRep for RawTypingDepsMode {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(Self(value.to_bits()))
    }
}

impl RawTypingDepsMode {
    /// Convert the raw pointer into a Rust value
    ///
    /// # Safety
    ///
    /// Only safe if the OCaml pointer underlying `self` is still valid!
    /// You should not use this method if the OCaml runtime has had a change
    /// to run between obtaining `self` and calling this method!
    unsafe fn to_rust(self) -> Result<TypingDepsMode, FromError> {
        let value: Value<'_> = Value::from_bits(self.0);
        TypingDepsMode::from_ocamlrep(value)
    }
}

/// We wrap the dependency graph in an unsafe structure.
///
/// We need to do this, because we want to store both the
/// mmap and the dependency graph that references it in a
/// global variable.
pub struct UnsafeDepGraph {
    /// The opener contains the open mmap.
    _do_not_reference_opener: DepGraphOpener,
    /// The actual dependency graph references the opener above,
    /// as such we must make sure that the dependency graph
    /// does NOT outlive the opener.
    ///
    /// The lifetime on this is a LIE.
    _do_not_reference_depgraph: DepGraph<'static>,
}

impl UnsafeDepGraph {
    pub fn new(opener: DepGraphOpener) -> Result<Self, String> {
        let depgraph: DepGraph<'_> = opener.open()?;

        // Safety:
        //
        // We cast a bounded lifetime to a static lifetime. This is
        // of course a lie. However, using the API of UnsafeDepGraph,
        // we make sure that any reference to `depgraph` will not
        // outlive the opener.
        let depgraph: DepGraph<'static> = unsafe { std::mem::transmute(depgraph) };
        Ok(Self {
            _do_not_reference_opener: opener,
            _do_not_reference_depgraph: depgraph,
        })
    }

    /// Return a reference to the depgraph.
    ///
    /// The returned depgraph cannot outlive `self`.
    pub fn depgraph(&self) -> &DepGraph<'_> {
        &self._do_not_reference_depgraph
    }

    /// Load the graph using the given mode.
    ///
    /// # Safety
    ///
    /// The pointer to the dependency graph mode should still be pointing
    /// to a valid OCaml object.
    pub unsafe fn load(mode: RawTypingDepsMode) -> Result<Option<&'static UnsafeDepGraph>, String> {
        let depgraph = DEP_GRAPH.get_or_try_init::<_, String>(|| {
            let mode = mode.to_rust().unwrap();
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
                    let opener = DepGraphOpener::from_path(&depgraph_fn)
                        .map_err(|err| format!("could not open dep graph file: {:?}", err))?;
                    let depgraph = UnsafeDepGraph::new(opener)?;
                    Ok(Some(depgraph))
                }
                TypingDepsMode::HhFanoutRustMode { hh_fanout: _ } => {
                    todo!()
                }
            }
        })?;

        Ok(depgraph.as_ref())
    }

    /// Run the closure with the loaded dep graph. If the custom dep graph
    /// mode was enabled without a saved-state, return the passed default
    /// value.
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
    pub unsafe fn with_default<F, R>(mode: RawTypingDepsMode, default: R, f: F) -> R
    where
        for<'a> F: FnOnce(&'a DepGraph<'a>) -> R,
    {
        match Self::load(mode).unwrap() {
            None => default,
            Some(g) => f(g.depgraph()),
        }
    }

    /// Run the closure with the loaded dep graph. If the custom dep graph
    /// mode was enabled without a saved-state, the closure is run without
    /// a dep graph.
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
    pub unsafe fn with_option<F, R>(mode: RawTypingDepsMode, f: F) -> R
    where
        for<'a> F: FnOnce(Option<&'a DepGraph<'a>>) -> R,
    {
        let g = Self::load(mode).unwrap();
        f(g.map(|g| g.depgraph()))
    }
}

pub fn dep_graph_delta_with_cell<R>(f: impl FnOnce(&Mutex<DepGraphDelta>) -> R) -> R {
    let cell = DEP_GRAPH_DELTA.get_or_init(|| Mutex::new(DepGraphDelta::new()));
    f(cell)
}

/// Run the closure with the dep graph delta.
///
/// # Panics
///
/// When another reference to delta is still active, but that
/// isn't likely,given that we only have one thread, and the
/// `with`/`with_mut` auxiliary functions disallow the reference
/// to escape.
pub fn dep_graph_delta_with<R>(f: impl FnOnce(&DepGraphDelta) -> R) -> R {
    dep_graph_delta_with_cell(|cell| f(&cell.lock().unwrap()))
}

/// Run the closure with the mutable dep graph delta.
///
/// # Panics
///
/// See `with`
pub fn dep_graph_delta_with_mut<R>(f: impl FnOnce(&mut DepGraphDelta) -> R) -> R {
    dep_graph_delta_with_cell(|cell| f(&mut cell.lock().unwrap()))
}

/// Rust set of dependencies that can be transferred from
/// OCaml to Rust memory.
#[derive(Debug, Eq, PartialEq)]
pub struct DepSet(HashTrieSet<Dep>);

impl std::ops::Deref for DepSet {
    type Target = HashTrieSet<Dep>;

    fn deref(&self) -> &HashTrieSet<Dep> {
        &self.0
    }
}

impl From<HashTrieSet<Dep>> for DepSet {
    fn from(x: HashTrieSet<Dep>) -> Self {
        Self(x)
    }
}

impl CamlSerialize for DepSet {
    caml_serialize_default_impls!();

    fn serialize(&self) -> Vec<u8> {
        let num_elems = self.size();
        let mut buf = Vec::with_capacity(std::mem::size_of::<u64>() * num_elems);
        for &x in self.iter() {
            let x: u64 = x.into();
            buf.write_all(&x.to_le_bytes()).unwrap();
        }
        buf
    }

    fn deserialize(data: &[u8]) -> Self {
        const U64_SIZE: usize = std::mem::size_of::<u64>();

        let num_elems = data.len() / U64_SIZE;
        let max_index = num_elems * U64_SIZE;
        let mut s: HashTrieSet<Dep> = HashTrieSet::new();
        let mut index = 0;
        while index < max_index {
            let x = u64::from_le_bytes(data[index..index + U64_SIZE].try_into().unwrap());
            s.insert_mut(Dep::new(x));
            index += U64_SIZE;
        }
        s.into()
    }
}

impl DepSet {
    /// Returns the union of two sets.
    ///
    /// The underlying data structure does not implement union. So let's
    /// implement it here.
    pub fn union(&self, other: &Self) -> Self {
        // `HashTrieSet`'s insert is O(1) on average, O(n) worst-case, so let's
        // make sure we loop over the smaller set.
        //
        // Note that the sizes of the arguments are expected to be
        // very skewed.
        let (bigger, smaller) = if self.size() > other.size() {
            (self, other)
        } else {
            (other, self)
        };
        let mut bigger = bigger.0.clone();
        for dep in smaller.iter() {
            bigger.insert_mut(*dep);
        }
        bigger.into()
    }

    /// Returns the intersection of two sets.
    ///
    /// The underlying data structure does not implement intersection. So let's
    /// implement it here.
    pub fn intersect(&self, other: &Self) -> Self {
        // Let's make sure we loop over the smaller set.
        let (bigger, smaller) = if self.size() > other.size() {
            (self, other)
        } else {
            (other, self)
        };
        let mut result = HashTrieSet::new();
        for dep in smaller.iter() {
            if bigger.contains(dep) {
                result.insert_mut(*dep);
            }
        }
        result.into()
    }

    /// Returns the difference of two sets, i.e. all elements in the first
    /// set but not in the second set.
    ///
    /// The underlying data structure does not implement intersection. So let's
    /// implement it here.
    pub fn difference(&self, other: &Self) -> Self {
        let mut result = self.0.clone();
        // Let's make sure we loop over the smaller set.
        if self.size() < other.size() {
            for dep in self.iter() {
                if other.contains(dep) {
                    result.remove_mut(dep);
                }
            }
        } else {
            for dep in other.iter() {
                result.remove_mut(dep);
            }
        }
        result.into()
    }
}

/// Rust set of visited hashes
#[derive(Debug)]
pub struct VisitedSet(RefCell<HashSet<Dep>>);

impl std::ops::Deref for VisitedSet {
    type Target = RefCell<HashSet<Dep>>;

    fn deref(&self) -> &RefCell<HashSet<Dep>> {
        &self.0
    }
}

impl From<RefCell<HashSet<Dep>>> for VisitedSet {
    fn from(x: RefCell<HashSet<Dep>>) -> Self {
        Self(x)
    }
}

impl CamlSerialize for VisitedSet {
    caml_serialize_default_impls!();
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_dep_set_serialize() {
        let mut x: HashTrieSet<Dep> = HashTrieSet::new();
        x.insert_mut(Dep::new(1));
        x.insert_mut(Dep::new(2));
        let x: DepSet = x.into();

        let buf = x.serialize();
        let y = DepSet::deserialize(&buf);

        assert_eq!(x, y);
    }

    #[test]
    fn test_dep_set_union() {
        let s = |x: &[u64]| DepSet::from(HashTrieSet::from_iter(x.iter().copied().map(Dep::new)));

        assert_eq!(s(&[4, 7]).union(&s(&[1, 4, 3])), s(&[1, 4, 3, 7]));
        assert_eq!(s(&[1, 4, 3]).union(&s(&[4, 7])), s(&[1, 4, 3, 7]));
    }

    #[test]
    fn test_dep_set_inter() {
        let s = |x: &[u64]| DepSet::from(HashTrieSet::from_iter(x.iter().copied().map(Dep::new)));

        assert_eq!(s(&[4, 7]).intersect(&s(&[1, 4, 3])), s(&[4]));
        assert_eq!(s(&[1, 4, 3]).intersect(&s(&[4, 7])), s(&[4]));
    }

    #[test]
    fn test_dep_set_diff() {
        let s = |x: &[u64]| DepSet::from(HashTrieSet::from_iter(x.iter().copied().map(Dep::new)));

        assert_eq!(s(&[4, 7]).difference(&s(&[1, 4, 3, 9, 8, 10])), s(&[7]));
        assert_eq!(
            s(&[1, 4, 3, 9, 8, 10]).difference(&s(&[4, 11])),
            s(&[1, 3, 9, 8, 10])
        );
    }
}
