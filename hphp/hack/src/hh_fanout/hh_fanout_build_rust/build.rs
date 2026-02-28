// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ffi::OsString;
use std::fs;
use std::fs::File;
use std::io;
use std::path::Path;
use std::path::PathBuf;
use std::sync::atomic::AtomicUsize;
use std::sync::atomic::Ordering;

use dep_graph_delta::DepGraphDeltaIterator;
use depgraph_compress::OptimizeConfig;
use depgraph_compress::WriteConfig;
use depgraph_reader::Dep;
use depgraph_reader::DepGraph;
use depgraph_writer::HashIndex;
use depgraph_writer::HashIndexSet;
use depgraph_writer::HashListIndex;
use depgraph_writer::MemDepGraph;
use hash::DashMap;
use log::info;
use newtype::IdVec;
use parking_lot::Mutex;
use rayon::prelude::*;
use smallvec::SmallVec;

struct EdgesDir {
    // For remote typechecks, we output edges in a serialized rust structure format
    struct_handles: Vec<PathBuf>,
}

impl EdgesDir {
    fn open<P: AsRef<Path>>(dir: P) -> io::Result<EdgesDir> {
        let struct_handles = fs::read_dir(&dir)?
            .map(|entry| {
                let path = entry?.path();
                if matches!(
                    path.extension().and_then(|x| x.to_str()),
                    Some("bin") | Some("hhdg_delta")
                ) {
                    Ok(Some(path))
                } else {
                    Ok(None)
                }
            })
            .filter_map(|x| x.transpose())
            .collect::<io::Result<Vec<PathBuf>>>()?;

        Ok(EdgesDir { struct_handles })
    }

    fn struct_handle_count(&self) -> usize {
        self.struct_handles.len()
    }

    fn read_all_edges(self) -> io::Result<Edges> {
        let mut acc = Edges::default();

        register_dep_graph_delta_files(&self.struct_handles, &mut acc)?;

        Ok(acc)
    }
}

/// Read in DepGraphDelta files and add their contents to `edges`.
fn register_dep_graph_delta_files(all_paths: &[PathBuf], edges: &mut Edges) -> io::Result<()> {
    let num_files_read = AtomicUsize::new(0);

    all_paths
        .par_iter()
        .with_min_len(1)
        .with_max_len(1)
        .try_for_each(|path| -> io::Result<()> {
            // Log progress in a human-readably way no matter how rayon parallelizes things.
            // It's not required that these messages be strictly in order (no need to lock here just for this).
            let i = num_files_read.fetch_add(1, Ordering::Relaxed);
            if all_paths.len() < 100 || (i + 1) % 100 == 0 {
                info!("Reading in struct file {}/{}", i + 1, all_paths.len());
            }

            if std::fs::metadata(path)?.len() == 0 {
                // We can't mmap empty files, but they have no data anyway so ignore.
                return Ok(());
            }

            // Memory-map the .hhdg_delta file and tell Linux we're going to need its bytes.
            let mmap = {
                let file = File::open(path)?;
                let mmap = unsafe { memmap2::Mmap::map(&file)? };

                // Prefetch the file. We don't care if this fails, it's just an optimization.
                let _ = mmap.advise(memmap2::Advice::WillNeed);

                mmap
            };

            // Turn mmap raw bytes into &[u64], which we know it contains.
            let contents: &[u64] = bytemuck::cast_slice(&mmap as &[u8]);

            // Dump edges into the graph.
            for (dependency, dependents) in DepGraphDeltaIterator::new(contents) {
                edges.register_many(dependency, dependents.iter().copied());
            }

            Ok(())
        })
}

/// We have high DashMap lock contention with the default number of shards, so increase it by 4x.
fn high_dashmap_shard_count() -> usize {
    std::cmp::max(1, rayon::current_num_threads() * 16).next_power_of_two()
}

/// Maps a Dep to a densely numbered 32-bit newtype.
///
/// This mapping is not deterministic, it's designed to hand out numbers
/// quickly. Later, when we have seen all Deps, we renumber uses to
/// use a final deterministic numbering based on Dep order.
#[derive(Debug)]
struct DepToHashIndex {
    /// Recently added mappings we haven't flushed to deps.
    deps: DashMap<Dep, HashIndex>,

    /// The next index to hand out when something is added to `deps`.
    next_index: AtomicUsize,
}

