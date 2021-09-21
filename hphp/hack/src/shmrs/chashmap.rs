// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::{BuildHasher, Hash, Hasher};
use std::mem::MaybeUninit;

use hashbrown::hash_map::DefaultHashBuilder;

use crate::filealloc::FileAlloc;
use crate::hashmap::Map;
use crate::mapalloc::{MapAlloc, MapAllocControlData};
use crate::sync::{RwLock, RwLockRef};

/// The number of shards.
///
/// DashMap uses (nproc * 4) rounded up to the next power of two. Let's do the
/// same under the assumption of 40 processors.
///
/// Must be a power of 2, as we use `trailing_zeros` for bitshifting.
const NUM_SHARDS: usize = 256;
static_assertions::const_assert!(NUM_SHARDS.is_power_of_two());

/// A concurrent hash map implemented as multiple sharded non-concurrent
/// hash maps.
///
/// This is the struct as laid out in memory. As such, it should live
/// in shared memory.
///
/// Use `initialize` or `attach` to get an interface into the map.
pub struct CMap<'shm, K, V, S = DefaultHashBuilder> {
    hash_builder: S,
    file_alloc: FileAlloc,
    map_allocs: [MapAllocControlData; NUM_SHARDS],
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
    file_alloc: &'shm FileAlloc,
    maps: Vec<RwLockRef<'shm, Map<'shm, K, V, S>>>,
}

impl<'shm, K, V> CMap<'shm, K, V, DefaultHashBuilder> {
    /// Initialize a new concurrent hash map at the given location.
    ///
    /// See `initialize_with_hasher`
    pub unsafe fn initialize(
        file_start: *mut libc::c_void,
        file_size: usize,
    ) -> CMapRef<'shm, K, V, DefaultHashBuilder> {
        Self::initialize_with_hasher(DefaultHashBuilder::new(), file_start, file_size)
    }
}

impl<'shm, K, V, S: Clone> CMap<'shm, K, V, S> {
    /// Initialize a new concurrent hash map at the given location.
    ///
    /// Safety:
    ///  - You must only initialize once and exactly once.
    ///  - Use `attach` to attach other processes to this memory location.
    ///  - The hash builder must not contain pointers to process-local memory.
    ///  - Obviously, `file_start` and `file_size` shouldn't lie.
    ///  - Make sure the lifetime returned matches the lifetime of the shared
    ///    memory pointer.
    ///  - Don't mutate or read the shared memory segment outside this API!
    ///
    /// Panics:
    ///  - If `file_size` is not large enough.
    pub unsafe fn initialize_with_hasher(
        hash_builder: S,
        file_start: *mut libc::c_void,
        file_size: usize,
    ) -> CMapRef<'shm, K, V, S> {
        let (self_ptr, next_free_byte) =
            Self::maybe_uninit_ptr_and_initial_next_free_byte(file_start, file_size);

        // Safety: Calling this function assumes:
        //  - The lifetime matches.
        //  - We are the sole users of the underlying memory.
        let cmap: &'shm mut MaybeUninit<Self> = &mut *self_ptr;

        // Initialize the memory properly.
        //
        // See MaybeUninit docs for examples.
        let mut map_allocs: [MaybeUninit<MapAllocControlData>; NUM_SHARDS] =
            MaybeUninit::uninit().assume_init();
        for map_alloc in &mut map_allocs[..] {
            *map_alloc = MaybeUninit::new(MapAllocControlData::new());
        }

