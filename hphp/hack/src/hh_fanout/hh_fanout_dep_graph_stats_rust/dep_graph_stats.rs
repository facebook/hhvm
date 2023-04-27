// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::HashSet;
use std::ffi::OsString;

use depgraph_reader::DepGraph;
use log::info;
use ocamlrep_ocamlpool::ocaml_ffi;

fn main(dep_graph: OsString) {
    env_logger::init_from_env(
        env_logger::Env::default().filter_or(env_logger::DEFAULT_FILTER_ENV, "info"),
    );

    info!("Opening dependency graph at {:?}", dep_graph);
    let depgraph = DepGraph::from_path(&dep_graph).unwrap();

    info!("Validating integrity of dependency graph");
    depgraph.validate_hash_lists().unwrap();

    let num_unique_hashes = depgraph.all_hashes().len();

    info!(
        "Gathering hash list stats for {} unique hashes",
        num_unique_hashes
    );
    let mut all_hash_list_ids = HashSet::new();
    let mut num_total_edges = 0;
    let mut num_stored_edges = 0;
    for hash in depgraph.all_hashes() {
        if let Some(hash_list_id) = depgraph.hash_list_id_for_dep(hash) {
            let num_hash_indices = depgraph.hash_list_for_id(hash_list_id).len() as u64;
            num_total_edges += num_hash_indices;
            if all_hash_list_ids.insert(hash_list_id) {
                num_stored_edges += num_hash_indices;
            }
        }
    }
    let num_unique_hash_lists = all_hash_list_ids.len();

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
