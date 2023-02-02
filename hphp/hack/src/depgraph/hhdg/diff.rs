// Copyright (c) Meta Platforms, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::path::PathBuf;

use anyhow::Context;
use anyhow::Result;
use clap::Parser;
use depgraph_reader::Dep;
use depgraph_reader::DepGraph;
use hash::HashSet;
use human_readable_dep_map::HumanReadableDepMap;
use rayon::prelude::*;

/// A tool for comparing hhdg files
#[derive(Parser, Debug)]
pub struct Opts {
    /// The first .hhdg file
    dg1: PathBuf,

    /// The second .hhdg file
    dg2: PathBuf,

    /// Optional .txt files providing human-readable hash -> (kind, Dependency) pairs
    depmaps: Vec<PathBuf>,
}

/// Report nodes and edges in dg1 but not in dg2.
///
/// Returns the total number of differences found.
fn compare(dg1: &DepGraph, dg2: &DepGraph, prefix: &str, dep_map: &HumanReadableDepMap) -> usize {
    let mut num_different = 0;

    // list nodes in dg1 that are not in dg2
    for h in dg1.all_hashes() {
        if !dg2.contains(h) {
            num_different += 1;
            println!("{prefix} {h} {}", dep_map.fmt(h));
        }
    }

    // list edges in dg1 that are not in dg2
    num_different += dg1
        .par_all_hashes()
        .with_min_len(1)
        .with_max_len(1)
        .filter(|&dependency| {
            let mut different = false;

            if let Some(dg1_hash_list) = dg1.hash_list_for(dependency) {
                let dg2_hashes: HashSet<Dep> = dg2
                    .hash_list_for(dependency)
                    .map_or_else(HashSet::default, |hl| dg2.hash_list_hashes(hl).collect());

                for dependent in dg1.hash_list_hashes(dg1_hash_list) {
                    if !dg2_hashes.contains(&dependent) {
                        different = true;
                        println!(
                            "{prefix} {} -> {}",
                            dep_map.fmt(dependent),
                            dep_map.fmt(dependency)
                        );
                    }
                }
            }

            different
        })
        .count();

    num_different
}

pub(crate) fn run(opts: Opts) -> Result<usize> {
    let dg1 = DepGraph::from_path(&opts.dg1).with_context(|| opts.dg1.display().to_string())?;
    let dg2 = DepGraph::from_path(&opts.dg2).with_context(|| opts.dg2.display().to_string())?;
    let dep_map = HumanReadableDepMap::default();

    if !opts.depmaps.is_empty() {
        // Only list unknown nodes if depmaps were provided
        (opts.depmaps.par_iter())
            .map(|path| dep_map.load(path))
            .collect::<Result<_>>()?;

        // list unknown nodes in dg1
        for h in dg1.all_hashes() {
            if !dep_map.contains(h) {
                println!("{}: {h:016x} ({h}): unknown hash", opts.dg1.display());
            }
        }

        // list unknown nodes in dg2
        for h in dg2.all_hashes() {
            if !dep_map.contains(h) {
                println!("{}: {h:016x} ({h}): unknown hash", opts.dg2.display());
            }
        }
    }

    Ok(compare(&dg1, &dg2, "-", &dep_map) + compare(&dg2, &dg1, "+", &dep_map))
}
