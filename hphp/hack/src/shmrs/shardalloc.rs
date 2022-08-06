// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::alloc::AllocError;
use std::alloc::Allocator;
use std::alloc::Layout;
use std::ptr::NonNull;

use crate::filealloc::FileAlloc;
use crate::sync::RwLockRef;

/// A pointer to a chunk.
#[repr(transparent)]
#[derive(Clone, Copy)]
struct ChunkPtr(*mut u8);

impl ChunkPtr {
    fn null() -> Self {
        ChunkPtr(std::ptr::null_mut())
    }

    fn is_null(self) -> bool {
        self.0.is_null()
    }

    fn is_aligned(self) -> bool {
        self.0.align_offset(std::mem::align_of::<ChunkPtr>()) == 0
    }

    fn next_chunk_ptr(self) -> *mut ChunkPtr {
        assert!(!self.is_null());
        assert!(self.is_aligned());

        // Yes, you read that right, a pointer to a pointer!
        let chunk_next_ptr: *mut ChunkPtr = self.0 as _;
        chunk_next_ptr
    }

    fn get_next_chunk(self) -> ChunkPtr {
        let chunk_next_ptr = self.next_chunk_ptr();

        // Safety: ptr is (1) valid (2) aligned (3) it points to a initialized value
        unsafe { chunk_next_ptr.read() }
    }

    fn set_next_chunk(self, value: ChunkPtr) {
        let chunk_next_ptr = self.next_chunk_ptr();

        // Safety: ptr is (1) valid (2) aligned
        unsafe { chunk_next_ptr.write(value) };
    }
}

/// Structure that contains the control data for a shard allocator.
///
/// This structure should be allocated in shared memory. Turn it
/// into an actual allocator by combining it with a `FileAlloc` using
/// `ShardAlloc::new`.
pub struct ShardAllocControlData {
    /// A linked-list of filled chunks. Might be null.
    ///
    /// The first word is aligned and points to the next element of the
    /// linked list.
    filled_chunks: ChunkPtr,

    /// A linked-list of free chunks. Might be null.
    ///
    /// The first word is aligned and points to the next element of the
    /// linked list.
    free_chunks: ChunkPtr,

    /// Pointer to the first byte of the current chunk.
    ///
    /// Note that the first word of the chunk is reserved for metadata
    /// (i.e. a pointer that can be set if the chunk is added to the
    /// filled or free chunks list).
    current_start: ChunkPtr,

    /// Pointer to the next free byte in the current chunk.
    ///
    /// Might be null if no current chunk has been initialized yet.
    current_next: *mut u8,

    /// End of the current chunk. Do not allocate past this pointer.
    current_end: *mut u8,
}

/**
* Safety:
* - The methods of ShardAllocControlData below all mutate the direct fields
*   by taking &mut self, so there is no concurrent writes to the fields themselves
* - ChunkPtr is a bookkeeping struct, and we perform all mutations to the inner
    linked list pointers via methods taking &mut self, so again we are
    protected by the upper level rwlock.
* - current_next and current_end are simply raw pointer types for bookkeeping
*   and are not dereferenced directly in concurrent context
*/
unsafe impl Sync for ShardAllocControlData {}
unsafe impl Send for ShardAllocControlData {}

impl ShardAllocControlData {
    /// A new empty allocator. Useful as a placeholder.
    pub fn new() -> Self {
        Self {
            filled_chunks: ChunkPtr::null(),
            free_chunks: ChunkPtr::null(),
            current_start: ChunkPtr::null(),
            current_next: std::ptr::null_mut(),
            current_end: std::ptr::null_mut(),
        }
    }
}

impl ShardAllocControlData {
    /// Mark the current chunk as filled by adding it to the "filled chunks"
    /// list.
    fn mark_current_chunk_as_filled(&mut self) {
        if self.current_start.is_null() {
            return;
        }

        self.current_start.set_next_chunk(self.filled_chunks);
        self.filled_chunks = self.current_start;

        self.current_start = ChunkPtr::null();
        self.current_next = std::ptr::null_mut();
        self.current_end = std::ptr::null_mut();
    }

    /// Mark the currently filled chunks as free!
    fn mark_filled_chunks_as_free(&mut self) {
        // Find the last "filled chunk"
        let mut last_filled = ChunkPtr::null();
        let mut this_filled = self.filled_chunks;
        while !this_filled.is_null() {
            last_filled = this_filled;
            this_filled = this_filled.get_next_chunk();
        }
        if last_filled.is_null() {
            // Nothing to move
            return;
        }

        // Update its next pointer.
        last_filled.set_next_chunk(self.free_chunks);
        self.free_chunks = self.filled_chunks;
        self.filled_chunks = ChunkPtr::null();
    }

    fn set_current_chunk(&mut self, chunk_start: ChunkPtr, chunk_size: usize) {
        chunk_start.set_next_chunk(ChunkPtr::null());
        self.current_start = chunk_start;
        self.current_next = unsafe { chunk_start.0.add(std::mem::size_of::<*mut u8>()) };
        self.current_end = unsafe { chunk_start.0.add(chunk_size) };
    }

    /// Pop a free chunk of the free list. Update the current-chunk pointers.
    ///
    /// Returns true on success, false if no free chunk was available.
    fn pop_free_chunk(&mut self, chunk_size: usize) -> bool {
        if self.free_chunks.is_null() {
            return false;
        }

        let current_chunk = self.free_chunks;
        self.free_chunks = current_chunk.get_next_chunk();
        self.set_current_chunk(current_chunk, chunk_size);
        true
    }
}

/// The minimum chunk size an allocator can be initialized with.
///
/// `ShardAlloc::new` will panic if given a smaller `chunk_size`.
pub const SHARD_ALLOC_MIN_CHUNK_SIZE: usize = 64;