impl DepToHashIndex {
    fn new() -> Self {
        let deps = dashmap::DashMap::with_hasher_and_shard_amount(
            hash::BuildHasher::default(),
            high_dashmap_shard_count(),
        );

        Self {
            deps,
            next_index: AtomicUsize::new(0),
        }
    }

    fn get_or_allocate(&self, dep: Dep) -> HashIndex {
        *self.deps.entry(dep).or_insert_with(|| {
            HashIndex::from_usize(self.next_index.fetch_add(1, Ordering::Relaxed))
        })
    }

    fn finish(self) -> DashMap<Dep, HashIndex> {
        // Check for overflow here.
        assert!(self.next_index.into_inner() <= 1 + !0u32 as usize);
        self.deps
    }
}

/// Structure used to read in all edges in parallel
#[derive(Debug)]
pub struct Edges {
    /// This table maps a HashIndex for dependencies to a HashSet of dependents.
    ///
    /// For scalability it's sharded. The low bits of the HashIndex pick which shard
    /// to use, and remaining bits index into that shard's Vec.
    shards: Box<[Mutex<Vec<HashIndexSet>>; Self::NUM_SHARDS]>,

    /// Assigns new, temporary Dep -> HashIndex mappings.
    dep_to_temp_index: DepToHashIndex,
}

impl Default for Edges {
    fn default() -> Self {
        Self {
            shards: Box::new(std::array::from_fn(|_| Default::default())),
            dep_to_temp_index: DepToHashIndex::new(),
        }
    }
}

impl Edges {
    const NUM_SHARDS: usize = 2048;

    /// Register many dependents for one dependency in a single shot.
    pub fn register_many<T>(&self, dependency: Dep, dependents: T)
    where
        T: Iterator<Item = Dep>,
    {
        let dependency_index = self.dep_to_temp_index.get_or_allocate(dependency);
        let dependency_num = dependency_index.as_usize();

        let dependents: Vec<HashIndex> = dependents
            .map(|dep| self.dep_to_temp_index.get_or_allocate(dep))
            .collect();

        let shard = &mut self.shards[dependency_num % Self::NUM_SHARDS].lock();
        let index = dependency_num / Self::NUM_SHARDS;

        if shard.len() <= index {
            shard.resize_with(index + 1, Default::default);
        }

        shard[index].extend(dependents);
    }

    pub fn finish(self) -> MemDepGraph {
        // Sort the hashes.
        info!("Sorting all hashes");
        let mut dep_to_temp_index: Vec<_> = self.dep_to_temp_index.finish().into_iter().collect();
        dep_to_temp_index.par_sort_unstable_by_key(|(dep, _)| *dep);
        info!("Sorting all hashes done");

        // Create the vec of hashes & inverse mapping so we can update all HashIndex values.
        info!("Creating remap table");
        let mut remap_table: Vec<HashIndex> = vec![HashIndex(0); dep_to_temp_index.len()];
        let hashes: IdVec<HashIndex, Dep> = dep_to_temp_index
            .into_iter()
            .enumerate()
            .map(|(i, (dep, old_index))| {
                // While we're here, update `remap_table`.
                //
                // This can be done in parallel in remap_table holds atomics.
                remap_table[old_index.as_usize()] = HashIndex::from_usize(i);

                dep
            })
            .collect();
        info!("Creating remap table done");

        info!("Interning edge lists");

        // Collect up all sorted edge lists. They had 32 bit HashIndex values, but they were
        // using the old, unsorted numbering scheme so we need to update them as we go.
        let edge_list_interner: DashMap<Box<[HashIndex]>, SmallVec<[HashIndex; 4]>> =
            dashmap::DashMap::with_hasher_and_shard_amount(
                hash::BuildHasher::default(),
                high_dashmap_shard_count(),
            );

        // Guarantee there exists an entry empty edge list, as it's the default we use later.
        edge_list_interner.insert(Default::default(), Default::default());

        self.shards
            .into_par_iter()
            .enumerate()
            .for_each(|(shard_lo_index, shard)| {
                shard
                    .into_inner()
                    .into_par_iter()
                    .enumerate()
                    .for_each(|(i, dependents_set)| {
                        let shard_hi_index = i * Self::NUM_SHARDS;
                        let dependency = HashIndex::from_usize(shard_hi_index + shard_lo_index);

                        // Sort both to canonicalize, and so interning works.
                        let mut dependents: Box<[HashIndex]> = dependents_set
                            .into_iter()
                            .map(|d| remap_table[d.as_usize()])
                            .collect();
                        dependents.sort_unstable();

                        edge_list_interner
                            .entry(dependents)
                            .or_default()
                            .push(remap_table[dependency.as_usize()]);
                    })
            });
        drop(remap_table);

        // Sort into edge lists into canonical order, placing the empty list first.
        let mut interned_edge_lists: Vec<_> = edge_list_interner.into_iter().collect();
        interned_edge_lists.par_sort_unstable_by(|(d1, _), (d2, _)| {
            let key1 = (d1.len(), d1);
            let key2 = (d2.len(), d2);
            key1.cmp(&key2)
        });
        let empty_edge_list_index = HashListIndex(0);
        assert!(
            interned_edge_lists[empty_edge_list_index.0 as usize]
                .0
                .is_empty()
        );

        info!("Interning edge lists done");

        info!("Building edge lists");

        // Here we give each Dep an index into a vec of shared edge lists. It's common for multiple
        // Deps to share the same edge list.

        // Everything defaults to having an empty edge list unless we see otherwise.
        let mut edge_list_indices = IdVec::new_from_vec(vec![empty_edge_list_index; hashes.len()]);

        // Build up a Vec of the edge lists, and remember which edge list each HashIndex uses.
        let edge_lists = interned_edge_lists
            .into_iter()
            .enumerate()
            .map(|(i, (dependents, dependencies))| {
                // Allocate an edge list index for this edge list.
                let hash_list_index = HashListIndex::from_usize(i);

                // Tell all of the dependencies with this edge list that they are using it.
                for dependency in dependencies {
                    edge_list_indices[dependency] = hash_list_index;
                }

                // Remember the edge list.
                dependents
            })
            .collect();

        info!("Building edge lists done");

        MemDepGraph {
            hashes,
            edge_list_indices,
            edge_lists,
        }
    }

