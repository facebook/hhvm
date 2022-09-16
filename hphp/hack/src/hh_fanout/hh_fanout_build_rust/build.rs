// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::HashMap;
use std::collections::HashSet;
use std::ffi::OsString;
use std::fs;
use std::hash::BuildHasher;
use std::hash::Hash;
use std::hash::Hasher;
use std::io;
use std::io::BufReader;
use std::io::Read;
use std::path::Path;

use dep_graph_delta::DepGraphDelta;
use depgraph::dep::Dep;
use depgraph::reader::DepGraph;
use depgraph::reader::DepGraphOpener;
use depgraph_writer::DepGraphWriter;
use depgraph_writer::HashListIndex;
use depgraph_writer::ShardedLookupTableWriter;
use log::info;
use parking_lot::Mutex;
use rayon::prelude::*;

struct EdgesDir {
    // For local typechecks, we output edges in a list format
    list_handles: Vec<BufReader<fs::File>>,
    // For remote typechecks, we output edges in a serialized rust structure format
    struct_handles: Vec<OsString>,
}

impl EdgesDir {
    fn open<P: AsRef<Path>>(dir: P) -> io::Result<EdgesDir> {
        let list_handles: io::Result<Vec<_>> = fs::read_dir(&dir)?
            .map(|res| {
                res.and_then(|entry| {
                    let path = entry.path();
                    if path.extension().and_then(|x| x.to_str()) == Some("bin") {
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

        let struct_handles: io::Result<Vec<_>> = fs::read_dir(&dir)?
            .map(|res| {
                res.map(|entry| {
                    let path = entry.path();
                    if path.extension().and_then(|x| x.to_str()) == Some("hhdg_delta") {
                        Some(OsString::from(&entry.path()))
                    } else {
                        None
                    }
                })
            })
            .filter_map(|x| match x {
                Err(x) => Some(Err(x)),
                Ok(Some(x)) => Some(Ok(x)),
                Ok(None) => None,
            })
            .collect();

        Ok(EdgesDir {
            list_handles: list_handles?,
            struct_handles: struct_handles?,
        })
    }

    fn list_handle_count(&self) -> usize {
        self.list_handles.len()
    }

    fn struct_handle_count(&self) -> usize {
        self.struct_handles.len()
    }

    fn read_all_edges(&mut self) -> io::Result<Edges> {
        let acc = Edges::new();
        let num_list_handles = self.list_handle_count();
        let num_struct_handles = self.struct_handle_count();
        let list_result: io::Result<Vec<()>> = self
            .list_handles
            .par_iter_mut()
            .enumerate()
            .map(|(i, handle)| {
                info!("Reading in list file {}/{}", i + 1, num_list_handles);
                Self::read_all_list_edges_for(handle, &acc)?;
                Ok(())
            })
            .collect();
        let struct_result: io::Result<Vec<()>> = self
            .struct_handles
            .par_iter()
            .enumerate()
            .map(|(i, handle)| {
                info!("Reading in struct file {}/{}", i + 1, num_struct_handles);
                Self::read_all_struct_edges_for(handle, &acc)?;
                Ok(())
            })
            .collect();
        let _ = (list_result?, struct_result?);
        Ok(acc)
    }

    fn read_all_list_edges_for<R: Read>(reader: &mut R, acc: &Edges) -> io::Result<()> {
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

    fn read_all_struct_edges_for(source: &OsString, acc: &Edges) -> io::Result<()> {
        let f = std::fs::OpenOptions::new()
            .read(true)
            .open(&source)
            .unwrap();

        let mut r = std::io::BufReader::new(f);

        let mut delta = DepGraphDelta::new();
        delta.read_from(&mut r, |_, _| true).unwrap();

        for (&dependency, dependents) in delta.0.iter() {
            for &dependent in dependents.iter() {
                // To be kept in sync with Typing_deps.ml::filter_discovered_deps_batch
                let dependent: u64 = dependent.into();
                let dependency: u64 = dependency.into();
                acc.register(dependent, dependency);
            }
        }

        Ok(())
    }
}

const NUM_BUCKETS: usize = 2048;
const BUCKET_INDEX_MASK: u64 = (NUM_BUCKETS - 1) as u64;

/// Structure used to read in all edges in parallel.
///
/// Basically a shared HashMap+HashSet
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
                    let mut dependents_vec: Vec<u64> = Vec::with_capacity(dependents.len());
                    for x in dependents.iter() {
                        dependents_vec.push(*x);
                    }
                    dependents_vec.sort_unstable();
                    new_map.insert(*dependency, dependents_vec);
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
        hashes.sort_unstable();
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

/// Structure used to calculate hash list offsets in parallel.
///
/// Each bucket keeps track of
///   - an internal offset `next_offset` that is incremented
///     when reserving space for a hash list.
///   - a map of hash list to
///       1. internal hash list offset
///       2. a list of dependencies that use this hash list
///   - a bucket offset which is only calculated after the fact
///     when all buckets have been processed.
struct HashListsIndicesBucket<'a> {
    next_offset: HashListIndex,
    hash_list_indices: HashMap<&'a [u64], (HashListIndex, Vec<u64>)>,
    bucket_offset: u32,
}

impl HashListsIndicesBucket<'_> {
    fn with_hasher(hasher: std::collections::hash_map::RandomState) -> Self {
        Self {
            next_offset: HashListIndex(0),
            hash_list_indices: HashMap::with_hasher(hasher),
            bucket_offset: 0,
        }
    }
}

/// Allocates hash lists and populates the lookup table
struct HashListsIndices<'a> {
    buckets: Vec<Mutex<HashListsIndicesBucket<'a>>>,
    hash_builder: std::collections::hash_map::RandomState,
}

impl<'a> HashListsIndices<'a> {
    fn new() -> Self {
        let hash_builder = std::collections::hash_map::RandomState::new();
        Self {
            hash_builder: hash_builder.clone(),
            buckets: std::iter::repeat_with(|| {
                Mutex::new(HashListsIndicesBucket::with_hasher(hash_builder.clone()))
            })
            .take(NUM_BUCKETS)
            .collect(),
        }
    }

    fn hash_u64(&self, key: &[u64]) -> u64 {
        let mut hasher = self.hash_builder.build_hasher();
        key.hash(&mut hasher);
        hasher.finish() as u64
    }

    fn allocate_hash_list(&self, dependency: u64, hash_list: &'a [u64]) {
        let hash = self.hash_u64(hash_list);
        let bucket_index = (hash & BUCKET_INDEX_MASK) as usize;
        let mut bucket = self.buckets.get(bucket_index).unwrap().lock();

        let mut new_next_offset = bucket.next_offset;
        bucket
            .hash_list_indices
            .entry(hash_list)
            .and_modify(|(_, dependencies)| dependencies.push(dependency))
            .or_insert_with(|| {
                let size = DepGraphWriter::size_needed_for_hash_list(hash_list);
                let offset = new_next_offset;
                new_next_offset = offset.incr(size);
                let dependencies = vec![dependency];
                (offset, dependencies)
            });
        bucket.next_offset = new_next_offset;
    }

    fn make_absolute(&self, first_set_offset: u32) -> HashListIndex {
        let mut bucket_offset: u32 = first_set_offset;
        for lck in self.buckets.iter() {
            let mut bucket = lck.lock();
            bucket.bucket_offset = bucket_offset;
            bucket_offset += bucket.next_offset.0;
        }
        HashListIndex(bucket_offset)
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

pub fn build(
    allow_empty: bool,
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
            panic!("build: at least one of --edges-dir or --delta-file flags should be passed")
        }
        (Some(_), Some(_)) => {
            panic!("build: cannot specify both --edges-dir and --delta-file")
        }
        (Some(new_edges_dir), None) => {
            info!("Opening binary files in {:?}", new_edges_dir);
            let mut new_edges_dir = EdgesDir::open(new_edges_dir)?;

            info!(
                "Discovered {} list files with edges",
                new_edges_dir.list_handle_count()
            );
            info!(
                "Discovered {} struct files with edges",
                new_edges_dir.struct_handle_count()
            );
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
        .read(true)
        .write(true)
        .create(true)
        .open(&output)?;

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

    if !allow_empty && all_edges.count_edges() == 0 {
        panic!("No input edges. Refusing to build as --allow-empty not set.");
    }

    info!("Converting to structured edges");
    let structured_edges = all_edges.structured_edges();
    info!("Getting sorted unique hashes");
    let sorted_hashes = all_edges.sorted_hashes();

    info!("Registering unique hashes");
    let w = DepGraphWriter::new(sorted_hashes)?;

    info!("Calculating relative hash list offsets");
    let hash_list_indices = HashListsIndices::new();
    structured_edges.par_iter().for_each(|m| {
        for (dependency, dependents) in m.iter() {
            hash_list_indices.allocate_hash_list(*dependency, dependents);
        }
    });

    info!("Calculating absolute hash list offsets");
    let next_set_offset = hash_list_indices.make_absolute(w.first_hash_list_offset());

    info!("Cloning out indexer");
    let mut w = w.open_writer(&f, next_set_offset)?;
    let indexer = w.clone_indexer();

    info!("Writing hash lists to database and constructing lookup table");
    let lookup_table = ShardedLookupTableWriter::new();
    let mmap = w.sharded_mmap();
    hash_list_indices.buckets.par_iter().for_each(|lck| {
        let bucket = lck.lock();
        for (dependents, (hash_list_index, dependencies)) in bucket.hash_list_indices.iter() {
            let absolute_hash_list_index = hash_list_index.incr(bucket.bucket_offset);
            DepGraphWriter::write_hash_list(&mmap, &indexer, absolute_hash_list_index, dependents)
                .unwrap();
            for dependency in dependencies {
                lookup_table.insert(*dependency, absolute_hash_list_index);
            }
        }
    });

    info!("Writing lookup table");
    let mut w = w.finalize();
    w.register_lookup_table(lookup_table)?;

    info!("Writing indexer and syncing to disk");
    w.write_indexer_and_finalize()?;

    info!("Done");
    Ok(())
}
