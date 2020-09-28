// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg_attr(use_unstable_features, feature(test))]

use depgraph::reader::{Dep, DepGraph, DepGraphOpener};
use fnv::FnvHasher;
use im_rc::OrdSet;
use ocamlrep::{from, Allocator, FromError, FromOcamlRep, OpaqueValue, ToOcamlRep, Value};
use ocamlrep_custom::{caml_serialize_default_impls, CamlSerialize, Custom};
use ocamlrep_ocamlpool::ocaml_ffi;
use once_cell::sync::OnceCell;
use std::cell::RefCell;
use std::collections::BTreeSet;
use std::convert::TryInto;
use std::ffi::OsString;
use std::hash::Hasher;
use std::io::Write;
use std::panic;

fn _static_assert() {
    // The use of 64-bit (actually 63-bit) dependency hashes requires that we
    // are compiling for a 64-bit architecture. Let's assert that at compile time.
    //
    // OCaml only supports unboxed integers of WORD SIZE - 1 bits. We don't want to
    // be boxing dependency hashes, so we require a 64-bit word size.
    let _ = [(); 0 - (!(8 == std::mem::size_of::<usize>()) as usize)];
}

/// We record the dep graph filename in every worker. If a worker
/// tries to read from it, we lazily open the graph.
static DEPGRAPH_FILENAME: OnceCell<OsString> = OnceCell::new();

/// A structure wrapping the memory-mapped dependency graph.
/// Each worker will itself lazily (or eagerly upon request)
/// open a memory-mapping to the dependency graph.
static DEPGRAPH: OnceCell<UnsafeDepGraph> = OnceCell::new();

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

    /// Register the dep graph filename.
    ///
    /// # Panics
    ///
    /// If a previous file name was already registered.
    pub fn register(depgraph_fn: OsString) {
        let current_fn = DEPGRAPH_FILENAME.get_or_init(|| depgraph_fn.clone());

        if current_fn != &depgraph_fn {
            panic!("programming error: dep graph filename already registered");
        }
    }

    /// Load the graph using `DEPGRAPH_FILENAME`
    ///
    /// A no-op if the graph is already loaded.
    ///
    /// # Panics
    ///
    /// If this function is called before first registering
    /// the file path using `register`.
    pub fn load() -> Result<(), String> {
        let _depgraph = DEPGRAPH.get_or_try_init::<_, String>(|| {
            let depgraph_fn: Option<&OsString> = DEPGRAPH_FILENAME.get();
            let depgraph_fn = depgraph_fn.unwrap_or_else(|| {
                panic!("programming error: cannot call load before registering path")
            });

            let opener = DepGraphOpener::from_path(&depgraph_fn)
                .map_err(|err| format!("could not open dep graph file: {:?}", err))?;
            let depgraph = UnsafeDepGraph::new(opener)?;
            Ok(depgraph)
        })?;

        Ok(())
    }

    /// Run the closure with the loaded dep graph.
    ///
    /// # Panics
    ///
    /// Panics if the graph is not loaded, and a graph
    /// file name is not registered.
    ///
    /// Panics if the graph is not yet loaded, and opening
    /// the graph results in an error.
    pub fn with<F, R>(f: F) -> R
    where
        for<'a> F: FnOnce(&'a DepGraph<'a>) -> R,
    {
        if !Self::is_initialized() {
            Self::load().unwrap();
        }

        let g = DEPGRAPH.get().unwrap();
        f(g.depgraph())
    }

    /// Return whether the global custom dependency graph is initialized.
    #[inline(always)]
    pub fn is_initialized() -> bool {
        // This should be really fast on x86, as the `get` call
        // just does an atomic load with acquire semantics, which doesn't
        // require any memory fences on x86.
        DEPGRAPH.get().is_some()
    }
}

/// Variant types used in the naming table.
///
/// NOTE: Keep in sync with the order of the fields in `Typing_deps.ml`.
#[derive(Copy, Clone, Debug)]
pub enum DepType {
    GConst = 0,
    Fun = 1,
    Class = 2,
    Extends = 3,
    RecordDef = 4,
    Const = 5,
    Cstr = 6,
    Prop = 7,
    SProp = 8,
    Method = 9,
    SMethod = 10,
    AllMembers = 11,
    FunName = 12,
    GConstName = 13,
}

fn tag_to_dep_type(tag: u8) -> DepType {
    match tag {
        0 => DepType::GConst,
        1 => DepType::Fun,
        2 => DepType::Class,
        3 => DepType::Extends,
        4 => DepType::RecordDef,
        5 => DepType::Const,
        6 => DepType::Cstr,
        7 => DepType::Prop,
        8 => DepType::SProp,
        9 => DepType::Method,
        10 => DepType::SMethod,
        11 => DepType::AllMembers,
        12 => DepType::FunName,
        13 => DepType::GConstName,
        _ => panic!("Invalid dep type: {:?}", tag),
    }
}

/// Select the hashing algorithm to use for dependency hashes.
///
/// FnvHasher appears to produce better hashes (fewer collisions) than
/// `std::collections::hash_map::DefaultHasher` on our workloads. However, other
/// hashing algorithms may perform better still.
fn make_hasher() -> FnvHasher {
    Default::default()
}

