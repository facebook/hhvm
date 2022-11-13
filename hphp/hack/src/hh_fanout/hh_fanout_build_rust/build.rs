// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
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
use hash::HashMap;
use hash::HashSet;
use hash::IndexMap;
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

/// Structure used to read in all edges in parallel.
///
/// Basically a shared HashMap+HashSet
struct Edges {
    /// edges is a sharded map with NUM_BUCKETS entries. It is indexed
    /// by dependency % NUM_BUCKETS. The same dependency cannot appear in more
    /// than one buckets; the inner HashMaps have disjoint keys.
    edges: Vec<Mutex<HashMap<u64, HashSet<u64>>>>,

    /// hashes is a sharded set with NUM_BUCKETS entries. It is indexed by both
    /// dependency and dependent u64 values. The inner HashSet have disjoint
    /// values.
    ///
    /// The u64 values stored here are used by DepGraphWriter to build a
    /// hash-cons table so unique u64 values can be referred to using
    /// an arbitrary u32 index.
    hashes: Vec<Mutex<HashSet<u64>>>,
}

impl Edges {
    pub fn new() -> Self {
        Edges {
            edges: std::iter::repeat_with(Default::default)
                .take(NUM_BUCKETS)
                .collect(),
            hashes: std::iter::repeat_with(Default::default)
                .take(NUM_BUCKETS)
                .collect(),
        }
    }

    pub fn register(&self, dependent: u64, dependency: u64) {
        self.register_hash(dependent);
        self.register_hash(dependency);
        let bucket_index = dependency as usize % NUM_BUCKETS;
        let bucket_mutex = self.edges.get(bucket_index).unwrap();
        bucket_mutex
            .lock()
            .entry(dependency)
            .or_default()
            .insert(dependent);
    }

    fn register_hash(&self, hash: u64) {
        let bucket_index = hash as usize % NUM_BUCKETS;
        let bucket_mutex = self.hashes.get(bucket_index).unwrap();
        bucket_mutex.lock().insert(hash);
    }

