// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use lazy_static::lazy_static;
use std::collections::{BTreeMap, HashMap};

lazy_static! {
    static ref ALIAS_MAP: HashMap<String, String> = {
        ALIASES.iter().fold(HashMap::new(), |mut map, s| {
            map.insert(s.to_lowercase(), "HH\\".to_string() + s);
            map
        })
    };
    pub static ref FUNCS_MAP: BTreeMap<String, String> = make_map(FUNCS);
    pub static ref CONSTS_MAP: BTreeMap<String, String> = make_map(CONSTS);
    pub static ref NAMESPACES_MAP: BTreeMap<String, String> = make_map(NAMESPACES);
}

static ALIASES: &'static [&'static str] = &[
    "AsyncIterator",
    "AsyncKeyedIterator",
    "Traversable",
    "Container",
    "KeyedTraversable",
    "KeyedContainer",
    "Iterator",
    "KeyedIterator",
    "Iterable",
    "KeyedIterable",
    "Collection",
    "Vector",
    "Map",
    "Set",
    "Pair",
    "ImmVector",
    "ImmMap",
    "ImmSet",
    "InvariantException",
    "IMemoizeParam",
    "Shapes",
    "TypeStructureKind",
    "TypeStructure",
    "dict",
    "vec",
    "keyset",
    "varray",
    "darray",
    "Awaitable",
    "AsyncGenerator",
    "StaticWaitHandle",
    "WaitableWaitHandle",
    "ResumableWaitHandle",
    "AsyncFunctionWaitHandle",
    "AsyncGeneratorWaitHandle",
    "AwaitAllWaitHandle",
    "ConditionWaitHandle",
    "RescheduleWaitHandle",
    "SleepWaitHandle",
    "ExternalThreadEventWaitHandle",
    "bool",
    "int",
    "float",
    "string",
    "void",
    "num",
    "arraykey",
    "resource",
    "mixed",
    "noreturn",
    "this",
    "varray_or_darray",
    "vec_or_dict",
    "arraylike",
    "nonnull",
    "null",
    "nothing",
    "dynamic",
];

static FUNCS: &'static [&'static str] = &[
    "asio_get_current_context_idx",
    "asio_get_running_in_context",
    "asio_get_running",
    "class_meth",
    "darray",
    "dict",
    "fun",
    "heapgraph_create",
    "heapgraph_dfs_edges",
    "heapgraph_dfs_nodes",
    "heapgraph_edge",
    "heapgraph_foreach_edge",
    "heapgraph_foreach_node",
    "heapgraph_foreach_root",
    "heapgraph_node_in_edges",
    "heapgraph_node_out_edges",
    "heapgraph_node",
    "heapgraph_stats",
    "idx",
    "inst_meth",
    "invariant_callback_register",
    "invariant_violation",
    "invariant",
    "is_darray",
    "is_dict",
    "is_keyset",
    "is_varray",
    "is_vec",
    "keyset",
    "meth_caller",
    "objprof_get_data",
    "objprof_get_paths",
    "objprof_get_strings",
    "server_warmup_status",
    "thread_mark_stack",
    "thread_memory_stats",
    "type_structure",
    "varray",
    "vec",
    "xenon_get_data",
];

static CONSTS: &'static [&'static str] = &[];

static NAMESPACES: &'static [&'static str] = &["Rx"];

fn make_map(items: &[&str]) -> BTreeMap<String, String> {
    items.iter().fold(BTreeMap::new(), |mut map, s| {
        let prefix = "HH\\";
        let v = String::with_capacity(prefix.len() + s.len());
        map.insert(s.to_string(), v + prefix + s);
        map
    })
}

pub fn normalize(s: &str) -> &str {
    match ALIAS_MAP.get(&s.to_lowercase()[..]) {
        None => s,
        Some(alias) => normalize(alias),
    }
}

pub fn opt_normalize(s: &str) -> Option<&str> {
    match ALIAS_MAP.get(&s.to_lowercase()[..]) {
        Some(v) => Some(normalize(v)),
        None => None,
    }
}

pub fn is_hh_autoimport(s: &str) -> bool {
    ALIAS_MAP.contains_key(&s.to_lowercase()[..])
}

#[cfg(test)]
mod tests {
    use crate::is_hh_autoimport;
    use crate::normalize;
    use crate::opt_normalize;

    #[test]
    fn test_is_hh_autoimport() {
        assert_eq!(is_hh_autoimport("float"), true);
        assert_eq!(is_hh_autoimport("KeyedIterable"), true);
        assert_eq!(is_hh_autoimport("non-exisit"), false);
    }

    #[test]
    fn test_normalize() {
        assert_eq!(normalize("float"), "HH\\float");
        assert_eq!(normalize("KeyedIterable"), "HH\\KeyedIterable");
        assert_eq!(normalize("non-exisit"), "non-exisit");
    }

    #[test]
    fn test_opt_normalize() {
        assert_eq!(opt_normalize("float"), Some("HH\\float"));
        assert_eq!(opt_normalize("KeyedIterable"), Some("HH\\KeyedIterable"));
        assert_eq!(opt_normalize("non-exisit"), None);
    }

}
