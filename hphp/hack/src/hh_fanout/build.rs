// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use log::info;
use parking_lot::Mutex;
use rayon::prelude::*;

use std::collections::{HashMap, HashSet};
use std::ffi::OsString;
use std::io::{BufReader, BufWriter, Read};
use std::path::Path;
use std::{fs, io};

use depgraph::dep::Dep;
use depgraph::reader::{DepGraph, DepGraphOpener};
use depgraph::writer::{write_hash_list, DepGraphWriter, HashListIndex};
use deps_rust::DepGraphDelta;
use ocamlrep_ocamlpool::ocaml_ffi;

use crossbeam;
use crossbeam::channel::{bounded, Receiver, Sender};

struct EdgesDir {
    handles: Vec<BufReader<fs::File>>,
}

impl EdgesDir {
    fn open<P: AsRef<Path>>(dir: P) -> io::Result<EdgesDir> {
        let handles: io::Result<Vec<_>> = fs::read_dir(dir)?
            .map(|res| {
                res.and_then(|entry| {
                    let path = entry.path();
                    if path.extension().and_then(|x| x.to_str()) == Some(&"bin") {
                        let fh = fs::OpenOptions::new().read(true).open(&entry.path())?;
                        Ok(Some(BufReader::new(fh)))
                    } else {
                        Ok(None)
                    }
                })
            })
            .filter_map(|x| match x {
                Err(x) => Some(Err(x)),
                Ok(Some(x)) => Some(Ok(x)),
                Ok(None) => None,
            })
            .collect();

        let handles = handles?;
        Ok(EdgesDir { handles })
    }

    fn count(&self) -> usize {
        self.handles.len()
    }

    fn read_all_edges(&mut self) -> io::Result<Edges> {
        let acc = Edges::new();
        let num_handles = self.count();
        let result: io::Result<Vec<()>> = self
            .handles
            .par_iter_mut()
            .enumerate()
            .map(|(i, handle)| {
                info!("Reading in file {}/{}", i + 1, num_handles);
                Self::read_all_edges_for(handle, &acc)?;
                Ok(())
            })
            .collect();
        let _ = result?;
        Ok(acc)
    }

    fn read_all_edges_for<R: Read>(reader: &mut R, acc: &Edges) -> io::Result<()> {
        let mut bytes: [u8; 8] = [0; 8];
        loop {
            if let Err(err) = reader.read_exact(&mut bytes) {
                if err.kind() == io::ErrorKind::UnexpectedEof {
                    return Ok(());
                }
                return Err(err);
            }
            let dependent = u64::from_be_bytes(bytes);

            reader.read_exact(&mut bytes).expect("a dependency hash");
            let dependency = u64::from_be_bytes(bytes);

            acc.register(dependent, dependency);
        }
    }
}

const NUM_BUCKETS: usize = 2048;
const BUCKET_INDEX_MASK: u64 = (NUM_BUCKETS - 1) as u64;

struct Edges {
    edges: Vec<Mutex<HashMap<u64, HashSet<u64>>>>,
    hashes: Vec<Mutex<HashSet<u64>>>,
}

impl Edges {
    pub fn new() -> Self {
        Edges {
            edges: std::iter::repeat_with(|| Mutex::new(HashMap::new()))
                .take(NUM_BUCKETS)
                .collect(),
            hashes: std::iter::repeat_with(|| Mutex::new(HashSet::new()))
                .take(NUM_BUCKETS)
                .collect(),
        }
    }

    pub fn register(&self, dependent: u64, dependency: u64) {
        self.register_hash(dependent);
        self.register_hash(dependency);
        let bucket_index = (dependency & BUCKET_INDEX_MASK) as usize;
        let bucket_mutex = self.edges.get(bucket_index).unwrap();
        bucket_mutex
            .lock()
            .entry(dependency)
            .and_modify(|s| {
                s.insert(dependent);
            })
            .or_insert_with(|| {
                let mut s = HashSet::new();
                s.insert(dependent);
                s
            });
    }

    fn register_hash(&self, hash: u64) {
        let bucket_index = (hash & BUCKET_INDEX_MASK) as usize;
        let bucket_mutex = self.hashes.get(bucket_index).unwrap();
        bucket_mutex.lock().insert(hash);
    }

    pub fn structured_edges(&self) -> Vec<HashMap<u64, Vec<u64>>> {
        self.edges
            .par_iter()
            .map(|old_map| {
                let mut new_map = HashMap::new();
                for (dependency, dependents) in old_map.lock().iter() {
                    let mut dependents: Vec<u64> = dependents.iter().copied().collect();
                    dependents.sort();
                    new_map.insert(*dependency, dependents);
                }
                new_map
            })
            .collect()
    }

    pub fn sorted_hashes(&self) -> Vec<u64> {
        let mut hashes: Vec<u64> = Vec::with_capacity(self.count_hashes());
        for hash_set in self.hashes.iter() {
            hash_set.lock().iter().copied().for_each(|h| hashes.push(h));
        }
        hashes.sort();
        hashes
    }

    pub fn count_edges(&self) -> usize {
        self.edges
            .iter()
            .map(|bucket_mutex| {
                bucket_mutex
                    .lock()
                    .iter()
                    .map(|(_, xs)| xs.len())
                    .sum::<usize>()
            })
            .sum()
    }

