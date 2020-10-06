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

use depgraph::writer::{DepGraphWriter, HashListIndex};
use ocamlrep_ocamlpool::ocaml_ffi;

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

fn main(new_edges_dir: OsString, output: OsString) -> io::Result<()> {
    env_logger::init_from_env(
        env_logger::Env::default().filter_or(env_logger::DEFAULT_FILTER_ENV, "info"),
    );

    info!("Opening output file at {:?}", output);
    let f = fs::OpenOptions::new()
        .write(true)
        .create(true)
        .open(&output)?;
    let f = BufWriter::new(f);

    info!("Opening binary files in {:?}", new_edges_dir);
    let mut new_edges_dir = EdgesDir::open(new_edges_dir)?;

    info!("Discovered {} files with edges", new_edges_dir.count());
    let all_edges = new_edges_dir.read_all_edges()?;
    info!("All files loaded");

    info!("Read {} unique hashes", all_edges.count_hashes());
    info!("Read {} edges", all_edges.count_edges());

    info!("Converting to structured edges");
    let structured_edges = all_edges.structured_edges();
    info!("Getting sorted unique hashes");
    let sorted_hashes = all_edges.sorted_hashes();

    info!("Registering unique hashes");
    let mut w = DepGraphWriter::new(f, sorted_hashes)?;

    info!("Registering unique sets of dependents");
    let mut hash_list_indices: HashMap<&[u64], HashListIndex> = HashMap::new();
    let mut lookup_table: HashMap<u64, HashListIndex> = HashMap::new();
    for m in structured_edges.iter() {
        for (dependency, dependents) in m.iter() {
            let hash_list_index = hash_list_indices
                .entry(dependents)
                .or_insert_with(|| w.allocate_hash_list(dependents).unwrap());
            lookup_table.insert(*dependency, *hash_list_index);
        }
    }
    let mut w = w.finalize();

    info!("Finalizing database");
    w.register_lookup_table(lookup_table);
    w.finalize()?;

    info!("Done");
    std::process::exit(0)
}

ocaml_ffi! {
  fn hh_fanout_build_main(new_edges_dir: OsString, output: OsString) {
    main(new_edges_dir, output).unwrap();
  }
}
