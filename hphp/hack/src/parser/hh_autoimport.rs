// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use lazy_static::lazy_static;

lazy_static! {
    pub static ref TYPES_MAP: BTreeMap<String, String> = make_map(TYPES);
    pub static ref FUNCS_MAP: BTreeMap<String, String> = make_map(FUNCS);
    pub static ref CONSTS_MAP: BTreeMap<String, String> = make_map(CONSTS);
    pub static ref NAMESPACES_MAP: BTreeMap<String, String> = make_map(NAMESPACES);
}

pub static TYPES: &[&str] = &[
    "AnyArray",
    "AsyncFunctionWaitHandle",
    "AsyncGenerator",
    "AsyncGeneratorWaitHandle",
    "AsyncIterator",
    "AsyncKeyedIterator",
    "Awaitable",
    "AwaitAllWaitHandle",
    "classname",
    "Collection",
    "ConcurrentWaitHandle",
    "ConditionWaitHandle",
    "Container",
    "darray",
    "dict",
    "ExternalThreadEventWaitHandle",
    "IMemoizeParam",
    "UNSAFESingletonMemoizeParam",
    "ImmMap",
    "ImmSet",
    "ImmVector",
    "InvariantException",
    "Iterable",
    "Iterator",
    "KeyedContainer",
    "KeyedIterable",
    "KeyedIterator",
    "KeyedTraversable",
    "keyset",
    "Map",
    "ObjprofObjectStats",
    "ObjprofPathsStats",
    "ObjprofStringStats",
    "Pair",
    "RescheduleWaitHandle",
    "ResumableWaitHandle",
    "Set",
    "Shapes",
    "SleepWaitHandle",
    "StaticWaitHandle",
    "supportdyn",
    "Traversable",
    "typename",
    "TypeStructure",
    "TypeStructureKind",
    "varray_or_darray",
    "varray",
    "vec_or_dict",
    "vec",
    "Vector",
    "WaitableWaitHandle",
    "XenonSample",
];

static FUNCS: &[&str] = &[
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
    "idx_readonly",
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
    "objprof_get_data_extended",
    "package_exists",
    "server_warmup_status",
    "thread_mark_stack",
    "thread_memory_stats",
    "type_structure",
    "type_structure_for_alias",
    "varray",
    "vec",
    "xenon_get_data",
];

static CONSTS: &[&str] = &[];

pub static NAMESPACES: &[&str] = &["Rx"];

fn make_map(items: &[&str]) -> BTreeMap<String, String> {
    items.iter().fold(BTreeMap::new(), |mut map, s| {
        let prefix = "HH\\";
        let v = String::with_capacity(prefix.len() + s.len());
        map.insert(s.to_string(), v + prefix + s);
        map
    })
}

pub fn is_hh_autoimport(s: &str) -> bool {
    TYPES_MAP.contains_key(s)
}

pub fn is_hh_autoimport_fun(s: &str) -> bool {
    FUNCS.contains(&s)
}

#[cfg(test)]
mod tests {
    use crate::is_hh_autoimport;
    use crate::is_hh_autoimport_fun;

    #[test]
    fn test_is_hh_autoimport() {
        assert!(is_hh_autoimport("vec"));
        assert!(is_hh_autoimport("KeyedIterable"));
        assert!(!is_hh_autoimport("non-exisit"));
        assert!(is_hh_autoimport_fun("keyset"));
        assert!(!is_hh_autoimport_fun("KeyedIterable"));
    }
}