    pub fn finish(self) -> (Vec<HashMap<u64, Vec<u64>>>, Vec<u64>) {
        let num_hashes = self.count_hashes();
        let edges = (self.edges.into_par_iter())
            .map(|old_map| {
                let mut new_map = HashMap::default();
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
            .collect();
        let mut hashes: Vec<u64> = Vec::with_capacity(num_hashes);
        for hash_set in self.hashes {
            hashes.extend(hash_set.into_inner())
        }
        hashes.par_sort_unstable();
        (edges, hashes)
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

/// Structure used to calculate hash lists in parallel.
///
/// Each bucket keeps track of a map of sorted_dependents to:
///   1. internal hash list offset (set in make_absolute())
///   2. a list of dependencies that use this sorted_dependents
struct HashListsIndices<'a> {
    buckets: Vec<Mutex<IndexMap<&'a [u64], (HashListIndex, Vec<u64>)>>>,
}

impl<'a> HashListsIndices<'a> {
    fn new() -> Self {
        Self {
            buckets: std::iter::repeat_with(Default::default)
                .take(NUM_BUCKETS)
                .collect(),
        }
    }

    fn allocate_hash_list(&self, dependency: u64, sorted_dependents: &'a [u64]) {
        let mut hasher = hash::BuildHasher::default().build_hasher();
        sorted_dependents.hash(&mut hasher);
        let bucket_index = hasher.finish() as usize % NUM_BUCKETS;
        let mut bucket = self.buckets[bucket_index].lock();
        let (_, dependencies) = bucket
            .entry(sorted_dependents)
            .or_insert_with(|| (HashListIndex(0), Vec::new()));
        dependencies.push(dependency);
    }

    /// Assign deterministic offsets to every key in HashListIndicesBucket::hash_list_indices.
    /// The contents of each bucket are determinstic because keys were assigned to buckets
    /// using a determistic hash. But the order of keys in each bucket is nondeterministic because
    /// they were inserted in parallel.
    ///
    /// for each bucket in parallel:
    ///   * sort the &[u64] keys (sorted_dependents) of each bucket lexographically.
    ///   * assign each key a relative offset (starting from 0) within this bucket
    ///   * compute the total size of the bucket
    ///   * while we're here, sort the dependents list for each key
    ///
    /// Then sequentially for each bucket:
    ///   * assign the bucket a starting offset
    ///   * adjust the next starting offset by this bucket's size.
    ///
    /// Now every bucket and every (sorted_dependents, dependencies) pair
    /// has a deterministic offset and is in deterministic order.
    fn make_absolute(&self, mut cur_offset: u32) -> (Vec<u32>, u32) {
        let bucket_sizes: Vec<u32> = (self.buckets.par_iter())
            .map(|m| {
                let mut bucket = m.lock();
                let mut size = 0;
                bucket.sort_unstable_keys();
                for (sorted_dependents, (offset, dependencies)) in bucket.iter_mut() {
                    *offset = HashListIndex(size);
                    size += DepGraphWriter::size_needed_for_hash_list(sorted_dependents);
                    dependencies.sort_unstable();
                }
                size
            })
            .collect();
        let bucket_offsets: Vec<u32> = (bucket_sizes.into_iter())
            .map(|size| {
                let offset = cur_offset;
                cur_offset += size;
                offset
            })
            .collect();
        (bucket_offsets, cur_offset)
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

/// Reads all the edges from the edges files in `edges_dir` and collects them in
/// a list of hashmaps of dependency to its dependents.
pub fn edges_from_dir(edges_dir: &Path) -> io::Result<Vec<HashMap<u64, Vec<u64>>>> {
    let mut edges_dir = EdgesDir::open(edges_dir)?;
    let edges = edges_dir.read_all_edges()?;
    let (structured, _) = edges.finish();
    Ok(structured)
}

pub fn build(
    allow_empty: bool,
    incremental: Option<OsString>,
    new_edges_dir: Option<OsString>,
    delta_file: Option<OsString>,
    output: OsString,
) -> io::Result<()> {
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

    info!("Converting to structured_edges & unique hashes");
    let (structured_edges, sorted_hashes) = all_edges.finish();

    info!("Registering unique hashes");
    let w = DepGraphWriter::new(sorted_hashes)?;

    info!("Calculating relative hash list offsets");
    let hash_list_indices = HashListsIndices::new();
    structured_edges.par_iter().for_each(|m| {
        for (dependency, sorted_dependents) in m.iter() {
            hash_list_indices.allocate_hash_list(*dependency, sorted_dependents);
        }
    });

    info!("Calculating absolute hash list offsets");
    let (bucket_offsets, next_set_offset) =
        hash_list_indices.make_absolute(w.first_hash_list_offset());

    info!("Cloning out indexer");
    let mut w = w.open_writer(&f, HashListIndex(next_set_offset))?;
    let indexer = w.clone_indexer();

    info!("Writing hash lists to database and constructing lookup table");
    let lookup_table = ShardedLookupTableWriter::new();
    let mmap = w.sharded_mmap();
    (hash_list_indices.buckets.into_par_iter())
        .zip(bucket_offsets)
        .for_each(|(m, offset)| {
            let bucket = m.into_inner();
            for (dependents, (hash_list_index, dependencies)) in bucket {
                let absolute_hash_list_index = hash_list_index.incr(offset);
                DepGraphWriter::write_hash_list(
                    &mmap,
                    &indexer,
                    absolute_hash_list_index,
                    dependents,
                )
                .unwrap();
                for dependency in dependencies {
                    lookup_table.insert(dependency, absolute_hash_list_index);
                }
            }
        });

    structured_edges.into_par_iter().for_each(drop);

    info!("Writing lookup table");
    let mut w = w.finalize();
    w.register_lookup_table(lookup_table)?;

    info!("Writing indexer and syncing to disk");
    w.write_indexer_and_finalize()?;

    info!("Done");
    Ok(())
}
