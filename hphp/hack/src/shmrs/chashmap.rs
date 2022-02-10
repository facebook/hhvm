// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::{BuildHasher, Hash, Hasher};
use std::mem::MaybeUninit;

use hashbrown::hash_map::DefaultHashBuilder;

use crate::filealloc::FileAlloc;
use crate::hashmap::Map;
use crate::shardalloc::{ShardAlloc, ShardAllocControlData};
use crate::sync::{RwLock, RwLockRef};

/// The number of shards.
///
/// DashMap uses (nproc * 4) rounded up to the next power of two. Let's do the
/// same under the assumption of 40 processors.
///
/// Must be a power of 2, as we use `trailing_zeros` for bitshifting.
pub const NUM_SHARDS: usize = 256;
static_assertions::const_assert!(NUM_SHARDS.is_power_of_two());

/// The non-evictable allocator itself allocates regions of memory in chunks.
const NON_EVICTABLE_CHUNK_SIZE: usize = 1024 * 1024;

/// This struct gives access to a shard, including its hashmap and its
/// allocators.
pub struct Shard<'shm, 'a, K, V, S> {
    pub map: &'a mut Map<'shm, K, V, S>,
    alloc_non_evictable: &'a ShardAlloc<'shm>,
    alloc_evictable: &'a ShardAlloc<'shm>,
}

impl<'shm, 'a, K, V, S> Shard<'shm, 'a, K, V, S> {
    /// Return an allocator.
    ///
    /// If the argument is true, return the allocator for evictable values.
    /// If the argument is false, return the allocator for non-evictable values.
    #[inline(always)]
    pub fn alloc(&self, evictable: bool) -> &'a ShardAlloc<'shm> {
        if evictable {
            self.alloc_evictable
        } else {
            self.alloc_non_evictable
        }
    }
}

/// A concurrent hash map implemented as multiple sharded non-concurrent
/// hash maps.
///
/// This is the struct as laid out in memory. As such, it should live
/// in shared memory.
///
/// Use `initialize` or `attach` to get an interface into the map.
///
/// ## Invariants
/// There are important invariants about this data structure. Failing to
/// uphold these invariants might crash the program.
///
///   1. Each sharded hashmap has two sharded allocators: one for
///      non-evictable items, and one for evictable items. Putting
///      non-evictable items in the evictable shard (and vice versa)
///      is an invariant violation.
///   2. Each sharded hashmap can only contain pointers to values in
///      its own sharded allocators. Pointing to a value in a different
///      shard allocator is an invariant violation.
pub struct CMap<'shm, K, V, S = DefaultHashBuilder> {
    hash_builder: S,
    max_evictable_bytes_per_shard: usize,
    file_alloc: &'shm FileAlloc,
    shard_allocs_non_evictable: [RwLock<ShardAllocControlData>; NUM_SHARDS],
    shard_allocs_evictable: [RwLock<ShardAllocControlData>; NUM_SHARDS],
    maps: [RwLock<Map<'shm, K, V, S>>; NUM_SHARDS],
}

/// A reference to a concurrent hash map.
///
/// This struct is merely a reference to the shared memory data. As such,
/// it is process-local.
///
/// Obtained by calling `initialize` or `attach` on `CMap`.
pub struct CMapRef<'shm, K, V, S = DefaultHashBuilder> {
    hash_builder: S,
    pub max_evictable_bytes_per_shard: usize,
    file_alloc: &'shm FileAlloc,
    shard_allocs_non_evictable: Vec<ShardAlloc<'shm>>,
    shard_allocs_evictable: Vec<ShardAlloc<'shm>>,
    maps: Vec<RwLockRef<'shm, Map<'shm, K, V, S>>>,
}

impl<'shm, K, V> CMap<'shm, K, V, DefaultHashBuilder> {
    /// Initialize a new concurrent hash map at the given location.
    ///
    /// See `initialize_with_hasher`
    pub unsafe fn initialize(
        cmap: &'shm mut MaybeUninit<Self>,
        file_alloc: &'shm FileAlloc,
        max_evictable_bytes_per_shard: usize,
    ) -> CMapRef<'shm, K, V, DefaultHashBuilder> {
        Self::initialize_with_hasher(
            cmap,
            DefaultHashBuilder::new(),
            file_alloc,
            max_evictable_bytes_per_shard,
        )
    }
}

