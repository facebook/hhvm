// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::HashSet;
use std::ffi::OsString;
use std::io;

use depgraph_reader::Dep;
use depgraph_reader::DepGraph;
use depgraph_reader::DepGraphOpener;
use log::info;
use ocamlrep_ocamlpool::ocaml_ffi;

struct MissingEdge {
    dependent: Dep,
    dependency: Dep,
}

fn find_missing_edge(sub_graph: &DepGraph<'_>, super_graph: &DepGraph<'_>) -> Option<MissingEdge> {
    // TODO: This could be much faster.
    //
    // 1. Use rayon.
    // 2. Use `hash_list_id_for_dep` instead of `hash_list_for`. That returns a token that uniquely identifies
    //    the physical `HashList`, and identical rows share the same `HashListId`. Duplicates are common.
    //    Then use a table (say, `DashSet`) to remember which pairs of `HashListId` values have already been
    //    compared, and only check each pair once. If not in the table, `hash_list_for_id` can efficiently map
    //    the `HashListId` to a `HashList`.
    let sub_hashes = sub_graph.all_hashes();
    for dependency in sub_hashes {
        if let Some(sub_hash_list) = sub_graph.hash_list_for(dependency) {
            let super_hashes: HashSet<Dep> = super_graph
                .hash_list_for(dependency)
                .map_or_else(HashSet::default, |hl| {
                    super_graph.hash_list_hashes(hl).collect()
                });

            for dependent in sub_graph.hash_list_hashes(sub_hash_list) {
                if !super_hashes.contains(&dependent) {
                    return Some(MissingEdge {
                        dependent,
                        dependency,
                    });
                }
            }
        }
    }
    None
}

fn main(sub_graph: OsString, super_graph: OsString) -> io::Result<()> {
    env_logger::init_from_env(
        env_logger::Env::default().filter_or(env_logger::DEFAULT_FILTER_ENV, "info"),
    );

    info!("Opening sub-graph at {:?}", sub_graph);
    let sub_opener = DepGraphOpener::from_path(&sub_graph)?;
    let sub_graph = sub_opener.open().unwrap();

    info!("Opening super-graph at {:?}", super_graph);
    let super_opener = DepGraphOpener::from_path(&super_graph)?;
    let super_graph = super_opener.open().unwrap();

    match find_missing_edge(&sub_graph, &super_graph) {
        None => println!("OK"),
        Some(MissingEdge {
            dependent,
            dependency,
        }) => {
            let dependent: u64 = dependent.into();
            let dependency: u64 = dependency.into();
            println!("FAIL: the following dependent -> dependency edge is missing:");
            println!("      {:x} -> {:x}", dependent, dependency);
            std::process::exit(1)
        }
    }

    Ok(())
}

ocaml_ffi! {
  fn hh_fanout_dep_graph_is_subgraph_main(
    sub_graph: OsString,
    super_graph: OsString,
  ) {
    main(sub_graph, super_graph).unwrap();
  }
}