    fn count_edges(&self) -> usize {
        self.shards
            .par_iter()
            .with_min_len(1)
            .with_max_len(1)
            .map(|e| e.lock().iter().map(|s| s.len()).sum::<usize>())
            .sum()
    }
}

/// Extend a collection of edges by adding all edges from the given
/// dependency graph.
fn extend_edges_from_dep_graph(all_edges: &Edges, graph: &DepGraph) {
    graph.par_all_hashes().for_each(|dependency| {
        if let Some(hash_list) = graph.hash_list_for(dependency) {
            all_edges.register_many(dependency, graph.hash_list_hashes(hash_list));
        }
    });
}

/// Reads all the edges from the edges files in `edges_dir` and collects them in
/// a list of (dependency, list of dependents).
pub fn edges_from_dir(edges_dir: &Path) -> io::Result<MemDepGraph> {
    let edges_dir = EdgesDir::open(edges_dir)?;
    let edges = edges_dir.read_all_edges()?;
    Ok(edges.finish())
}

pub fn build(
    allow_empty: bool,
    incremental: Option<OsString>,
    new_edges_dir: Option<OsString>,
    delta_file: Option<OsString>,
    output: &Path,
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
            let new_edges_dir = EdgesDir::open(new_edges_dir)?;

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
            let mut all_edges = Edges::default();
            register_dep_graph_delta_files(&[delta_file.into()], &mut all_edges)?;
            info!("Delta loaded with {} edges", all_edges.count_edges());
            all_edges
        }
    };

    match incremental {
        None => info!("Not reading in edges from previous dependency graph (incremental=None)"),
        Some(incremental) => {
            info!(
                "Reading in edges from previous dependency graph at {:?}",
                incremental
            );
            let old_dep_graph = DepGraph::from_path(&incremental)?;
            extend_edges_from_dep_graph(&all_edges, &old_dep_graph);
            info!("Done reading in old edges");
        }
    }

    info!("Done reading Edges");

    info!("Converting to structured_edges & unique hashes");
    let mem_dep_graph = all_edges.finish();
    info!("Converting to structured_edges & unique hashes done");

    if !allow_empty && mem_dep_graph.edge_lists.iter().all(|list| list.is_empty()) {
        panic!("No input edges. Refusing to build as --allow-empty not set.");
    }

    info!("Registering {} unique hashes", mem_dep_graph.hashes.len());

    if output.extension().and_then(|x| x.to_str()) == Some("zhhdg") {
        let write_config = WriteConfig::default();
        let optimize_config = OptimizeConfig::default();
        depgraph_compress::write_dep_graph(output, mem_dep_graph, &write_config, &optimize_config)?;
    } else {
        depgraph_writer::write_dep_graph(output, &mem_dep_graph)?;
    }

    info!("Done");
    Ok(())
}