impl<'shm, K, V, S: Clone> CMap<'shm, K, V, S> {
    /// Initialize a new concurrent hash map at the given location.
    ///
    /// Safety:
    ///  - You must only initialize once and exactly once.
    ///  - Use `attach` to attach other processes to this memory location.
    ///  - The hash builder must not contain pointers to process-local memory.
    ///  - Don't mutate or read the shared memory segment outside this API!
    ///
    /// Panics:
    ///  - If `file_size` is not large enough.
    pub unsafe fn initialize_with_hasher(
        cmap: &'shm mut MaybeUninit<Self>,
        hash_builder: S,
        file_alloc: &'shm FileAlloc,
        max_evictable_bytes_per_shard: usize,
    ) -> CMapRef<'shm, K, V, S> {
        // Initialize the memory properly.
        //
        // See MaybeUninit docs for examples.
        let mut shard_allocs_non_evictable: [MaybeUninit<RwLock<ShardAllocControlData>>;
            NUM_SHARDS] = MaybeUninit::uninit().assume_init();
        for shard_alloc in &mut shard_allocs_non_evictable[..] {
            *shard_alloc = MaybeUninit::new(RwLock::new(ShardAllocControlData::new()));
        }
        let mut shard_allocs_evictable: [MaybeUninit<RwLock<ShardAllocControlData>>; NUM_SHARDS] =
            MaybeUninit::uninit().assume_init();
        for shard_alloc in &mut shard_allocs_evictable[..] {
            *shard_alloc = MaybeUninit::new(RwLock::new(ShardAllocControlData::new()));
        }

        let mut maps: [MaybeUninit<RwLock<Map<'shm, K, V, S>>>; NUM_SHARDS] =
            MaybeUninit::uninit().assume_init();
        for map in &mut maps[..] {
            *map = MaybeUninit::new(RwLock::new(Map::new()));
        }

        cmap.as_mut_ptr().write(CMap {
            hash_builder,
            max_evictable_bytes_per_shard,
            file_alloc,
            shard_allocs_non_evictable: MaybeUninit::array_assume_init(shard_allocs_non_evictable),
            shard_allocs_evictable: MaybeUninit::array_assume_init(shard_allocs_evictable),
            maps: MaybeUninit::array_assume_init(maps),
        });
        let cmap = cmap.assume_init_mut();

        // Initialize map locks.
        let maps: Vec<RwLockRef<'shm, _>> = cmap
            .maps
            .iter_mut()
            .map(|r| r.initialize().unwrap())
            .collect();

        // Initialize shard allocator locks.
        let mut shard_allocs_non_evictable: Vec<ShardAlloc<'shm>> =
            Vec::with_capacity(cmap.shard_allocs_non_evictable.len());
        for lock in &mut cmap.shard_allocs_non_evictable {
            shard_allocs_non_evictable.push(ShardAlloc::new(
                lock.initialize().unwrap(),
                &cmap.file_alloc,
                NON_EVICTABLE_CHUNK_SIZE,
                false,
            ));
        }
        let mut shard_allocs_evictable: Vec<ShardAlloc<'shm>> =
            Vec::with_capacity(cmap.shard_allocs_evictable.len());
        for lock in &mut cmap.shard_allocs_evictable {
            shard_allocs_evictable.push(ShardAlloc::new(
                lock.initialize().unwrap(),
                &cmap.file_alloc,
                max_evictable_bytes_per_shard,
                true,
            ));
        }

        // Initialize maps themselves.
        for map in maps.iter() {
            map.write()
                .unwrap()
                .reset_with_hasher(&cmap.file_alloc, cmap.hash_builder.clone());
        }

        CMapRef {
            hash_builder: cmap.hash_builder.clone(),
            max_evictable_bytes_per_shard: cmap.max_evictable_bytes_per_shard,
            file_alloc: cmap.file_alloc,
            shard_allocs_non_evictable,
            shard_allocs_evictable,
            maps,
        }
    }