/// An allocator used for shared memory hash maps.
///
/// For now, each shard allocator is a bumping allocator that requests chunks from
/// the underlying file allocator.
///
/// Since its control structures lives somewhere in shared memory, it's bound
/// by a lifetime parameter that represents the lifetime of the shared memory
/// region.
#[derive(Clone)]
pub struct ShardAlloc<'shm> {
    /// Mutable control data.
    control_data: RwLockRef<'shm, ShardAllocControlData>,

    /// Underlying file allocator used to request new chunks and allocate
    /// large chunks.
    file_alloc: &'shm FileAlloc,

    /// Is the allocator of a fixed pre-allocated size? If this flag is true
    /// the allocator will refuse to allocate stuff once it's first chunk is
    /// full.
    is_fixed_size: bool,

    /// The chunk size that the allocator will use to allocate new chunks.
    chunk_size: usize,
}

impl<'shm> ShardAlloc<'shm> {
    /// Create a new shard allocator using the given lock-protected control
    /// data and a file allocator.
    ///
    /// This function will fail if `chunk_size` < `SHARD_ALLOC_MIN_CHUNK_SIZE`
    /// bytes. As some of the first bytes of a chunk are used as a header.
    pub unsafe fn new(
        control_data: RwLockRef<'shm, ShardAllocControlData>,
        file_alloc: &'shm FileAlloc,
        chunk_size: usize,
        is_fixed_size: bool,
    ) -> Self {
        assert!(chunk_size >= SHARD_ALLOC_MIN_CHUNK_SIZE);
        Self {
            control_data,
            file_alloc,
            is_fixed_size,
            chunk_size,
        }
    }

    fn alloc_large(&self, l: Layout) -> Result<NonNull<[u8]>, AllocError> {
        self.file_alloc.allocate(l)
    }

    fn extend(&self, control_data: &mut ShardAllocControlData) -> Result<(), AllocError> {
        control_data.mark_current_chunk_as_filled();
        if !control_data.pop_free_chunk(self.chunk_size) {
            let l =
                Layout::from_size_align(self.chunk_size, std::mem::align_of::<ChunkPtr>()).unwrap();
            let ptr = self.file_alloc.allocate(l)?;
            control_data.set_current_chunk(ChunkPtr(ptr.as_ptr() as *mut u8), self.chunk_size);
        }
        Ok(())
    }

    /// Reset the allocator.
    ///
    /// All previously allocated chunks will be marked as free.
    ///
    /// Safety:
    ///
    ///  - Of course, all values that were previously allocated using this
    ///    allocator are now garbage. You shouldn't try to read them anymore!
    pub unsafe fn reset(&self) {
        let mut control_data = self.control_data.write(None).unwrap();
        control_data.mark_current_chunk_as_filled();
        control_data.mark_filled_chunks_as_free();
    }
}

unsafe impl<'shm> Allocator for ShardAlloc<'shm> {
    fn allocate(&self, l: Layout) -> Result<NonNull<[u8]>, AllocError> {
        let size = l.size();

        // Large allocations go directly to the underlying file allocator.
        // We'll consider an allocation as large if it is larger than 5% of
        // the chunk size. That means, unusable memory due to internal
        // fragmentation will be less than 5%.
        //
        // We don't special case large allocations when the allocator size is
        // fixed.
        if !self.is_fixed_size && size > self.chunk_size / 20 {
            return self.alloc_large(l);
        }

        let mut control_data = self.control_data.write(None).unwrap();

        let align_offset = control_data.current_next.align_offset(l.align());
        let mut pointer = unsafe { control_data.current_next.add(align_offset) };
        let mut new_current = unsafe { pointer.add(size) };
        // We must make sure we don't produce null pointers when `size == 0`
        // and `new_current == NULL`.
        if new_current > control_data.current_end || new_current.is_null() {
            // Refuse to allocate another chunk if this allocator is marked to
            // have a fixed size
            if self.is_fixed_size && !control_data.current_next.is_null() {
                return Err(AllocError);
            }

            self.extend(&mut control_data)?;
            let align_offset = control_data.current_next.align_offset(l.align());
            pointer = unsafe { control_data.current_next.add(align_offset) };
            new_current = unsafe { pointer.add(size) };
        }
        control_data.current_next = new_current;
        let slice = unsafe { std::slice::from_raw_parts(pointer, size) };
        Ok(NonNull::from(slice))
    }

    unsafe fn deallocate(&self, _ptr: NonNull<u8>, _layout: Layout) {
        // Doesn't do anything.
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::sync::RwLock;

    const CHUNK_SIZE: usize = 200 * 1024;
    const FILE_ALLOC_SIZE: usize = 10 * 1024 * 1024;

    fn with_file_alloc(f: impl FnOnce(&FileAlloc)) {
        let mut vec: Vec<u8> = vec![0; FILE_ALLOC_SIZE];
        let vec_ptr = vec.as_mut_ptr();
        let file_alloc = FileAlloc::new(vec_ptr as *mut _, FILE_ALLOC_SIZE, 0);
        f(&file_alloc);
        drop(file_alloc);
        drop(vec);
    }

    #[test]
    fn test_alloc_zero() {
        with_file_alloc(|file_alloc| {
            let core_data = RwLock::new(ShardAllocControlData::new());
            let core_data_ref = unsafe { core_data.initialize().unwrap() };
            let alloc = unsafe { ShardAlloc::new(core_data_ref, file_alloc, CHUNK_SIZE, false) };

            let layout = std::alloc::Layout::from_size_align(0, 1).unwrap();
            let _ = alloc.allocate(layout).unwrap();
        })
    }
}
