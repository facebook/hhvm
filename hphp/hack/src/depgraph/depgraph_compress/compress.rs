// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use depgraph_writer::MemDepGraph;

use crate::*;

/// Write a `MemDepGraph` to disk, optionally optimizing it first.
pub fn write_dep_graph(
    path: &Path,
    mut m: MemDepGraph,
    write_config: &WriteConfig,
    optimize_config: &OptimizeConfig,
) -> std::io::Result<()> {
    match optimize_config {
        OptimizeConfig::Bisect(config) => {
            let mut tg = transpose::transpose(&m);
            balanced_partition::optimize_doc_order(&mut tg.docs, config);
            renumber::apply_node_renumbering(&mut m, tg);
        }
        OptimizeConfig::Copy(path) => copy::copy_node_order(&mut m, path)?,
        OptimizeConfig::None => log::info!("Skipping graph compression"),
    }

    write::write_to_disk(path, m, write_config)
}