    pub fn count_hashes(&self) -> usize {
        self.hashes
            .iter()
            .map(|bucket_mutex| bucket_mutex.lock().len())
            .sum()
    }
}

/// Extend a collection of edges by adding all edges from the given
/// dependency graph.
fn extend_edges_from_dep_graph(all_edges: &Edges, graph: &DepGraph<'_>) {
    let all_hashes = graph.all_hashes();

    all_hashes.par_iter().for_each(|dependency| {
        let dependency = Dep::new(*dependency);
        if let Some(hash_list) = graph.hash_list_for(dependency) {
            for dependent in graph.hash_list_hashes(hash_list) {
                all_edges.register(dependent.into(), dependency.into());
            }
        }
    });
}

fn main(
    incremental: Option<OsString>,
    new_edges_dir: Option<OsString>,
    delta_file: Option<OsString>,
    output: OsString,
) -> io::Result<()> {
    env_logger::init_from_env(
        env_logger::Env::default().filter_or(env_logger::DEFAULT_FILTER_ENV, "info"),
    );

    let all_edges = match (new_edges_dir, delta_file) {
        (None, None) => {
            panic!("buid: at least one of --edges-dir or --delta-file flags should be passed")
        }
        (Some(_), Some(_)) => {
            panic!("build: cannot specify both --edges-dir and --delta-file")
        }
        (Some(new_edges_dir), None) => {
            info!("Opening binary files in {:?}", new_edges_dir);
            let mut new_edges_dir = EdgesDir::open(new_edges_dir)?;

            info!("Discovered {} files with edges", new_edges_dir.count());
            let all_edges = new_edges_dir.read_all_edges()?;
            info!(
                "All binary files loaded ({} unique edges)",
                all_edges.count_edges()
            );
            all_edges
        }
        (None, Some(delta_file)) => {
            info!("Opening dep graph delta at {:?}", delta_file);
            let f = std::fs::OpenOptions::new().read(true).open(&delta_file)?;
            let mut r = std::io::BufReader::new(f);
            let mut delta = DepGraphDelta::new();
            let num_read = delta.read_from(&mut r, |_, _| true)?;
            info!("Delta loaded with {} edges", num_read);

            let all_edges = Edges::new();
            for (dependent, dependency) in delta.iter() {
                all_edges.register(dependent.into(), dependency.into());
            }
            all_edges
        }
    };

    info!("Opening output file at {:?}", output);
    let f = fs::OpenOptions::new()
        .write(true)
        .create(true)
        .open(&output)?;
    let f = BufWriter::new(f);

    match incremental {
        None => info!("Not reading in edges from previous dependency graph (incremental=None)"),
        Some(incremental) => {
            info!(
                "Reading in edges from previous dependency graph at {:?}",
                incremental
            );
            let old_dep_graph_opener = DepGraphOpener::from_path(&incremental)?;
            let old_dep_graph = old_dep_graph_opener.open().unwrap();
            extend_edges_from_dep_graph(&all_edges, &old_dep_graph);
            info!("Done reading in old edges");
        }
    }


    info!(
        "Building graph with {} unique hashes",
        all_edges.count_hashes()
    );
    info!(
        "            ... and {} unique edges",
        all_edges.count_edges()
    );

    info!("Converting to structured edges");
    let structured_edges = all_edges.structured_edges();
    info!("Getting sorted unique hashes");
    let sorted_hashes = all_edges.sorted_hashes();

    info!("Registering unique hashes");
    let mut w = DepGraphWriter::new(f, sorted_hashes)?;

    info!("Registering unique sets of dependents");
    let mut hash_list_indices: HashMap<&[u64], HashListIndex> = HashMap::new();
    let mut lookup_table: HashMap<u64, HashListIndex> = HashMap::new();

    let thread_num = 16;
    let (tx, rx): (
        Sender<(&[u64], HashListIndex)>,
        Receiver<(&[u64], HashListIndex)>,
    ) = bounded(2 * thread_num);

    let indexer = w.get_indexer();
    info!("Finished indexer clone");
    crossbeam::scope(|s| {
        for _n in 0..thread_num {
            s.spawn(|_| {
                let ff = fs::OpenOptions::new().write(true).open(&output).unwrap();
                let mut ff = BufWriter::new(ff);
                let thread_rx = rx.clone();
                for d in thread_rx.iter() {
                    write_hash_list(&indexer, &mut ff, d.1, &*d.0).unwrap();
                }
            });
        }

        for m in structured_edges.iter() {
            for (dependency, dependents) in m.iter() {
                let hash_list_index = hash_list_indices.entry(dependents).or_insert_with(|| {
                    let h = w.allocate_hash_list(dependents).unwrap();
                    tx.send((dependents, h)).unwrap();
                    h
                });
                lookup_table.insert(*dependency, *hash_list_index);
            }
        }
        drop(tx);
    })
    .unwrap();

    let mut w = w.finalize();

    info!("Finalizing database");
    w.register_lookup_table(lookup_table);
    w.finalize()?;

    info!("Done");
    std::process::exit(0)
}

ocaml_ffi! {
  fn hh_fanout_build_main(
    incremental: Option<OsString>,
    new_edges_dir: Option<OsString>,
    delta_file: Option<OsString>,
    output: OsString,
  ) {
    main(incremental, new_edges_dir, delta_file, output).unwrap();
  }
}
