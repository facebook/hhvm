// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg_attr(use_unstable_features, feature(test))]

use std::cell::RefCell;
use std::collections::VecDeque;
use std::ffi::OsString;
use std::fs::File;
use std::path::Path;

use dep::Dep;
use deps_rust::dep_graph_override;
use deps_rust::iter_dependents_with_duplicates;
use deps_rust::DepSet;
use deps_rust::RawTypingDepsMode;
use deps_rust::VisitedSet;
use deps_rust::DEP_GRAPH;
use hash::HashSet;
use ocamlrep::Value;
use ocamlrep_custom::CamlSerialize;
use ocamlrep_custom::Custom;
use ocamlrep_ocamlpool::ocaml_ffi;
use rpds::HashTrieSet;
use typing_deps_hash::hash1;
use typing_deps_hash::hash2;
use typing_deps_hash::DepType;

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

/// Hashes an `int` and two `string`s, arising from one of the two-argument cases of
/// `Typing_deps.Dep.variant`.
///
/// # Safety
///
/// `type_hash` must be a valid OCaml int.
/// `member_name` must point to a valid OCaml string. It must not be concurrently
/// modified while this function holds a reference to it.
///
/// This function is only called from OCaml, and it is passed values allocated
/// entirely by OCaml code, so the argument will be a valid string. The OCaml
/// runtime is interrupted by the FFI call to this function, none of the
/// transitively called functions from here call into the OCaml runtime, and we
/// do not spawn threads in our OCaml code, so the pointed-to value will not be
/// concurrently modified.
#[no_mangle]
unsafe extern "C" fn hash2_ocaml(
    dep_type_tag: usize,
    type_hash: usize,
    member_name: usize,
) -> usize {
    fn do_hash(
        dep_type_tag: Value<'_>,
        type_hash: Value<'_>,
        member_name: Value<'_>,
    ) -> Value<'static> {
        let dep_type_tag = dep_type_tag
            .as_int()
            .expect("dep_type_tag could not be converted to int");
        let dep_type = tag_to_dep_type(dep_type_tag as u8);

        // Ocaml ints are i63, signed extended to i64. clear the MSB while
        // converting to u64, to match FromOcamlRep for Dep.
        let type_hash = type_hash
            .as_int()
            .expect("type_hash could not be converted to int");
        let type_hash = type_hash as u64 & !(1 << 63);

        let member_name = member_name
            .as_byte_string()
            .expect("member_name could not be converted to byte string");

        let result: u64 = hash2(dep_type, type_hash, member_name);

        // In Rust, a numeric cast between two integers of the same size
        // is a no-op. We require a 64-bit word size.
        let result = result as isize;

        Value::int(result)
    }

    let dep_type_tag = Value::from_bits(dep_type_tag);
    let type_hash = Value::from_bits(type_hash);
    let member_name = Value::from_bits(member_name);
    let result = do_hash(dep_type_tag, type_hash, member_name);
    Value::to_bits(result)
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
    fn hh_custom_dep_graph_replace(mode: RawTypingDepsMode) {
        // Safety: we don't call into OCaml again, so mode will remain valid.
        dep_graph_override(mode);
    }

    // Returns true if we know for sure that the depgraph has the edge, false
    // if we don't know.
    fn hh_depgraph_has_edge_for_sure(mode: RawTypingDepsMode, dependent: Dep, dependency: Dep) -> bool {
        depgraph_has_edge_for_sure(mode, dependent, dependency)
    }

    fn hh_custom_dep_graph_get_ideps_from_hash(mode: RawTypingDepsMode, dep: Dep) -> Custom<DepSet> {
        get_ideps_from_hash(mode, dep)
    }

    // Returns the union of the provided dep set and their direct typing dependents.
    fn hh_custom_dep_graph_add_typing_deps(mode: RawTypingDepsMode, query: Custom<DepSet>) -> Custom<DepSet> {
        query_and_accumulate_typing_deps(mode, query)
    }

    // Returns the union of the provided dep set and their recursive 'extends' dependents.
    fn hh_custom_dep_graph_add_extend_deps(mode: RawTypingDepsMode, query: Custom<DepSet>) -> Custom<DepSet> {
        add_extend_deps(mode, query)
    }

    // Returns the recursive 'extends' dependents of a dep.
    fn hh_custom_dep_graph_get_extend_deps(
        mode: RawTypingDepsMode,
        visited: Custom<VisitedSet>,
        source_class: Dep,
        acc: Custom<DepSet>,
    ) -> Custom<DepSet> {
        get_extend_deps(mode, visited, source_class, acc)
    }

    // Add edge into the in-memory depgraph delta
    fn hh_custom_dep_graph_register_discovered_dep_edge(
        dependent: Dep,
        dependency: Dep,
    ) {
        DEP_GRAPH.lock_mut_delta_and(move |s| {
            s.insert(dependent, dependency);
        });
    }

    fn hh_custom_dep_graph_dep_graph_delta_num_edges() -> usize {
        deps_rust::dep_graph_delta_num_edges()
    }

    fn hh_custom_dep_graph_save_delta(dest: OsString, reset_state_after_saving: bool) -> usize {
        save_delta(dest, reset_state_after_saving)
    }

    fn hh_custom_dep_graph_load_delta(mode: RawTypingDepsMode, source: OsString) -> usize {
        load_delta(mode, source)
    }

    // Moves the source file to the destination directory.
    fn hh_save_custom_dep_graph_save_delta(source: OsString, dest_dir: OsString) -> usize {
        let dest_file = Path::new(&dest_dir)
            .join(source.to_str().unwrap().replace('/', "-"));
        std::fs::rename(&source, dest_file).unwrap();

        // Technically we loaded 0 deps into the hh_server dep graph
        0
    }
}

