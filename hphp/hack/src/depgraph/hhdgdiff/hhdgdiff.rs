// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fs::File;
use std::io::BufRead;
use std::io::BufReader;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Context;
use anyhow::Result;
use clap::Parser;
use depgraph_reader::Dep;
use depgraph_reader::DepGraphOpener;
use hash::DashMap;
use rayon::prelude::*;
use typing_deps_hash::DepType;

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

fn main() -> Result<()> {
    let opts = Opts::parse();
    let opener = DepGraphOpener::from_path(&opts.dg1)?;
    let dg1 = (opener.open())
        .map_err(|s| anyhow::anyhow!("could not open: {s}"))
        .with_context(|| opts.dg1.display().to_string())?;
    let opener = DepGraphOpener::from_path(&opts.dg2)?;
    let dg2 = (opener.open())
        .map_err(|s| anyhow::anyhow!("could not open: {s}"))
        .with_context(|| opts.dg2.display().to_string())?;
    let nodes = Nodes::default();

    if !opts.depmaps.is_empty() {
        // Only list unknown nodes if depmaps were provided
        (opts.depmaps.par_iter())
            .map(|path| nodes.load(path))
            .collect::<Result<_>>()?;

        // list unknown nodes in dg1
        for &h in dg1.all_hashes().iter() {
            if !nodes.map.contains_key(&Dep::new(h)) {
                println!("{}: {h:016x} ({h}): unknown hash", opts.dg1.display());
            }
        }

        // list unknown nodes in dg2
        for &h in dg2.all_hashes().iter() {
            if !nodes.map.contains_key(&Dep::new(h)) {
                println!("{}: {h:016x} ({h}): unknown hash", opts.dg2.display());
            }
        }
    }

    let mut different = 0;

    // list nodes in dg1 that are not in dg2
    for &h in dg1.all_hashes().iter() {
        if dg2.all_hashes().binary_search(&h).is_err() {
            different += 1;
            println!("- {} {}", h, nodes.fmt(Dep::new(h)));
        }
    }

    // list edges in dg1 that are not in dg2
    different += (dg1.all_hashes().par_iter())
        .with_min_len(1)
        .with_max_len(1)
        .filter(|&&h| {
            let mut different = false;
            let dependency = Dep::new(h);
            if let Some(list) = dg1.hash_list_for(dependency) {
                for dependent in dg1.hash_list_hashes(list) {
                    if !dg2.dependent_dependency_edge_exists(dependent, dependency) {
                        different = true;
                        println!("- {} -> {}", nodes.fmt(dependent), nodes.fmt(dependency));
                    }
                }
            }
            different
        })
        .count();

    // list nodes in dg2 that are not in dg1
    for &h in dg2.all_hashes().iter() {
        if dg1.all_hashes().binary_search(&h).is_err() {
            different += 1;
            println!("+ {} {}", h, nodes.fmt(Dep::new(h)));
        }
    }

    // list edges in dg2 that are not in dg1
    different += (dg2.all_hashes().par_iter())
        .with_min_len(1)
        .with_max_len(1)
        .filter(|&&h| {
            let mut different = false;
            let dependency = Dep::new(h);
            if let Some(list) = dg2.hash_list_for(dependency) {
                for dependent in dg2.hash_list_hashes(list) {
                    if !dg1.dependent_dependency_edge_exists(dependent, dependency) {
                        different = true;
                        println!("+ {} -> {}", nodes.fmt(dependent), nodes.fmt(dependency));
                    }
                }
            }
            different
        })
        .count();

    if different != 0 {
        std::process::exit(1);
    }
    Ok(())
}

#[derive(Default)]
struct Nodes {
    map: DashMap<Dep, (DepType, String)>,
}

impl Nodes {
    fn load(&self, path: &Path) -> Result<()> {
        for line in BufReader::new(File::open(path)?).lines() {
            let line = line?;
            let mut parts = line.split(' ');
            let hash: u64 = match parts.next() {
                Some(s) => s.parse()?,
                None => anyhow::bail!("expected hash"),
            };
            let kind: DepType = match parts.next() {
                Some(s) => match s {
                    "GConst" => DepType::GConst,
                    "Fun" => DepType::Fun,
                    "Type" => DepType::Type,
                    "Extends" => DepType::Extends,
                    "Const" => DepType::Const,
                    "Constructor" => DepType::Constructor,
                    "Prop" => DepType::Prop,
                    "SProp" => DepType::SProp,
                    "Method" => DepType::Method,
                    "SMethod" => DepType::SMethod,
                    "AllMembers" => DepType::AllMembers,
                    "GConstName" => DepType::GConstName,
                    "Module" => DepType::Module,
                    bad => anyhow::bail!("unexpected DepType {}", bad),
                },
                None => anyhow::bail!("expected DepType"),
            };
            let sym: &str = match parts.next() {
                Some(s) => s,
                None => anyhow::bail!("expected symbol"),
            };
            match self.map.insert(Dep::new(hash), (kind, sym.into())) {
                Some((old_kind, old_sym)) if old_kind != kind || old_sym != sym => {
                    println!(
                        "{}: collision: ({:?},{}) != ({:?},{})",
                        hash, old_kind, old_sym, kind, sym
                    );
                }
                _ => {}
            }
        }
        Ok(())
    }

    fn fmt(&self, dep: Dep) -> String {
        match self.map.get(&dep) {
            Some(e) => {
                let (kind, sym) = &*e;
                format!("{kind:?} {sym}")
            }
            None => {
                format!("{dep:016x} ({dep})")
            }
        }
    }
}
