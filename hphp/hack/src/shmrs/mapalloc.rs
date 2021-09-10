// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::alloc::{AllocError, Allocator, Layout};
use std::cell::RefCell;
use std::ptr::NonNull;

use crate::filealloc::FileAlloc;

const LARGE_ALLOCATION: usize = 4096;
const CHUNK_SIZE: usize = 200 * 1024;

/// Structure that contains the control data for a map allocator.
///
/// This structure should be allocated in shared memory. Turn it
/// into an actual allocator by combining it with a `FileAlloc` using
/// `MapAlloc::new`.
pub struct MapAllocControlData {
    inner: RefCell<MapAllocControlDataInner>,
}

struct MapAllocControlDataInner {
    current: *mut u8,
    end: *mut u8,
}

impl MapAllocControlData {
    /// A new empty allocator. Useful as a placeholder.
    pub fn new() -> Self {
        Self {
            inner: RefCell::new(MapAllocControlDataInner {
                current: std::ptr::null_mut(),
                end: std::ptr::null_mut(),
            }),
        }
    }
}

/// An allocator used in shared memory hash maps.
///
/// For now, the map allocator is a bumping allocator that requests chunks from
/// the underlying file allocator.
///
/// Since its control structures lives somewhere in shared memory, it's bound
/// by a lifetime parameter that represents the lifetime of the shared memory
/// region.
#[derive(Clone)]
pub struct MapAlloc<'shm> {
    control_data: &'shm MapAllocControlData,
    file_alloc: &'shm FileAlloc,
}

impl<'shm> MapAlloc<'shm> {
    /// Create a new map allocator using the given control data and file
    /// allocator.
    ///
    /// Safety:
    ///  - Each `MapAlloc` must have a unique `MapAllocControlData`.
    ///    Otherwise, two maps might be trying to mutate the same control data,
    ///    which is undefined behavior. (Note that the `RefCell` only
    ///    guarantees mutability uniqueness constraints within one process.
    ///    Forking bypasses the `!Sync` trait on `RefCell`).
    pub unsafe fn new(
        control_data: &'shm MapAllocControlData,
        file_alloc: &'shm FileAlloc,
    ) -> Self {
        Self {
            control_data,
            file_alloc,
        }
    }

    fn alloc_large(&self, l: Layout) -> Result<NonNull<[u8]>, AllocError> {
        self.file_alloc.allocate(l)
    }

    fn extend(&self, control_data: &mut MapAllocControlDataInner) -> Result<(), AllocError> {
        let l = Layout::from_size_align(CHUNK_SIZE, 1).unwrap();
        let ptr = self.file_alloc.allocate(l)?;
        unsafe {
            control_data.current = ptr.as_ptr() as *mut u8;
            control_data.end = control_data.current.add(CHUNK_SIZE);
        }
        Ok(())
    }
}

unsafe impl<'shm> Allocator for MapAlloc<'shm> {
    fn allocate(&self, l: Layout) -> Result<NonNull<[u8]>, AllocError> {
        let size = l.size();
        if size > LARGE_ALLOCATION {
            return self.alloc_large(l);
        }

        let mut control_data = self.control_data.inner.borrow_mut();

        let align_offset = control_data.current.align_offset(l.align());
        let mut pointer = unsafe { control_data.current.add(align_offset) };
        let mut new_current = unsafe { pointer.add(size) };
        if new_current > control_data.end {
            self.extend(&mut control_data)?;
            let align_offset = control_data.current.align_offset(l.align());
            pointer = unsafe { control_data.current.add(align_offset) };
            new_current = unsafe { pointer.add(size) };
        }
        control_data.current = new_current;
        let slice = unsafe { std::slice::from_raw_parts(pointer, size) };
        Ok(NonNull::from(slice))
    }

    unsafe fn deallocate(&self, _ptr: NonNull<u8>, _layout: Layout) {
        // Doesn't do anything.
        //
        // Note that for deallocate to work, we will have to make our allocator
        // control structure thread-safe: Even though, for allocation, we can
        // only access the allocator when the corresponding map is locked (and thus
        // guarantee we're the only ones allocating), ownership of any object that
        // was allocated using this allocator, may be transferred outside of the
        // critical region protected by the corresponding map lock. As such,
        // deallocations might happen outside that critical region, no longer
        // guaranteeing unique access to this allocator.
    }
}