/// Returns true if we know for sure that the depgraph has the edge, false
/// if we don't know.
fn depgraph_has_edge_for_sure(mode: RawTypingDepsMode, dependent: Dep, dependency: Dep) -> bool {
    DEP_GRAPH.lock_and(mode, |g| g.has_edge_for_sure(dependent, dependency))
}

fn get_ideps_from_hash(mode: RawTypingDepsMode, dep: Dep) -> Custom<DepSet> {
    let deps =
        iter_dependents_with_duplicates(mode, dep, |iter_dep| iter_dep.collect::<HashTrieSet<_>>());
    Custom::from(DepSet::from(deps))
}

/// Returns the union of the provided dep set and their direct typing dependents.
fn query_and_accumulate_typing_deps(
    mode: RawTypingDepsMode,
    query: Custom<DepSet>,
) -> Custom<DepSet> {
    let mut acc = query.clone();
    for dependency in query.iter() {
        iter_dependents_with_duplicates(mode, *dependency, |iter| {
            iter.for_each(|dependent| acc.insert_mut(dependent))
        })
    }
    Custom::from(DepSet::from(acc))
}

/// Returns the union of the provided dep set and their recursive 'extends' dependents.
fn add_extend_deps(mode: RawTypingDepsMode, query: Custom<DepSet>) -> Custom<DepSet> {
    let mut visited = HashSet::default();
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

/// Returns the recursive 'extends' dependents of a dep.
fn get_extend_deps(
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

/// Helper function to recursively get extend deps
///
/// # Safety
///
/// The dependency graph mode must be a pointer to an OCaml value that's
/// still valid.
unsafe fn get_extend_deps_visit(
    mode: RawTypingDepsMode,
    visited: &mut HashSet<Dep>,
    queue: &mut VecDeque<Dep>,
    source_class: Dep,
    acc: &mut HashTrieSet<Dep>,
) {
    if !visited.insert(source_class) {
        return;
    }
    let extends_hash = match source_class.class_to_extends() {
        None => return,
        Some(hash) => hash,
    };
    iter_dependents_with_duplicates(mode, extends_hash, |iter| {
        iter.for_each(|dep: Dep| {
            if dep.is_class() {
                if !acc.contains(&dep) {
                    acc.insert_mut(dep);
                    queue.push_back(dep);
                }
            }
        })
    })
}

fn save_delta(dest: OsString, reset_state_after_saving: bool) -> usize {
    let f = std::fs::OpenOptions::new()
        .create(true)
        .append(true)
        .open(dest)
        .unwrap();
    let hashes_added = DEP_GRAPH.lock_delta_and(move |s| {
        let mut w = std::io::BufWriter::new(f);
        let hashes_added = s.write_added_edges_to(&mut w).unwrap();
        w.into_inner().unwrap();
        hashes_added
    });

    if reset_state_after_saving {
        DEP_GRAPH.lock_mut_delta_and(|s| {
            s.clear();
        });
    }
    hashes_added
}

fn load_delta(mode: RawTypingDepsMode, source: OsString) -> usize {
    let f = File::open(source).unwrap();
    let mut r = std::io::BufReader::new(f);

    // Safety: we don't call into OCaml again, so mode will remain valid.
    DEP_GRAPH.lock_base_and(mode, move |g| {
        DEP_GRAPH.lock_mut_delta_and(|s| {
            let result = match g {
                Some(g) => {
                    s.read_added_edges_from(&mut r, |dependent, dependency| {
                        // Only add when it's not already in
                        // the graph!
                        !g.dependent_dependency_edge_exists(dependent, dependency)
                    })
                }
                None => s.read_added_edges_from(&mut r, |_, _| true),
            };
            result.unwrap()
        })
    })
}

// Auxiliary functions for Typing_deps.DepSet/Typing_deps.VisitedSet
ocaml_ffi! {
    fn hh_visited_set_make() -> Custom<VisitedSet> {
        Custom::from(RefCell::new(HashSet::default()).into())
    }

    fn hh_dep_set_make() -> Custom<DepSet> {
        Custom::from(HashTrieSet::new().into())
    }

    fn hh_dep_set_singleton(dep: Dep) -> Custom<DepSet> {
        let mut s = HashTrieSet::new();
        s.insert_mut(dep);
        Custom::from(s.into())
    }

    fn hh_dep_set_add(s: Custom<DepSet>, dep: Dep) -> Custom<DepSet> {
        let mut s = s.clone();
        s.insert_mut(dep);
        Custom::from(s.into())
    }

    fn hh_dep_set_union(s1: Custom<DepSet>, s2: Custom<DepSet>) -> Custom<DepSet> {
        Custom::from(s1.union(&s2))
    }

    fn hh_dep_set_inter(s1: Custom<DepSet>, s2: Custom<DepSet>) -> Custom<DepSet> {
        Custom::from(s1.intersect(&s2))
    }

    fn hh_dep_set_diff(s1: Custom<DepSet>, s2: Custom<DepSet>) -> Custom<DepSet> {
        Custom::from(s1.difference(&s2))
    }

    fn hh_dep_set_mem(s: Custom<DepSet>, dep: Dep) -> bool {
        s.contains(&dep)
    }

    fn hh_dep_set_elements(s: Custom<DepSet>) -> Vec<Dep> {
        s.iter().copied().map(Dep::from).collect()
    }

     fn hh_dep_set_cardinal(s: Custom<DepSet>) -> usize {
         s.size()
     }

     fn hh_dep_set_is_empty(s: Custom<DepSet>) -> bool {
         s.is_empty()
     }

     fn hh_dep_set_of_list(xs: Vec<Dep>) -> Custom<DepSet> {
         Custom::from(HashTrieSet::from_iter(xs).into())
     }
}

#[cfg(all(test, use_unstable_features))]
mod tests {
    extern crate test;

    use ocamlrep::Arena;
    use ocamlrep::ToOcamlRep;
    use ocamlrep::Value;
    use test::Bencher;

    const SHORT_CLASS_NAME: &str = "\\Foo";
    const LONG_CLASS_NAME: &str = "\\EntReasonablyLongClassNameSinceSomeClassNamesAreLong";

    #[bench]
    fn bench_hash1_short(b: &mut Bencher) {
        b.iter(|| {
            crate::hash1(crate::DepType::Type, SHORT_CLASS_NAME.as_bytes());
        });
    }

    #[bench]
    fn bench_hash1_long(b: &mut Bencher) {
        b.iter(|| {
            crate::hash1(crate::DepType::Type, LONG_CLASS_NAME.as_bytes());
        });
    }

    #[bench]
    fn bench_hash2_short(b: &mut Bencher) {
        b.iter(|| {
            let type_hash = crate::hash1(crate::DepType::Type, SHORT_CLASS_NAME.as_bytes());
            crate::hash2(crate::DepType::Const, type_hash, b"\\T");
        });
    }

    #[bench]
    fn bench_hash2_long(b: &mut Bencher) {
        b.iter(|| {
            let type_hash = crate::hash1(crate::DepType::Type, LONG_CLASS_NAME.as_bytes());
            crate::hash2(crate::DepType::Const, type_hash, b"\\TSomeTypeConstant");
        });
    }

    #[bench]
    fn bench_hash1_ocaml(b: &mut Bencher) {
        let arena = Arena::new();
        let dep_type = crate::DepType::Type;
        let name1 = arena.add(LONG_CLASS_NAME);
        b.iter(|| unsafe {
            crate::hash1_ocaml(Value::int(dep_type as isize).to_bits(), name1.to_bits())
        });
    }

    #[bench]
    fn bench_hash2_ocaml(b: &mut Bencher) {
        let arena = Arena::new();
        let dep_type = crate::DepType::Const;
        let type_hash = crate::hash1(crate::DepType::Type, LONG_CLASS_NAME.as_bytes());
        let member_name = arena.add("\\TSomeTypeConstant");
        b.iter(|| unsafe {
            crate::hash2_ocaml(
                Value::int(dep_type as isize).to_bits(),
                type_hash.to_ocamlrep(&arena).to_bits(),
                member_name.to_bits(),
            )
        });
    }
}