    /// Attach to an already initialized concurrent hash map.
    ///
    /// Safety:
    ///  - The map at this pointer must already be initialized by a different
    ///    process (or by the calling process itself).
    pub unsafe fn attach(cmap: &'shm MaybeUninit<Self>) -> CMapRef<'shm, K, V, S> {
        // Safety: already initialized!
        let cmap = cmap.assume_init_ref();

        // Attach to the map locks.
        let maps: Vec<RwLockRef<'shm, _>> = cmap.maps.iter().map(|r| r.attach()).collect();

        // Attach shard allocators.
        let mut shard_allocs_non_evictable: Vec<ShardAlloc<'shm>> =
            Vec::with_capacity(cmap.shard_allocs_non_evictable.len());
        for lock in &cmap.shard_allocs_non_evictable {
            shard_allocs_non_evictable.push(ShardAlloc::new(
                lock.attach(),
                &cmap.file_alloc,
                NON_EVICTABLE_CHUNK_SIZE,
                false,
            ));
        }
        let mut shard_allocs_evictable: Vec<ShardAlloc<'shm>> =
            Vec::with_capacity(cmap.shard_allocs_evictable.len());
        for lock in &cmap.shard_allocs_evictable {
            shard_allocs_evictable.push(ShardAlloc::new(
                lock.attach(),
                &cmap.file_alloc,
                cmap.max_evictable_bytes_per_shard,
                true,
            ));
        }

        CMapRef {
            hash_builder: cmap.hash_builder.clone(),
            max_evictable_bytes_per_shard: cmap.max_evictable_bytes_per_shard,
            file_alloc: &cmap.file_alloc,
            shard_allocs_non_evictable,
            shard_allocs_evictable,
            maps,
        }
    }
}

impl<'shm, K: Hash, V, S: BuildHasher> CMapRef<'shm, K, V, S> {
    fn shard_index_for(&self, key: &K) -> usize {
        let mut hasher = self.hash_builder.build_hasher();
        key.hash(&mut hasher);
        let hash = hasher.finish();

        // The higher bits are also used by hashbrown's HashMap.
        // This is a cheap mixer to get some entropy for shard selection.
        let hash: u64 = hash.wrapping_mul(0x9e3779b97f4a7c15);

        (hash >> (64 - NUM_SHARDS.trailing_zeros())) as usize
    }

    /// Access the map that belongs to the given key for reading.
    pub fn read_map<R>(&self, key: &K, f: impl FnOnce(&Map<'shm, K, V, S>) -> R) -> R {
        let shard_index = self.shard_index_for(key);
        let map = self.maps[shard_index].read().unwrap();
        f(&map)
    }

    /// Access the map and that belongs to the given key for writing. Also provides
    /// access to the corresponding shard allocator.
    ///
    /// Warning! May deadlock (and thus panic) when you already hold a writer lock on the
    /// queried map.
    ///
    /// Of course, if querying multiple maps at the same time, you should make
    /// sure your access pattern can't deadlock. In practice (at the moment)
    /// this means you can't try to hold multiple writer locks (or a write lock
    /// a read lock) at the same time, because the hasher is abstract. You have
    /// no way of knowing which map you need!
    pub fn write_map<R>(&self, key: &K, f: impl FnOnce(Shard<'shm, '_, K, V, S>) -> R) -> R {
        let shard_index = self.shard_index_for(key);
        let mut map = self.maps[shard_index].write().unwrap();
        let alloc_non_evictable = &self.shard_allocs_non_evictable[shard_index];
        let alloc_evictable = &self.shard_allocs_evictable[shard_index];
        f(Shard {
            map: &mut map,
            alloc_non_evictable,
            alloc_evictable,
        })
    }

    /// Return the total number of bytes allocated.
    ///
    /// Note that this might include bytes that were later free'd, as we
    /// (currently) don't free memory to the OS.
    pub fn allocated_bytes(&self) -> usize {
        self.file_alloc.allocated_bytes()
    }

    /// Return the number of total entries in the hash map.
    ///
    /// Will loop over each shard.
    pub fn len(&self) -> usize {
        self.maps.iter().map(|map| map.read().unwrap().len()).sum()
    }

    /// Return true if the hashmap is empty.
    /// Will loop over each shard.
    pub fn is_empty(&self) -> bool {
        self.maps.iter().all(|map| map.read().unwrap().is_empty())
    }
}

#[cfg(test)]
mod integration_tests {
    use super::*;

