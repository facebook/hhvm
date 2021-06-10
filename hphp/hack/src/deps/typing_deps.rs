// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg_attr(use_unstable_features, feature(test))]

use depgraph::reader::{Dep, DepGraph, DepGraphOpener};
use im_rc::OrdSet;
use ocamlrep::{FromError, FromOcamlRep, Value};
use ocamlrep_custom::{caml_serialize_default_impls, CamlSerialize, Custom};
use ocamlrep_ocamlpool::ocaml_ffi;
use once_cell::sync::OnceCell;
use oxidized::typing_deps_mode::{HashMode, TypingDepsMode};
use std::cell::RefCell;
use std::collections::{BTreeMap, BTreeSet, VecDeque};
use std::convert::TryInto;
use std::ffi::OsString;
use std::io;
use std::io::{Read, Write};
use std::panic;
use std::sync::Mutex;
use typing_deps_hash::{hash1, hash2, DepType};

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
static DEPGRAPH: OnceCell<Option<UnsafeDepGraph>> = OnceCell::new();

/// The dependency graph delta.
///
/// Even though this is only used in a single-threaded context (from OCaml)
/// we wrap it in a `Mutex` to ensure safety.
static DEPGRAPH_DELTA: OnceCell<Mutex<DepGraphDelta>> = OnceCell::new();

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
    ///
    /// Explicit lifetimes for clarity.
    #[allow(clippy::needless_lifetimes)]
    pub fn depgraph<'a>(&'a self) -> &'a DepGraph<'a> {
        &self._do_not_reference_depgraph
    }

    /// Load the graph using the given mode.
    ///
    /// # Safety
    ///
    /// The pointer to the dependency graph mode should still be pointing
    /// to a valid OCaml object.
    pub unsafe fn load(mode: RawTypingDepsMode) -> Result<Option<&'static UnsafeDepGraph>, String> {
        let depgraph = DEPGRAPH.get_or_try_init::<_, String>(|| {
            let mode = mode.to_rust().unwrap();
            match mode {
                TypingDepsMode::SQLiteMode => {
                    panic!("programming error: cannot call load in SQLite mode")
                }
                TypingDepsMode::CustomMode(None)
                | TypingDepsMode::SaveCustomMode {
                    graph: None,
                    new_edges_dir: _,
                } => {
                    // Enabled, but we don't have a saved-state, so we can't open it
                    Ok(None)
                }
                TypingDepsMode::CustomMode(Some(depgraph_fn))
                | TypingDepsMode::SaveCustomMode {
                    graph: Some(depgraph_fn),
                    new_edges_dir: _,
                } => {
                    let opener = DepGraphOpener::from_path(&depgraph_fn)
                        .map_err(|err| format!("could not open dep graph file: {:?}", err))?;
                    let depgraph = UnsafeDepGraph::new(opener)?;
                    Ok(Some(depgraph))
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

/// Structure to keep track of the dependency graph delta.
///
/// The second field is used to keep track of the number of edges.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct DepGraphDelta(BTreeMap<Dep, BTreeSet<Dep>>, usize);

impl DepGraphDelta {
    pub fn new() -> Self {
        DepGraphDelta(BTreeMap::new(), 0)
    }

    pub fn insert(&mut self, dependent: Dep, dependency: Dep) {
        let depts = self.0.entry(dependency).or_insert_with(|| BTreeSet::new());
        if depts.insert(dependent) {
            self.1 += 1;
        }
    }

    pub fn get(&self, dependency: Dep) -> Option<&BTreeSet<Dep>> {
        self.0.get(&dependency)
    }

    /// Return an iterator over this dependency graph delta.
    ///
    /// Iterates over (dependent, dependency) pairs
    pub fn iter(&self) -> impl Iterator<Item = (Dep, Dep)> + '_ {
        self.0.iter().flat_map(|(&dependency, dependents_set)| {
            dependents_set
                .iter()
                .map(move |&dependent| (dependent, dependency))
        })
    }

    /// Return the number of edges in the dep graph delta.
    pub fn num_edges(&self) -> usize {
        self.1
    }

    /// Write all edges in the delta to the writer in a custom format.
    pub fn write_to<W: Write>(&self, w: &mut W) -> io::Result<usize> {
        let mut edges_added = 0;
        for (&dependency, dependents) in self.0.iter() {
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
        self.0.clear();
        self.1 = 0;
    }

    pub fn with_cell<R>(f: impl FnOnce(&Mutex<Self>) -> R) -> R {
        let cell = DEPGRAPH_DELTA.get_or_init(|| Mutex::new(Self::new()));
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
    pub fn with<R>(f: impl FnOnce(&Self) -> R) -> R {
        Self::with_cell(|cell| f(&cell.lock().unwrap()))
    }

    /// Run the closure with the mutable dep graph delta.
    ///
    /// # Panics
    ///
    /// See `with`
    pub fn with_mut<R>(f: impl FnOnce(&mut Self) -> R) -> R {
        Self::with_cell(|cell| f(&mut cell.lock().unwrap()))
    }
}

fn tag_to_dep_type(tag: u8) -> DepType {
    match DepType::from_u8(tag) {
        Some(dep_type) => dep_type,
        None => panic!("Invalid dep type: {:?}", tag),
    }
}

/// Hashes an `int` and `string`, arising from one of the one-argument cases of
/// `Typing_deps.Dep.variant`.
///
/// # Safety
///
/// `name1` must point to a valid OCaml string. It must not be concurrently
/// modified while this function holds a reference to it.
///
/// This function is only called from OCaml, and it is passed a value allocated
/// entirely by OCaml code, so the argument will be a valid string. The OCaml
/// runtime is interrupted by the FFI call to this function, none of the
/// transitively called functions from here call into the OCaml runtime, and we
/// do not spawn threads in our OCaml code, so the pointed-to value will not be
/// concurrently modified.
#[no_mangle]
unsafe extern "C" fn hash1_ocaml(mode: usize, dep_type_tag: usize, name1: usize) -> usize {
    fn do_hash(mode: HashMode, dep_type_tag: Value<'_>, name1: Value<'_>) -> Value<'static> {
        let dep_type_tag = dep_type_tag
            .as_int()
            .expect("dep_type_tag could not be converted to int");
        let dep_type = tag_to_dep_type(dep_type_tag as u8);
        let name1 = name1
            .as_byte_string()
            .expect("name1 could not be converted to byte string");

        let result: u64 = hash1(mode, dep_type, name1);

        // In Rust, a numeric cast between two integers of the same size
        // is a no-op. We require a 64-bit word size.
        let result = result as isize;

        Value::int(result)
    }

    let mode = HashMode::from_ocaml(mode).unwrap();
    let dep_type_tag = Value::from_bits(dep_type_tag);
    let name1 = Value::from_bits(name1);
    let result = do_hash(mode, dep_type_tag, name1);
    Value::to_bits(result)
}

/// Hashes an `int` and two `string`s, arising from one of the two-argument cases of
/// `Typing_deps.Dep.variant`.
///
/// # Safety
///
/// Both `name1` and `name2` must point to valid OCaml strings. They must not be
/// concurrently modified while this function holds a reference to them.
///
/// This function is only called from OCaml, and it is passed values allocated
/// entirely by OCaml code, so the argument will be valid strings. The OCaml
/// runtime is interrupted by the FFI call to this function, none of the
/// transitively called functions from here call into the OCaml runtime, and we
/// do not spawn threads in our OCaml code, so the pointed-to value will not be
/// concurrently modified.
#[no_mangle]
unsafe extern "C" fn hash2_ocaml(
    mode: usize,
    dep_type_tag: usize,
    name1: usize,
    name2: usize,
) -> usize {
    fn do_hash(
        mode: HashMode,
        dep_type_tag: Value<'_>,
        name1: Value<'_>,
        name2: Value<'_>,
    ) -> Value<'static> {
        let dep_type_tag = dep_type_tag
            .as_int()
            .expect("dep_type_tag could not be converted to int");
        let dep_type = tag_to_dep_type(dep_type_tag as u8);
        let name1 = name1
            .as_byte_string()
            .expect("name1 could not be converted to byte string");
        let name2 = name2
            .as_byte_string()
            .expect("name2 could not be converted to byte string");

        let result: u64 = hash2(mode, dep_type, name1, name2);

        // In Rust, a numeric cast between two integers of the same size
        // is a no-op. We require a 64-bit word size.
        let result = result as isize;

        Value::int(result)
    }

    let mode = HashMode::from_ocaml(mode).unwrap();
    let dep_type_tag = Value::from_bits(dep_type_tag);
    let name1 = Value::from_bits(name1);
    let name2 = Value::from_bits(name2);
    let result = do_hash(mode, dep_type_tag, name1, name2);
    Value::to_bits(result)
}

/// Rust set of dependencies that can be transferred from
/// OCaml to Rust memory.
#[derive(Debug, Eq, PartialEq)]
pub struct DepSet(OrdSet<Dep>);

impl std::ops::Deref for DepSet {
    type Target = OrdSet<Dep>;

    fn deref(&self) -> &OrdSet<Dep> {
        &self.0
    }
}

impl From<OrdSet<Dep>> for DepSet {
    fn from(x: OrdSet<Dep>) -> Self {
        Self(x)
    }
}

impl CamlSerialize for DepSet {
    caml_serialize_default_impls!();

    fn serialize(&self) -> Vec<u8> {
        let num_elems = self.len();
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
        let mut s: OrdSet<Dep> = OrdSet::new();
        let mut index = 0;
        while index < max_index {
            let x = u64::from_le_bytes(data[index..index + U64_SIZE].try_into().unwrap());
            s.insert(Dep::new(x));
            index += U64_SIZE;
        }
        s.into()
    }
}

/// Rust set of visited hashes
#[derive(Debug)]
pub struct VisitedSet(RefCell<BTreeSet<Dep>>);

impl std::ops::Deref for VisitedSet {
    type Target = RefCell<BTreeSet<Dep>>;

    fn deref(&self) -> &RefCell<BTreeSet<Dep>> {
        &self.0
    }
}

impl From<RefCell<BTreeSet<Dep>>> for VisitedSet {
    fn from(x: RefCell<BTreeSet<Dep>>) -> Self {
        Self(x)
    }
}

impl CamlSerialize for VisitedSet {
    caml_serialize_default_impls!();
}

// Functions to register custom Rust types with the OCaml runtime
ocaml_ffi! {
    fn hh_custom_dep_graph_register_custom_types() {
        // Safety: The OCaml runtime is currently interrupted by a call into
        // this function, so it's safe to interact with it.
        unsafe {
            DepSet::register();
            VisitedSet::register();
        }
    }
}

// Functions to query the dependency graph
ocaml_ffi! {
    fn hh_custom_dep_graph_has_edge(mode: RawTypingDepsMode, dependent: Dep, dependency: Dep) -> bool {
        // Safety: we don't call into OCaml again, so mode will remain valid.
        unsafe {
            UnsafeDepGraph::with_default(mode, false, move |g| {
                g.dependent_dependency_edge_exists(dependent, dependency)
            })
        }
    }

    fn hh_custom_dep_graph_get_ideps_from_hash(mode: RawTypingDepsMode, dep: Dep) -> Custom<DepSet> {
        let mut deps = OrdSet::new();
        DepGraphDelta::with(|delta| {
            if let Some(delta_deps) = delta.get(dep) {
                deps.extend(delta_deps.iter().copied());
            }
        });
        // Safety: we don't call into OCaml again, so mode will remain valid.
        unsafe {
            UnsafeDepGraph::with_default(mode, (), |g| {
                if let Some(hash_list) = g.hash_list_for(dep) {
                    deps.extend(g.hash_list_hashes(hash_list));
                }
            });
        }

        Custom::from(DepSet(deps))
    }

    fn hh_custom_dep_graph_add_typing_deps(mode: RawTypingDepsMode, query: Custom<DepSet>) -> Custom<DepSet> {
        // Safety: we don't call into OCaml again, so mode will remain valid.
        let mut s = unsafe {
            UnsafeDepGraph::with_option(mode, |g| match g {
                Some(g) => g.query_typing_deps_multi(&query),
                None => query.clone(),
            })
        };
        DepGraphDelta::with(|delta| {
            for dep in query.iter() {
                if let Some(depies) = delta.get(*dep) {
                    s.extend(depies.iter().copied());
                }
            }
        });
        Custom::from(DepSet(s))
    }

    fn hh_custom_dep_graph_add_extend_deps(mode: RawTypingDepsMode, query: Custom<DepSet>) -> Custom<DepSet> {
        let mut visited = BTreeSet::new();
        let mut queue = VecDeque::new();
        let mut acc = query.clone();
        for source_class in query.iter() {
            // Safety: we don't call into OCaml again, so mode will remain valid.
            unsafe {
                get_extend_deps_visit(mode, &mut visited, &mut queue, *source_class, &mut acc);
            }
        }
        while let Some(source_class) = queue.pop_front() {
            // Safety: we don't call into OCaml again, so mode will remain valid.
            unsafe {
                get_extend_deps_visit(mode, &mut visited, &mut queue, source_class, &mut acc);
            }
        }
        Custom::from(acc.into())
    }

    fn hh_custom_dep_graph_get_extend_deps(
        mode: RawTypingDepsMode,
        visited: Custom<VisitedSet>,
        source_class: Dep,
        acc: Custom<DepSet>,
    ) -> Custom<DepSet> {
        let mut visited = visited.borrow_mut();
        let mut queue = VecDeque::new();
        let mut acc = acc.clone();

        // Safety: we don't call into OCaml again, so mode will remain valid.
        unsafe {
            get_extend_deps_visit(mode, &mut visited, &mut queue, source_class, &mut acc);
            while let Some(source_class) = queue.pop_front() {
                get_extend_deps_visit(mode, &mut visited, &mut queue, source_class, &mut acc);
            }
        }
        Custom::from(acc.into())
    }

    fn hh_custom_dep_graph_register_discovered_dep_edge(
        dependent: Dep,
        dependency: Dep,
    ) {
        DepGraphDelta::with_mut(move |s| {
            s.insert(dependent, dependency);
        });
    }

    fn hh_custom_dep_graph_dep_graph_delta_num_edges() -> usize {
        DepGraphDelta::with(|s| s.num_edges())
    }

    fn hh_custom_dep_graph_save_delta(dest: OsString, reset_state_after_saving: bool) -> usize {
        let f = std::fs::OpenOptions::new()
            .create(true)
            .write(true)
            .append(true)
            .open(&dest).unwrap();
        let mut w = std::io::BufWriter::new(f);
        let hashes_added = DepGraphDelta::with(move |s| {
            s.write_to(&mut w).unwrap()
        });

        if reset_state_after_saving {
            DepGraphDelta::with_mut(|s| {
                s.clear();
            });
        }
        hashes_added
    }

    fn hh_custom_dep_graph_load_delta(mode: RawTypingDepsMode, source: OsString) -> usize {
        let f = std::fs::OpenOptions::new()
            .read(true)
            .open(&source).unwrap();
        let mut r = std::io::BufReader::new(f);

        // Safety: we don't call into OCaml again, so mode will remain valid.
        unsafe {
            UnsafeDepGraph::with_option(mode, move |g| {
                DepGraphDelta::with_mut(|s| {
                    let result = match g {
                        Some(g) => {
                            s.read_from(
                                &mut r,
                                |dependent, dependency| {
                                    // Only add when it's not already in
                                    // the graph!
                                    !g.dependent_dependency_edge_exists(
                                        dependent,
                                        dependency,
                                    )
                                },
                            )
                        }
                        None => s.read_from(&mut r, |_, _| true),
                    };
                    result.unwrap()
                })
            })
        }
    }
}

/// Helper function to recursively get extend deps
///
/// # Safety
///
/// The dependency graph mode must be a pointer to an OCaml value that's
/// still valid.
unsafe fn get_extend_deps_visit(
    mode: RawTypingDepsMode,
    visited: &mut BTreeSet<Dep>,
    queue: &mut VecDeque<Dep>,
    source_class: Dep,
    acc: &mut OrdSet<Dep>,
) {
    if !visited.insert(source_class) {
        return;
    }
    let extends_hash = match source_class.class_to_extends() {
        None => return,
        Some(hash) => hash,
    };
    let mut handle_extends_dep = |dep: Dep| {
        if dep.is_class() {
            if acc.insert(dep).is_none() {
                queue.push_back(dep);
            }
        }
    };
    DepGraphDelta::with(|delta| {
        if let Some(delta_deps) = delta.get(extends_hash) {
            delta_deps.iter().copied().for_each(&mut handle_extends_dep);
        }
    });
    UnsafeDepGraph::with_default(mode, (), |g| {
        if let Some(hash_list) = g.hash_list_for(extends_hash) {
            g.hash_list_hashes(hash_list)
                .for_each(&mut handle_extends_dep);
        }
    })
}

// Auxiliary functions for Typing_deps.DepSet/Typing_deps.VisitedSet
ocaml_ffi! {
    fn hh_visited_set_make() -> Custom<VisitedSet> {
        Custom::from(RefCell::new(BTreeSet::new()).into())
    }

    fn hh_dep_set_make() -> Custom<DepSet> {
        Custom::from(OrdSet::new().into())
    }

    fn hh_dep_set_singleton(dep: Dep) -> Custom<DepSet> {
        let mut s = OrdSet::new();
        s.insert(dep);
        Custom::from(s.into())
    }

    fn hh_dep_set_add(s: Custom<DepSet>, dep: Dep) -> Custom<DepSet> {
        let mut s = s.clone();
        s.insert(dep);
        Custom::from(s.into())
    }

    fn hh_dep_set_union(s1: Custom<DepSet>, s2: Custom<DepSet>) -> Custom<DepSet> {
        // OrdSet's implementation of union is `O(|rhs| * log |lhs| )`. This is
        // in contrast to OCaml's `O(|lhs| + |rhs|)`, and poses a problem when
        // both `|lhs|` and `|rhs|` are large.
        //
        // However, it seems that for our purposes, most often either `|s1|` or `|s2|`
        // is very small.
        let s1_len = s1.len();
        let s2_len = s2.len();
        let (left, right) = if s1_len > s2_len {
            (s1, s2)
        } else {
            (s2, s1)
        };
        let left = left.clone();
        let right = right.clone();
        Custom::from(left.union(right).into())
    }

    fn hh_dep_set_inter(s1: Custom<DepSet>, s2: Custom<DepSet>) -> Custom<DepSet> {
        let s1 = s1.clone();
        let s2 = s2.clone();
        Custom::from(s1.intersection(s2).into())
    }

    fn hh_dep_set_diff(s1: Custom<DepSet>, s2: Custom<DepSet>) -> Custom<DepSet> {
        let s1 = s1.clone();
        let s2 = s2.clone();
        Custom::from(s1.difference(s2).into())
    }

    fn hh_dep_set_mem(s: Custom<DepSet>, dep: Dep) -> bool {
        s.contains(&dep)
    }

    fn hh_dep_set_elements(s: Custom<DepSet>) -> Vec<Dep> {
        s.iter().copied().map(Dep::from).collect()
    }

     fn hh_dep_set_cardinal(s: Custom<DepSet>) -> usize {
         s.len()
     }

     fn hh_dep_set_is_empty(s: Custom<DepSet>) -> bool {
         s.is_empty()
     }

     fn hh_dep_set_of_list(xs: Vec<Dep>) -> Custom<DepSet> {
         Custom::from(OrdSet::from(&xs).into())
     }
}

#[cfg(all(test, use_unstable_features))]
mod tests {
    extern crate test;

    use super::*;
    use ocamlrep::{Arena, ToOcamlRep, Value};
    use oxidized::typing_deps_mode::HashMode;
    use test::Bencher;

    const SHORT_CLASS_NAME: &str = "\\Foo";
    const LONG_CLASS_NAME: &str = "\\EntReasonablyLongClassNameSinceSomeClassNamesAreLong";

    const HASH_MODE: HashMode = HashMode::Hash32Bit;

    #[bench]
    fn bench_hash1_short(b: &mut Bencher) {
        b.iter(|| {
            crate::hash1(
                HASH_MODE,
                crate::DepType::Class,
                SHORT_CLASS_NAME.as_bytes(),
            );
        });
    }

    #[bench]
    fn bench_hash1_long(b: &mut Bencher) {
        b.iter(|| {
            crate::hash1(HASH_MODE, crate::DepType::Class, LONG_CLASS_NAME.as_bytes());
        });
    }

    #[bench]
    fn bench_hash2_short(b: &mut Bencher) {
        b.iter(|| {
            crate::hash2(
                HASH_MODE,
                crate::DepType::Const,
                SHORT_CLASS_NAME.as_bytes(),
                b"\\T",
            );
        });
    }

    #[bench]
    fn bench_hash2_long(b: &mut Bencher) {
        b.iter(|| {
            crate::hash2(
                HASH_MODE,
                crate::DepType::Const,
                LONG_CLASS_NAME.as_bytes(),
                b"\\TSomeTypeConstant",
            );
        });
    }

    #[bench]
    fn bench_hash1_ocaml(b: &mut Bencher) {
        let arena = Arena::new();
        let dep_type = crate::DepType::Class;
        let name1 = arena.add(&String::from(LONG_CLASS_NAME));
        b.iter(|| unsafe {
            crate::hash1_ocaml(
                HASH_MODE.to_ocamlrep(&arena).to_bits(),
                Value::int(dep_type as isize).to_bits(),
                name1.to_bits(),
            )
        });
    }

    #[bench]
    fn bench_hash2_ocaml(b: &mut Bencher) {
        let arena = Arena::new();
        let dep_type = crate::DepType::Const;
        let name1 = arena.add(&String::from(LONG_CLASS_NAME));
        let name2 = arena.add(&String::from("\\TSomeTypeConstant"));
        b.iter(|| unsafe {
            crate::hash2_ocaml(
                HASH_MODE.to_ocamlrep(&arena).to_bits(),
                Value::int(dep_type as isize).to_bits(),
                name1.to_bits(),
                name2.to_bits(),
            )
        });
    }

    #[test]
    fn test_dep_set_serialize() {
        let mut x: OrdSet<Dep> = OrdSet::new();
        x.insert(Dep::new(1));
        x.insert(Dep::new(2));
        let x: DepSet = x.into();

        let buf = x.serialize();
        let y = DepSet::deserialize(&buf);

        assert_eq!(x, y);
    }

    #[test]
    fn test_dep_graph_delta_serialize_empty() {
        let x = DepGraphDelta::new();
        let mut bytes = Vec::new();
        x.write_to(&mut bytes).unwrap();

        let mut y = DepGraphDelta::new();
        let mut bytes_read: &[u8] = &bytes;
        let num_loaded = y.read_from(&mut bytes_read, |_, _| true).unwrap();

        assert_eq!(num_loaded, 0);
        assert_eq!(x, y);
    }

    #[test]
    fn test_dep_graph_delta_serialize_non_empty() {
        let mut x = DepGraphDelta::new();
        x.insert(Dep::new(10), Dep::new(1));
        x.insert(Dep::new(10), Dep::new(2));
        x.insert(Dep::new(11), Dep::new(2));
        x.insert(Dep::new(12), Dep::new(3));
        let mut bytes = Vec::new();
        x.write_to(&mut bytes).unwrap();

        let mut y = DepGraphDelta::new();
        let mut bytes_read: &[u8] = &bytes;
        let num_loaded = y.read_from(&mut bytes_read, |_, _| true).unwrap();

        assert_eq!(num_loaded, 4);
        assert_eq!(x, y);
    }

    #[test]
    fn test_dep_graph_delta_iter_empty() {
        let x = DepGraphDelta::new();
        let v: Vec<_> = x.iter().collect();
        assert_eq!(v.len(), 0);
    }

    #[test]
    fn test_dep_graph_delta_iter_non_empty() {
        let mut x = DepGraphDelta::new();
        let edges = vec![
            (Dep::new(10), Dep::new(1)),
            (Dep::new(10), Dep::new(2)),
            (Dep::new(11), Dep::new(2)),
            (Dep::new(12), Dep::new(3)),
        ];
        for (dependency, dependent) in edges.iter() {
            x.insert(*dependency, *dependent)
        }
        let v: Vec<_> = x.iter().collect();
        assert_eq!(v, edges);
    }
}
