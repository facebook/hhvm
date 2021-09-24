// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::alloc::{AllocError, Allocator, Layout};
use std::ptr::NonNull;

use crate::filealloc::FileAlloc;
use crate::sync::RwLockRef;

const LARGE_ALLOCATION: usize = 4096;
const CHUNK_SIZE: usize = 200 * 1024;

/// Structure that contains the control data for a map allocator.
///
/// This structure should be allocated in shared memory. Turn it
/// into an actual allocator by combining it with a `FileAlloc` using
/// `MapAlloc::new`.
pub struct MapAllocControlData {
    current: *mut u8,
    end: *mut u8,
}

impl MapAllocControlData {
    /// A new empty allocator. Useful as a placeholder.
    pub fn new() -> Self {
        Self {
            current: std::ptr::null_mut(),
            end: std::ptr::null_mut(),
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
    control_data: RwLockRef<'shm, MapAllocControlData>,
    file_alloc: &'shm FileAlloc,
}

impl<'shm> MapAlloc<'shm> {
    /// Create a new map allocator using the given lock-protected control
    /// data and a file allocator.
    pub unsafe fn new(
        control_data: RwLockRef<'shm, MapAllocControlData>,
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

    fn extend(&self, control_data: &mut MapAllocControlData) -> Result<(), AllocError> {
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

        let mut control_data = self.control_data.write().unwrap();

        let align_offset = control_data.current.align_offset(l.align());
        let mut pointer = unsafe { control_data.current.add(align_offset) };
        let mut new_current = unsafe { pointer.add(size) };
        // We must make sure we don't produce null pointers when `size == 0`
        // and `new_current == NULL`.
        if new_current > control_data.end || new_current.is_null() {
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
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use crate::sync::RwLock;

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
            let core_data = RwLock::new(MapAllocControlData::new());
            let core_data_ref = unsafe { core_data.initialize().unwrap() };
            let alloc = unsafe { MapAlloc::new(core_data_ref, file_alloc) };

            let layout = std::alloc::Layout::from_size_align(0, 1).unwrap();
            let _ = alloc.allocate(layout).unwrap();
        })
    }
}