fn postprocess_hash(dep_type: DepType, hash: u64) -> u64 {
    let hash: u64 = match dep_type {
        DepType::Class => {
            // For class dependencies, set the lowest bit to 1. For extends
            // dependencies, the lowest bit will be 0 (in the case below), so we'll
            // be able to convert from a class hash to its extends hash without
            // reversing the hash.
            (hash << 1) | 1
        }
        _ => {
            // Ensure that only classes have the lowest bit set to 1, so that we
            // don't try to transitively traverse the subclasses of non-class
            // dependencies.
            hash << 1
        }
    };

    if !UnsafeDepGraph::is_initialized() {
        // We are in the legacy dependency graph system:
        //
        // The shared-memory dependency graph stores edges as pairs of vertices.
        // Each vertex has 31 bits of actual content and 1 bit of OCaml bookkeeping.
        // Thus, we truncate the hash to 31 bits.
        hash & ((1 << 31) - 1)
    } else {
        // One bit is used for OCaml bookkeeping!
        hash & !(1 << 63)
    }
}

fn get_dep_type_hash_key(dep_type: DepType) -> u8 {
    match dep_type {
        DepType::Class | DepType::Extends => {
            // Use the same tag for classes and extends dependencies, so that we can
            // convert between them without reversing the hash.
            DepType::Class as u8
        }
        _ => dep_type as u8,
    }
}

/// Hash a one-argument `Typing_deps.Dep.variant`'s fields.
pub fn hash1(dep_type: DepType, name1: &[u8]) -> u64 {
    let mut hasher = make_hasher();
    hasher.write_u8(get_dep_type_hash_key(dep_type));
    hasher.write(name1);
    postprocess_hash(dep_type, hasher.finish())
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
unsafe extern "C" fn hash1_ocaml(dep_type_tag: usize, name1: usize) -> usize {
    fn do_hash(dep_type_tag: Value<'_>, name1: Value<'_>) -> Value<'static> {
        let dep_type_tag = dep_type_tag
            .as_int()
            .expect("dep_type_tag could not be converted to int");
        let dep_type = tag_to_dep_type(dep_type_tag as u8);
        let name1 = name1
            .as_byte_string()
            .expect("name1 could not be converted to byte string");

        let result: u64 = hash1(dep_type, name1);

        // In Rust, a numeric cast between two integers of the same size
        // is a no-op. We require a 64-bit word size.
        let result = result as isize;

        Value::int(result)
    }

    let dep_type_tag = Value::from_bits(dep_type_tag);
    let name1 = Value::from_bits(name1);
    let result = do_hash(dep_type_tag, name1);
    Value::to_bits(result)
}

/// Hash a two-argument `Typing_deps.Dep.variant`'s fields.
pub fn hash2(dep_type: DepType, name1: &[u8], name2: &[u8]) -> u64 {
    let mut hasher = make_hasher();
    hasher.write_u8(get_dep_type_hash_key(dep_type));
    hasher.write(name1);
    hasher.write(name2);
    postprocess_hash(dep_type, hasher.finish())
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
unsafe extern "C" fn hash2_ocaml(dep_type_tag: usize, name1: usize, name2: usize) -> usize {
    fn do_hash(dep_type_tag: Value<'_>, name1: Value<'_>, name2: Value<'_>) -> Value<'static> {
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

        let result: u64 = hash2(dep_type, name1, name2);

        // In Rust, a numeric cast between two integers of the same size
        // is a no-op. We require a 64-bit word size.
        let result = result as isize;

        Value::int(result)
    }

    let dep_type_tag = Value::from_bits(dep_type_tag);
    let name1 = Value::from_bits(name1);
    let name2 = Value::from_bits(name2);
    let result = do_hash(dep_type_tag, name1, name2);
    Value::to_bits(result)
}

/// Rust implementation of `Typing_deps.NamingHash.combine_hashes`.
pub fn combine_hashes(dep_hash: u64, naming_hash: i64) -> i64 {
    let dep_hash = dep_hash & ((1 << 31) - 1);
    let upper_31_bits = (dep_hash as i64) << 31;
    let lower_31_bits = naming_hash & 0b01111111_11111111_11111111_11111111;
    upper_31_bits | lower_31_bits
}

// A wrapper around isize, an OCaml int, representing a dependency, with
// conversion support to and from `Dep`.
#[derive(Debug, Copy, Clone)]
struct OcamlDep(isize);

impl From<Dep> for OcamlDep {
    fn from(dep: Dep) -> OcamlDep {
        let dep: u64 = dep.into();
        // In Rust, a numeric cast between two integers of the same size
        // is a no-op. We require a 64-bit word size.
        OcamlDep(dep as isize)
    }
}

impl Into<Dep> for OcamlDep {
    fn into(self) -> Dep {
        let dep: isize = self.0;
        // In Rust, a numeric cast between two integers of the same size
        // is a no-op. We require a 64-bit word size.
        Dep::new(dep as u64)
    }
}

impl FromOcamlRep for OcamlDep {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let x = from::expect_int(value)?;
        Ok(OcamlDep(x))
    }
}

impl ToOcamlRep for OcamlDep {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> OpaqueValue<'a> {
        let dep = self.0;
        OpaqueValue::int(dep)
    }
}