    use std::collections::{HashMap, HashSet};
    use std::time::Duration;

    use nix::sys::wait::WaitStatus;
    use nix::unistd::ForkResult;
    use rand::prelude::*;

    #[test]
    fn test_insert_many() {
        const NUM_PROCS: usize = 20;
        const NUM_INSERTS_PER_PROC: usize = 1000;
        const OP_SLEEP: Duration = Duration::from_micros(10);

        const MEM_HEAP_SIZE: usize = 100 * 1024 * 1024;

        struct Segment<'shm> {
            file_alloc: MaybeUninit<FileAlloc>,
            table: MaybeUninit<CMap<'shm, u64, u64>>,
        }

        let mut rng = StdRng::from_seed([0; 32]);
        let scenarios: Vec<Vec<(u64, u64)>> = std::iter::repeat_with(|| {
            std::iter::repeat_with(|| (rng.next_u64() % 10_000, rng.next_u64() % 10_000))
                .take(NUM_INSERTS_PER_PROC)
                .collect()
        })
        .take(NUM_PROCS)
        .collect();

        let mmap_ptr = unsafe {
            libc::mmap(
                std::ptr::null_mut(),
                MEM_HEAP_SIZE,
                libc::PROT_READ | libc::PROT_WRITE,
                libc::MAP_SHARED | libc::MAP_ANONYMOUS,
                -1,
                0,
            )
        };
        assert_ne!(mmap_ptr, libc::MAP_FAILED);

        let layout = std::alloc::Layout::new::<Segment<'_>>();
        assert_eq!(mmap_ptr.align_offset(layout.align()), 0);
        let segment = mmap_ptr as *mut MaybeUninit<Segment<'static>>;
        let cmap = unsafe {
            let segment = &mut *segment;
            segment.write(Segment {
                file_alloc: MaybeUninit::uninit(),
                table: MaybeUninit::uninit(),
            });
            let segment = segment.assume_init_mut();
            segment
                .file_alloc
                .write(FileAlloc::new(mmap_ptr, MEM_HEAP_SIZE, layout.size()));
            let file_alloc = segment.file_alloc.assume_init_mut();
            CMap::initialize(&mut segment.table, file_alloc, 128)
        };

        let mut child_procs = vec![];
        for scenario in &scenarios {
            match unsafe { nix::unistd::fork() }.unwrap() {
                ForkResult::Parent { child } => {
                    child_procs.push(child);
                }
                ForkResult::Child => {
                    // Exercise attach as well.

                    let cmap: CMapRef<'static, u64, u64> = unsafe {
                        let segment = mmap_ptr as *const MaybeUninit<Segment<'static>>;
                        let segment = &*segment;
                        let segment = segment.assume_init_ref();
                        CMap::attach(&segment.table)
                    };

                    for &(key, value) in scenario.iter() {
                        cmap.write_map(&key, |shard| {
                            shard.map.insert(key, value);
                            std::thread::sleep(OP_SLEEP);
                        });
                    }

                    std::process::exit(0)
                }
            }
        }

        for pid in child_procs {
            match nix::sys::wait::waitpid(pid, None).unwrap() {
                WaitStatus::Exited(_, status) => assert_eq!(status, 0),
                status => panic!("unexpected status for pid {:?}: {:?}", pid, status),
            }
        }

        let mut expected: HashMap<u64, HashSet<u64>> = HashMap::new();
        for scenario in scenarios {
            for (key, value) in scenario {
                expected.entry(key).or_default().insert(value);
            }
        }

        for (key, values) in expected {
            cmap.read_map(&key, |map| {
                let value = map[&key];
                assert!(values.contains(&value));
            });
        }

        assert_eq!(unsafe { libc::munmap(mmap_ptr, MEM_HEAP_SIZE) }, 0);
    }
}
