// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::Deref;
use std::ops::DerefMut;

use hashbrown::HashMap;

use crate::filealloc::FileAlloc;
use crate::hash_builder::ShmrsHashBuilder;

/// A hash map that lives in shared memory.
///
/// This is a wrapper around hashbrown's `HashMap`. The hash map lives
/// and allocates in shared memory.
pub struct Map<'shm, K, V, S = ShmrsHashBuilder>(Option<HashMap<K, V, S, &'shm FileAlloc>>);

impl<'shm, K, V, S> Deref for Map<'shm, K, V, S> {
    type Target = HashMap<K, V, S, &'shm FileAlloc>;

    fn deref(&self) -> &Self::Target {
        self.0.as_ref().unwrap()
    }
}

impl<'shm, K, V, S> DerefMut for Map<'shm, K, V, S> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.0.as_mut().unwrap()
    }
}

impl<'shm, K, V> Map<'shm, K, V, ShmrsHashBuilder> {
    /// Re-allocate the hash map.
    ///
    /// See `reset_with_hasher`
    pub fn reset(&mut self, alloc: &'shm FileAlloc) {
        self.0 = Some(HashMap::with_hasher_in(ShmrsHashBuilder::new(), alloc));
    }
}

impl<'shm, K, V, S> Map<'shm, K, V, S> {
    /// Return a map layout with an uninitialized map and allocator.
    ///
    /// Call `reset` to actually allocate the map.
    pub fn new() -> Self {
        Self(None)
    }

    /// Re-allocate the hash map.
    ///
    /// Uses the given file allocator to allocate the hashmap's table.
    pub fn reset_with_hasher(&mut self, alloc: &'shm FileAlloc, hash_builder: S) {
        // ??? apparently if we construct HashMap without capacity, then it won't
        // allocate any space for its table via alloc, but subsequent access
        // will assume that something was allocated. By giving it a capacity, we
        // avoid this.
        let map = HashMap::with_capacity_and_hasher_in(1, hash_builder, alloc);
        self.0 = Some(map);
    }
}

#[cfg(test)]
mod integration_tests {
    use std::collections::HashMap;
    use std::collections::HashSet;
    use std::mem::MaybeUninit;
    use std::time::Duration;

    use nix::sys::wait::WaitStatus;
    use nix::unistd::ForkResult;
    use rand::prelude::*;

    use super::*;
    use crate::filealloc::FileAlloc;
    use crate::sync::RwLock;

    struct InsertMany {
        file_alloc: FileAlloc,
        map: RwLock<Map<'static, u64, u64>>,
    }

    #[test]
    fn test_insert_many() {
        const NUM_PROCS: usize = 20;
        const NUM_INSERTS_PER_PROC: usize = 1000;
        const OP_SLEEP: Duration = Duration::from_micros(10);

        const MEM_HEAP_SIZE: usize = 10 * 1024 * 1024;

        let mut rng = StdRng::from_seed([0; 32]);
        let scenarios: Vec<Vec<(u64, u64)>> = std::iter::repeat_with(|| {
            std::iter::repeat_with(|| (rng.next_u64() % 1000, rng.next_u64() % 1000))
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
        let inserter_ptr: *mut MaybeUninit<InsertMany> = mmap_ptr as *mut _;
        let inserter: &'static mut MaybeUninit<InsertMany> =
            // Safety:
            //  - Pointer is not null
            //  - Pointer is aligned on a page
            //  - This is the only reference to the data, and the lifteim is
            //    static as we don't unmap the memory
            unsafe { &mut *inserter_ptr };
        // Safety: Initialize the memory properly
        let inserter = unsafe {
            inserter.as_mut_ptr().write(InsertMany {
                file_alloc: FileAlloc::new(
                    mmap_ptr,
                    MEM_HEAP_SIZE,
                    std::mem::size_of::<InsertMany>(),
                ),
                map: RwLock::new(Map::new()),
            });
            inserter.assume_init_mut()
        };
        // Safety: We are the only ones to attach to this lock.
        let map = unsafe { inserter.map.initialize() }.unwrap();

        map.write(None).unwrap().reset(&inserter.file_alloc);

        let mut child_procs = vec![];
        for scenario in &scenarios {
            match unsafe { nix::unistd::fork() }.unwrap() {
                ForkResult::Parent { child } => {
                    child_procs.push(child);
                }
                ForkResult::Child => {
                    for &(key, value) in scenario.iter() {
                        let mut guard = map.write(None).unwrap();
                        guard.insert(key, value);
                        std::thread::sleep(OP_SLEEP);
                        // Make sure we sleep while holding the lock.
                        drop(guard);
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

        let guard = map.read(None).unwrap();

        let mut expected: HashMap<u64, HashSet<u64>> = HashMap::new();
        for scenario in scenarios {
            for (key, value) in scenario {
                expected.entry(key).or_default().insert(value);
            }
        }

        for (key, values) in expected {
            let value = guard[&key];
            assert!(values.contains(&value));
        }

        // Must drop! Otherwise, munmap will already have unmapped the lock!
        drop(guard);
        assert_eq!(unsafe { libc::munmap(mmap_ptr, MEM_HEAP_SIZE) }, 0);
    }
}
