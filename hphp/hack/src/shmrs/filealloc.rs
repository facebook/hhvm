// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::alloc::{AllocError, Allocator, Layout};
use std::ptr::NonNull;
use std::sync::atomic::{AtomicUsize, Ordering};

/// An allocator that can allocate chunks from a file.
pub struct FileAlloc {
    file_start: *mut libc::c_void,
    file_size: usize,
    next_free_byte: AtomicUsize,
}

impl FileAlloc {
    /// Create a new file allocator.
    ///
    /// - `file_start` points to the beginning of the file
    /// - `file_size` is the total file size
    /// - `next_free_byte` is a locked `u64` indicating the next free byte
    pub fn new(file_start: *mut libc::c_void, file_size: usize, next_free_byte: usize) -> Self {
        Self {
            file_start,
            file_size,
            next_free_byte: AtomicUsize::new(next_free_byte),
        }
    }
}

unsafe impl Allocator for FileAlloc {
    fn allocate(&self, l: Layout) -> Result<NonNull<[u8]>, AllocError> {
        // A read-cmpxchg loop. Try to move the next-free-byte pointer but
        // check for out-of-memory errors. Retry if an other allocation has
        // succeeded in the meantime.
        loop {
            // We are only dealing with access to one atomic. There's no
            // other loads/stores that need to be synchronized. As such, we
            // can use relaxed ordering throughout.
            let next_free_byte = self.next_free_byte.load(Ordering::Relaxed);
            let ptr: *mut u8 = self.file_start as *mut u8;
            let ptr = unsafe { ptr.add(next_free_byte) };
            let align_offset = ptr.align_offset(l.align());
            let total_bytes = match align_offset.checked_add(l.size()) {
                None => return Err(AllocError),
                Some(total_bytes) => total_bytes,
            };

            if total_bytes > self.file_size - next_free_byte {
                return Err(AllocError);
            }

            let new_next_free_byte = next_free_byte + total_bytes;
            match self.next_free_byte.compare_exchange(
                next_free_byte,
                new_next_free_byte,
                Ordering::Relaxed,
                Ordering::Relaxed,
            ) {
                Ok(_) => {
                    let ptr = unsafe { ptr.add(align_offset) };
                    let slice = unsafe { std::slice::from_raw_parts(ptr, l.size()) };
                    return Ok(NonNull::from(slice));
                }
                Err(_) => {
                    // Try again!
                }
            }
        }
    }

    unsafe fn deallocate(&self, _ptr: NonNull<u8>, _layout: Layout) {
        // Doesn't do anything
    }
}
