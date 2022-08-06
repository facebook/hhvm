// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ffi::OsString;
use std::io;

use depgraph::reader::Dep;
use depgraph::reader::DepGraph;
use depgraph::reader::DepGraphOpener;
use log::info;
use ocamlrep_ocamlpool::ocaml_ffi;

struct MissingEdge {
    dependent: Dep,
    dependency: Dep,
}

fn find_missing_edge(sub_graph: &DepGraph<'_>, super_graph: &DepGraph<'_>) -> Option<MissingEdge> {
    let sub_hashes = sub_graph.all_hashes();
    for dependency in sub_hashes {
        let dependency = Dep::new(*dependency);
        if let Some(hash_list) = sub_graph.hash_list_for(dependency) {
            for dependent in sub_graph.hash_list_hashes(hash_list) {
                if !super_graph.dependent_dependency_edge_exists(dependent, dependency) {
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