        let mut maps: [MaybeUninit<RwLock<Map<'shm, K, V, S>>>; NUM_SHARDS] =
            MaybeUninit::uninit().assume_init();
        for map in &mut maps[..] {
            *map = MaybeUninit::new(RwLock::new(Map::new()));
        }

        cmap.as_mut_ptr().write(CMap {
            hash_builder,
            file_alloc: FileAlloc::new(file_start, file_size, next_free_byte),
            map_allocs: MaybeUninit::array_assume_init(map_allocs),
            maps: MaybeUninit::array_assume_init(maps),
        });
        let cmap = cmap.assume_init_mut();

        // Initialize map locks.
        let maps: Vec<RwLockRef<'shm, _>> = cmap
            .maps
            .iter_mut()
            .map(|r| r.initialize().unwrap())
            .collect();

        // Initialize maps themselves.
        assert_eq!(maps.len(), cmap.map_allocs.len());
        for (control_data, map) in cmap.map_allocs.iter().zip(maps.iter()) {
            let map_alloc = MapAlloc::new(control_data, &cmap.file_alloc);
            map.write()
                .unwrap()
                .reset_with_hasher(map_alloc, cmap.hash_builder.clone());
        }

        CMapRef {
            hash_builder: cmap.hash_builder.clone(),
            file_alloc: &cmap.file_alloc,
            maps,
        }
    }

    /// Attach to an already initialized concurrent hash map.
    ///
    /// Safety:
    ///  - The map at this pointer must already be initialized by a different
    ///    process (or by the calling process itself).
    pub unsafe fn attach(
        file_start: *mut libc::c_void,
        file_size: usize,
    ) -> CMapRef<'shm, K, V, S> {
        let (self_ptr, _) =
            Self::maybe_uninit_ptr_and_initial_next_free_byte(file_start, file_size);

        // Safety: already initialized!
        let cmap: &'shm mut Self = (&mut *self_ptr).assume_init_mut();

        // Attach to the map locks.
        let maps: Vec<RwLockRef<'shm, _>> = cmap.maps.iter_mut().map(|r| r.attach()).collect();

        CMapRef {
            hash_builder: cmap.hash_builder.clone(),
            file_alloc: &cmap.file_alloc,
            maps,
        }
    }

    unsafe fn maybe_uninit_ptr_and_initial_next_free_byte(
        file_start: *mut libc::c_void,
        file_size: usize,
    ) -> (*mut MaybeUninit<Self>, usize) {
        let layout = std::alloc::Layout::new::<Self>();
        let file_start = file_start as *mut u8;
        let align_offset = file_start.align_offset(layout.align());
        let total_size = align_offset + layout.size();
        assert!(file_size >= total_size);
        let ptr = file_start.add(align_offset);
        let ptr = ptr as *mut MaybeUninit<Self>;
        (ptr, total_size)
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

    /// Access the map that belongs to the given key for writing.
    ///
    /// Warning! May deadlock (and thus panic) when you already hold a writer lock on the
    /// queried map.
    ///
    /// Of course, if querying multiple maps at the same time, you should make
    /// sure your access pattern can't deadlock. In practice (at the moment)
    /// this means you can't try to hold multiple writer locks (or a write lock
    /// a read lock) at the same time, because the hasher is abstract. You have
    /// no way of knowing which map you need!
    pub fn write_map<R>(&self, key: &K, f: impl FnOnce(&mut Map<'shm, K, V, S>) -> R) -> R {
        let shard_index = self.shard_index_for(key);
        let mut map = self.maps[shard_index].write().unwrap();
        f(&mut map)
    }

    /// Return the total number of bytes allocated.
    ///
    /// Note that this might include bytes that were later free'd, as we
    /// (currently) don't free memory to the OS.
    pub fn allocated_bytes(&self) -> usize {
        self.file_alloc.allocated_bytes()
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
        let cmap = unsafe { CMap::initialize(mmap_ptr, MEM_HEAP_SIZE) };

        let mut child_procs = vec![];
        for scenario in &scenarios {
            match unsafe { nix::unistd::fork() }.unwrap() {
                ForkResult::Parent { child } => {
                    child_procs.push(child);
                }
                ForkResult::Child => {
                    // Exercise attach as well.
                    let cmap: CMapRef<'static, u64, u64> =
                        unsafe { CMap::attach(mmap_ptr, MEM_HEAP_SIZE) };

                    for &(key, value) in scenario.iter() {
                        cmap.write_map(&key, |map| {
                            map.insert(key, value);
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