// Functions to load the dep graph
ocaml_ffi! {
    fn hh_custom_dep_graph_register(depgraph_fn: OsString) {
        UnsafeDepGraph::register(depgraph_fn);
    }

    fn hh_custom_dep_graph_force_load() -> Result<(), String> {
        UnsafeDepGraph::load()
    }
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
    fn hh_custom_dep_graph_get_ideps_from_hash(dep: OcamlDep) -> Custom<DepSet> {
        let set_opt = UnsafeDepGraph::with(move |g| {
            let list = g.hash_list_for(dep.into())?;
            let hashes: OrdSet<Dep> = g.hash_list_hashes(list).collect();
            Some(hashes)
        });
        Custom::from(set_opt.unwrap_or_else(OrdSet::new).into())
    }

    fn hh_custom_dep_graph_add_typing_deps(s: Custom<DepSet>) -> Custom<DepSet> {
        UnsafeDepGraph::with(move |g| {
            Custom::from(g.query_typing_deps_multi(&s).into())
        })
    }

    fn hh_custom_dep_graph_add_extend_deps(s: Custom<DepSet>) -> Custom<DepSet> {
        let mut visited = BTreeSet::new();
        let s = s.clone();
        let mut acc = s.clone();
        let acc = UnsafeDepGraph::with(move |g| {
            for dep in s {
                if dep.is_class() {
                    g.add_extend_deps(&mut acc, dep, &mut visited);
                }
            }
            acc
        });
        Custom::from(acc.into())
    }

    fn hh_custom_dep_graph_get_extend_deps(
        visited: Custom<VisitedSet>,
        source_class: OcamlDep,
        acc: Custom<DepSet>,
    ) -> Custom<DepSet> {
        let mut visited = visited.borrow_mut();
        let mut acc = acc.clone();
        let acc = UnsafeDepGraph::with(move |g| {
            g.add_extend_deps(&mut acc, source_class.into(), &mut visited);
            acc
        });
        Custom::from(acc.into())
    }
}

// Auxiliary functions for Typing_deps.DepSet/Typing_deps.VisitedSet
ocaml_ffi! {
    fn hh_visited_set_make() -> Custom<VisitedSet> {
        Custom::from(RefCell::new(BTreeSet::new()).into())
    }

    fn hh_dep_set_make() -> Custom<DepSet> {
        Custom::from(OrdSet::new().into())
    }

    fn hh_dep_set_singleton(dep: OcamlDep) -> Custom<DepSet> {
        let mut s = OrdSet::new();
        s.insert(dep.into());
        Custom::from(s.into())
    }

    fn hh_dep_set_add(s: Custom<DepSet>, dep: OcamlDep) -> Custom<DepSet> {
        let mut s = s.clone();
        s.insert(dep.into());
        Custom::from(s.into())
    }

    fn hh_dep_set_union(s1: Custom<DepSet>, s2: Custom<DepSet>) -> Custom<DepSet> {
        let s1 = s1.clone();
        let s2 = s2.clone();
        Custom::from(s1.union(s2).into())
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

    fn hh_dep_set_mem(s: Custom<DepSet>, dep: OcamlDep) -> bool {
        s.contains(&dep.into())
    }

    fn hh_dep_set_elements(s: Custom<DepSet>) -> Vec<OcamlDep> {
        s.iter().copied().map(OcamlDep::from).collect()
    }

     fn hh_dep_set_cardinal(s: Custom<DepSet>) -> usize {
         s.len()
     }

     fn hh_dep_set_is_empty(s: Custom<DepSet>) -> bool {
         s.is_empty()
     }
}

#[cfg(all(test, use_unstable_features))]
mod tests {
    extern crate test;

    use super::*;
    use ocamlrep::{Arena, Value};
    use test::Bencher;

    const SHORT_CLASS_NAME: &str = "\\Foo";
    const LONG_CLASS_NAME: &str = "\\EntReasonablyLongClassNameSinceSomeClassNamesAreLong";

    #[bench]
    fn bench_hash1_short(b: &mut Bencher) {
        b.iter(|| {
            crate::hash1(crate::DepType::Class, SHORT_CLASS_NAME.as_bytes());
        });
    }

    #[bench]
    fn bench_hash1_long(b: &mut Bencher) {
        b.iter(|| {
            crate::hash1(crate::DepType::Class, LONG_CLASS_NAME.as_bytes());
        });
    }

    #[bench]
    fn bench_hash2_short(b: &mut Bencher) {
        b.iter(|| {
            crate::hash2(crate::DepType::Const, SHORT_CLASS_NAME.as_bytes(), b"\\T");
        });
    }

    #[bench]
    fn bench_hash2_long(b: &mut Bencher) {
        b.iter(|| {
            crate::hash2(
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
            crate::hash1_ocaml(Value::int(dep_type as isize).to_bits(), name1.to_bits())
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
}
