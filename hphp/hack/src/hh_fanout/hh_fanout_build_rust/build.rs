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
use std::path::PathBuf;

use dep_graph_delta::DepGraphDelta;
use depgraph_reader::DepGraph;
use depgraph_reader::DepGraphOpener;
use depgraph_writer::DepGraphWriter;
use depgraph_writer::HashListIndex;
use depgraph_writer::LookupTableWriter;
use hash::DashMap;
use hash::DashSet;
use hash::HashSet;
use hash::IndexMap;
use log::info;
use parking_lot::Mutex;
use rayon::prelude::*;

struct EdgesDir {
    // For local typechecks, we output edges in a list format
    list_handles: Vec<BufReader<fs::File>>,
    // For remote typechecks, we output edges in a serialized rust structure format
    struct_handles: Vec<PathBuf>,
}

impl EdgesDir {
    fn open<P: AsRef<Path>>(dir: P) -> io::Result<EdgesDir> {
        let list_handles = fs::read_dir(&dir)?
            .map(|entry| {
                let path = entry?.path();
                if path.extension().and_then(|x| x.to_str()) == Some("bin") {
                    let fh = fs::OpenOptions::new().read(true).open(path)?;
                    Ok(Some(BufReader::new(fh)))
                } else {
                    Ok(None)
                }
            })
            .filter_map(|x| x.transpose())
            .collect::<io::Result<Vec<_>>>()?;

        let struct_handles = fs::read_dir(&dir)?
            .map(|entry| {
                let path = entry?.path();
                if path.extension().and_then(|x| x.to_str()) == Some("hhdg_delta") {
                    Ok(Some(path))
                } else {
                    Ok(None)
                }
            })
            .filter_map(|x| x.transpose())
            .collect::<io::Result<Vec<PathBuf>>>()?;

        Ok(EdgesDir {
            list_handles,
            struct_handles,
        })
    }

    fn list_handle_count(&self) -> usize {
        self.list_handles.len()
    }

    fn struct_handle_count(&self) -> usize {
        self.struct_handles.len()
    }

    fn read_all_edges(&mut self) -> io::Result<Edges> {
        let acc = Edges::default();
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

    fn read_all_struct_edges_for(source: &Path, acc: &Edges) -> io::Result<()> {
        let f = std::fs::OpenOptions::new().read(true).open(source).unwrap();

        let mut r = std::io::BufReader::new(f);

        let mut delta = DepGraphDelta::default();
        delta.read_from(&mut r, |_, _| true).unwrap();

        for (dependent, dependency) in delta.iter() {
            // To be kept in sync with Typing_deps.ml::filter_discovered_deps_batch
            let dependent: u64 = dependent.into();
            let dependency: u64 = dependency.into();
            acc.register(dependent, dependency);
        }

        Ok(())
    }
}

/// Structure used to read in all edges in parallel.
#[derive(Default)]
struct Edges {
    /// Map<dependency, Set<dependent>>
    edges: DashMap<u64, HashSet<u64>>,
}

impl Edges {
    fn register(&self, dependent: u64, dependency: u64) {
        self.edges.entry(dependency).or_default().insert(dependent);
    }

    fn finish(self) -> (Vec<(u64, Vec<u64>)>, Vec<u64>) {
        // All hashes in use as dependents or dependencies.
        //
        // It's easy to collect this without a DashSet, using fold() and reduce() below, but it
        // would create a very large HashSet in use in each thread at the same time, before they
        // get merged, and consume way too much memory.
        let hashes = DashSet::with_capacity_and_hasher(self.edges.len(), Default::default());

        // Collect up all hashes and sorted edge lists.
        let edges = (self.edges.into_par_iter())
            .map(|(dependency, dependents)| {
                let mut dependents: Vec<_> = dependents.into_iter().collect();
                dependents.sort_unstable();

                hashes.insert(dependency);
                for &d in dependents.iter() {
                    hashes.insert(d);
                }

                (dependency, dependents)
            })
            .collect();

        let mut hashes: Vec<u64> = hashes.into_iter().collect();
        hashes.par_sort_unstable();
        (edges, hashes)
    }

    fn count_edges(&self) -> usize {
        self.edges.iter().map(|e| e.value().len()).sum()
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
    const NUM_BUCKETS: usize = 2048;

    fn new() -> Self {
        Self {
            buckets: std::iter::repeat_with(Default::default)
                .take(Self::NUM_BUCKETS)
                .collect(),
        }
    }

    fn allocate_hash_list(&self, dependency: u64, sorted_dependents: &'a [u64]) {
        let mut hasher = hash::BuildHasher::default().build_hasher();
        sorted_dependents.hash(&mut hasher);
        let bucket_index = hasher.finish() as usize % Self::NUM_BUCKETS;
        let mut bucket = self.buckets[bucket_index].lock();
        let (_, dependencies) = bucket
            .entry(sorted_dependents)
            .or_insert_with(|| (HashListIndex(0), Vec::new()));
        dependencies.push(dependency);
    }

    /// Assign deterministic offsets to every key in HashListIndicesBucket::hash_list_indices.
    /// The contents of each bucket are deterministic because keys were assigned to buckets
    /// using a deterministic hash. But the order of keys in each bucket is nondeterministic because
    /// they were inserted in parallel.
    ///
    /// for each bucket in parallel:
    ///   * sort the &[u64] keys (sorted_dependents) of each bucket lexicographically.
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
    fn make_absolute(&mut self, mut cur_offset: u32) -> (Vec<u32>, u32) {
        let bucket_sizes: Vec<u32> = self
            .buckets
            .par_iter_mut()
            .with_min_len(1)
            .with_max_len(1)
            .map(|m| {
                let bucket = m.get_mut();
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
    graph.par_all_hashes().for_each(|dependency| {
        if let Some(hash_list) = graph.hash_list_for(dependency) {
            for dependent in graph.hash_list_hashes(hash_list) {
                all_edges.register(dependent.into(), dependency.into());
            }
        }
    });
}

/// Reads all the edges from the edges files in `edges_dir` and collects them in
/// a list of (dependency, list of dependents).
pub fn edges_from_dir(edges_dir: &Path) -> io::Result<Vec<(u64, Vec<u64>)>> {
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
            let mut delta = DepGraphDelta::default();
            let num_read = delta.read_from(&mut r, |_, _| true)?;
            info!("Delta loaded with {} edges", num_read);

            let all_edges = Edges::default();
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

    info!("Converting to structured_edges & unique hashes");
    let (structured_edges, sorted_hashes) = all_edges.finish();

    if !allow_empty && structured_edges.is_empty() {
        panic!("No input edges. Refusing to build as --allow-empty not set.");
    }

    info!("Registering {} unique hashes", sorted_hashes.len());
    let w = DepGraphWriter::new(sorted_hashes)?;

    info!("Calculating relative hash list offsets");
    let mut hash_list_indices = HashListsIndices::new();
    (structured_edges.par_iter()).for_each(|(dependency, sorted_dependents)| {
        hash_list_indices.allocate_hash_list(*dependency, sorted_dependents);
    });

    info!("Calculating absolute hash list offsets");
    let (bucket_offsets, next_set_offset) =
        hash_list_indices.make_absolute(w.first_hash_list_offset());

    info!("Cloning out indexer");
    let mut w = w.open_writer(&f, HashListIndex(next_set_offset))?;
    let indexer = w.clone_indexer();

    info!("Writing hash lists to database and constructing lookup table");
    let lookup_table = LookupTableWriter::default();
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
