// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::BuildHasherDefault;
use std::mem::MaybeUninit;

use nohash_hasher::NoHashHasher;

use crate::chashmap::CMap;
use crate::chashmap::CMapRef;
use crate::filealloc::FileAlloc;

/// The u64s are the first 8 bytes of a shasum. As such, they should have
/// enough entropy to be used directly as keys into the hashmap.
///
/// Note that the underlying hash table implementation (hashbrown) requires
/// entropy in both the upper and the lower bits of this u64. If this
/// assumption ever changes, performance will suffer.
type HashBuilder = BuildHasherDefault<NoHashHasher<u64>>;

/// Shared memory segment that is used to store the sharded hashtable.
///
/// The shared memory pointer that we obtain from `mmap` is directly cast
/// to this struct. Its fields are all `MaybeUninit` because we can't
/// initialize them all at once.
pub struct ShmemTableSegment<'shm, T> {
    file_alloc: MaybeUninit<FileAlloc>,
    table: MaybeUninit<CMap<'shm, u64, T, HashBuilder>>,
}

/// A reference to an attached shared memory segment.
///
/// This struct is merely a reference to the shared memory segment and
/// the data it contains. As such, it is process-local.
///
/// Obtained by calling `initialize` or `attach` on `ShmemTableSegment`.
pub struct ShmemTableSegmentRef<'shm, T> {
    pub table: CMapRef<'shm, u64, T, HashBuilder>,
}

impl<'shm, T> ShmemTableSegment<'shm, T> {
    /// Initialize a shared memory segment, by setting up the file allocator
    /// and the hash tables.
    ///
    /// Safety:
    ///  - You must only intialize once and exactly once.
    ///  - Use `attach` to attach other processes to this memory location.
    ///  - Obviously, `file_start` and `file_size` shouldn't lie.
    ///  - Make sure the lifetime returned matches the lifetime of the shared
    ///    memory pointer.
    ///  - Don't mutate or read the shared memory segment outside this API!
    ///
    /// Panics:
    ///  - If `files_size` is not large enough.
    pub unsafe fn initialize(
        file_start: *mut libc::c_void,
        file_size: usize,
        max_evictable_bytes_per_shard: usize,
    ) -> ShmemTableSegmentRef<'shm, T> {
        let (self_ptr, next_free_byte) =
            Self::maybe_unint_ptr_and_next_free_byte(file_start, file_size);

        // Safety: Doing this cast assumes:
        //  - The lifetime matches.
        //  - We are the sole users of the underlying memory.
        let segment: &'shm mut MaybeUninit<Self> = &mut *self_ptr;
        segment.write(ShmemTableSegment {
            file_alloc: MaybeUninit::uninit(),
            table: MaybeUninit::uninit(),
        });
        let segment = segment.assume_init_mut();

        segment
            .file_alloc
            .write(FileAlloc::new(file_start, file_size, next_free_byte));
        let file_alloc = segment.file_alloc.assume_init_mut();
        let table = CMap::initialize_with_hasher(
            &mut segment.table,
            BuildHasherDefault::default(),
            file_alloc,
            max_evictable_bytes_per_shard,
        );

        ShmemTableSegmentRef { table }
    }

    /// Attach to an already initialized shared memory segment.
    ///
    /// Safety:
    ///  - The segment at this pointer must already be initialized by a
    ///    different process (or by the calling process itself).
    pub unsafe fn attach(
        file_start: *mut libc::c_void,
        file_size: usize,
    ) -> ShmemTableSegmentRef<'shm, T> {
        let (self_ptr, _) = Self::maybe_unint_ptr_and_next_free_byte(file_start, file_size);

        let segment: &'shm MaybeUninit<Self> = &*self_ptr;
        let segment = segment.assume_init_ref();

        let table = CMap::attach(&segment.table);
        ShmemTableSegmentRef { table }
    }

    unsafe fn maybe_unint_ptr_and_next_free_byte(
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
