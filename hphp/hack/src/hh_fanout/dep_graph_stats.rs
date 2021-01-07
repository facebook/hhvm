// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use log::info;

use std::collections::HashSet;
use std::ffi::OsString;

use depgraph::reader::{Dep, DepGraphOpener};
use ocamlrep_ocamlpool::ocaml_ffi;

fn main(dep_graph: OsString) {
    env_logger::init_from_env(
        env_logger::Env::default().filter_or(env_logger::DEFAULT_FILTER_ENV, "info"),
    );

    info!("Opening dependency graph at {:?}", dep_graph);
    let opener = DepGraphOpener::from_path(&dep_graph).unwrap();
    let depgraph = opener.open().unwrap();

    info!("Validating integrity of dependency graph");
    depgraph.validate_hash_lists().unwrap();

    let num_unique_hashes = depgraph.all_hashes().len();

    info!(
        "Gathering hash list stats for {} unique hashes",
        num_unique_hashes
    );
    let mut all_hash_list_pointers = HashSet::new();
    let mut num_total_edges = 0;
    let mut num_stored_edges = 0;
    for &hash in depgraph.all_hashes().iter() {
        if let Some(hash_list) = depgraph.hash_list_for(Dep::new(hash)) {
            let hash_indices = hash_list.hash_indices();
            num_total_edges += hash_list.len();
            let indices_pointer = hash_indices.as_ptr();
            if all_hash_list_pointers.insert(indices_pointer) {
                num_stored_edges += hash_list.len();
            };
        }
    }
    let num_unique_hash_lists = all_hash_list_pointers.len();

    let obj = json::object! {
        unique_hashes: num_unique_hashes,
        total_edges: num_total_edges,
        unique_hash_lists: num_unique_hash_lists,
        stored_edges: num_stored_edges,
    };
    println!("{}", json::stringify_pretty(obj, 4));
}

ocaml_ffi! {
  fn hh_fanout_dep_graph_stats_main(dep_graph: OsString) {
    main(dep_graph);
  }
}
