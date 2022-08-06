// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ptr::NonNull;

use lz4::liblz4;
use shmrs::chashmap::CMapValue;

extern "C" {
    fn caml_input_value_from_block(data: *const u8, size: usize) -> usize;
    fn caml_alloc_initialized_string(size: usize, data: *const u8) -> usize;
    fn caml_output_value_to_malloc(value: usize, flags: usize, ptr: *mut *mut u8, len: *mut usize);
}

/// A struct to make sure we don't mix up fields in `HeapValueHeader` that
/// have the same type.
struct HeapValueHeaderFields {
    buffer_size: usize,
    uncompressed_size: usize,
    is_serialized: bool,
    is_evictable: bool,
}

impl From<HeapValueHeaderFields> for HeapValueHeader {
    fn from(fields: HeapValueHeaderFields) -> Self {
        HeapValueHeader::new(fields)
    }
}

#[derive(Clone, Copy)]
pub struct HeapValueHeader(u64);

impl HeapValueHeader {
    fn new(fields: HeapValueHeaderFields) -> Self {
        let buffer_size: u32 = fields.buffer_size.try_into().unwrap();
        let uncompressed_size: u32 = fields.uncompressed_size.try_into().unwrap();
        // Make sure the MSB are 0. We only have 31 bits for the sizes as we need
        // one additional bit for `is_serialized` and one bit to mark a value as
        // evictable or not.
        //
        // Note that we can use the full 64-bits. This header never escapes into the
        // OCaml world.
        assert_eq!(buffer_size & (1 << 31), 0);
        assert_eq!(uncompressed_size & (1 << 31), 0);

        let mut result: u64 = 0;
        result |= buffer_size as u64;
        result |= (uncompressed_size as u64) << 31;
        result |= (fields.is_serialized as u64) << 62;
        result |= (fields.is_evictable as u64) << 63;
        Self(result)
    }

    /// Size of the buffer attached to this value.
    pub fn buffer_size(&self) -> usize {
        (self.0 & ((1 << 31) - 1)) as usize
    }

    /// Size if the buffer were uncompressed.
    pub fn uncompressed_size(&self) -> usize {
        ((self.0 >> 31) & ((1 << 31) - 1)) as usize
    }

    /// Was the buffer serialized, or does it contain a raw OCaml string?
    pub fn is_serialized(&self) -> bool {
        ((self.0 >> 62) & 1) == 1
    }

    /// Was the buffer compressed?
    pub fn is_compressed(&self) -> bool {
        self.uncompressed_size() != self.buffer_size()
    }

    /// Is the value evictable?
    pub fn is_evictable(&self) -> bool {
        ((self.0 >> 63) & 1) == 1
    }
}

/// A value stored in shared-memory.
///
/// This is just a pointer to some buffer in shared-memory,
/// together with some metadata.
///
/// Note that it does not implement drop to deallocate the
/// underlying buffer. That would require tracking which
/// shard allocator was originally used to allocate the buffer,
/// as values can freely move between shards. The memory overhead
/// for this is prohibitively expensive.
pub struct HeapValue {
    pub header: HeapValueHeader,
    pub data: NonNull<u8>,
}

// Safety: The memory behind `data` is owned by this HeapValue, but
// we never write to that memory, only read.
//
// Most importantly, we never violate the the aliasing rule: we never
// create two mutable references to the same underlying data.
unsafe impl Send for HeapValue {}
unsafe impl Sync for HeapValue {}

impl HeapValue {
    /// Convert the heap value into an OCaml object.
    ///
    /// Safety: this allocates in the OCaml heap, and thus enters the runtime.
    /// It may deallocate each and every object you haven't registered as a
    /// root. It may even reallocate (i.e. move from the young generation to
    /// the old) values *inside* registered nodes). There's no guarantee that
    /// every object reachable from a root won't move!
    pub unsafe fn to_ocaml_value(&self) -> usize {
        if !self.header.is_serialized() {
            caml_alloc_initialized_string(self.header.buffer_size(), self.data.as_ptr())
        } else if !self.header.is_compressed() {
            caml_input_value_from_block(self.data.as_ptr(), self.header.buffer_size())
        } else {
            let mut data: Vec<u8> = Vec::with_capacity(self.header.uncompressed_size());
            let uncompressed_size = liblz4::LZ4_decompress_safe(
                self.data.as_ptr() as *const libc::c_char,
                data.as_mut_ptr() as *mut libc::c_char,
                self.header.buffer_size().try_into().unwrap(),
                self.header.uncompressed_size().try_into().unwrap(),
            );
            let uncompressed_size: usize = uncompressed_size.try_into().unwrap();
            assert!(self.header.uncompressed_size() == uncompressed_size);
            // SAFETY: `LZ4_decompress_safe` should have initialized
            // `uncompressed_size` bytes; we assert above that
            // `uncompressed_size` is equal to the capacity we set
            data.set_len(uncompressed_size);

            caml_input_value_from_block(data.as_ptr(), data.len())
        }
    }

    pub fn as_slice(&self) -> &[u8] {
        let len = self.header.buffer_size();
        // Safety: We own the data. The return value cannot outlive `self`.
        unsafe { std::slice::from_raw_parts(self.data.as_ptr(), len) }
    }
}

impl CMapValue for HeapValue {
    fn points_to_evictable_data(&self) -> bool {
        self.header.is_evictable()
    }
}

/// An OCaml serialized value, in all its forms.
///
/// Each `SerializedValue` is bound by a lifetime 'a because it might reference
/// a borrowed string (which may be on the OCaml heap, or it may be in
/// Rust-managed memory).
pub enum SerializedValue<'a> {
    /// A plain uncompressed byte string. Stored in shm as-is (i.e., without
    /// marshaling/serialization/compression).
    BStr(&'a [u8]),
    /// An OCaml serialized (marshaled) value, allocated on the heap via `malloc`.
    Serialized(MallocBuf),
    /// An OCaml serialized (marshaled) value, which was then compressed using lz4.
    Compressed {
        data: Vec<u8>,
        uncompressed_size: usize,
    },
}

/// A byte buffer allocated by `malloc`, via `caml_output_value_to_malloc`.
/// Essentially `Box<[u8]>`, but with a `Drop` impl that invokes `free`.
pub struct MallocBuf {
    ptr: *const u8,
    len: usize,
}

impl Drop for MallocBuf {
    fn drop(&mut self) {
        extern "C" {
            fn free(data: *const u8);
        }
        unsafe { free(self.ptr) };
    }
}

impl MallocBuf {
    fn as_slice(&self) -> &[u8] {
        unsafe { std::slice::from_raw_parts(self.ptr, self.len) }
    }
}

impl<'a> From<ocamlrep::Value<'a>> for SerializedValue<'a> {
    fn from(value: ocamlrep::Value<'a>) -> Self {
        // We are entering the OCaml runtime, is there a risk
        // that `value` (or other values) get deallocated?
        // I don't think so: caml_output_value_to_malloc shouldn't
        // allocate on the OCaml heap, and thus not trigger the GC.
        if let Some(str) = value.as_byte_string() {
            SerializedValue::BStr(str)
        } else {
            let mut ptr: *mut u8 = std::ptr::null_mut();
            let mut len: usize = 0;
            unsafe {
                caml_output_value_to_malloc(
                    value.to_bits(),
                    ocamlrep::Value::int(0).to_bits(),
                    &mut ptr,
                    &mut len,
                )
            };
            SerializedValue::Serialized(MallocBuf { ptr, len })
        }
    }
}

impl<'a> SerializedValue<'a> {
    pub fn as_slice(&self) -> &[u8] {
        use SerializedValue::*;
        match self {
            BStr(value) => value,
            Serialized(buf) => buf.as_slice(),
            Compressed { data, .. } => data,
        }
    }

    pub fn maybe_compress(self) -> Self {
        use SerializedValue::*;
        match self {
            this @ (BStr(..) | Compressed { .. }) => this,
            Serialized(buf) => unsafe {
                let uncompressed_size: i32 = buf.len.try_into().unwrap();
                let max_compression_size = liblz4::LZ4_compressBound(uncompressed_size);
                let mut compressed_data =
                    Vec::with_capacity(max_compression_size.try_into().unwrap());
                let compressed_size = liblz4::LZ4_compress_default(
                    buf.ptr as *const libc::c_char,
                    compressed_data.as_mut_ptr() as *mut libc::c_char,
                    uncompressed_size,
                    max_compression_size,
                );
                if compressed_size == 0 || compressed_size >= uncompressed_size {
                    Serialized(buf)
                } else {
                    // SAFETY: `LZ4_compress_default` should have initialized
                    // `compressed_size` bytes (which should be no more than
                    // `max_compression_size` bytes, which is our vec's
                    // capacity).
                    compressed_data.set_len(compressed_size.try_into().unwrap());
                    Compressed {
                        data: compressed_data,
                        uncompressed_size: buf.len,
                    }
                }
            },
        }
    }

    pub fn to_heap_value_in(&self, is_evictable: bool, buffer: &mut [u8]) -> HeapValue {
        let slice = self.as_slice();
        buffer.copy_from_slice(slice);

        use SerializedValue::*;
        let header = match self {
            BStr(..) => HeapValueHeaderFields {
                buffer_size: slice.len(),
                uncompressed_size: slice.len(),
                is_serialized: false,
                is_evictable,
            },
            Serialized { .. } => HeapValueHeaderFields {
                buffer_size: slice.len(),
                uncompressed_size: slice.len(),
                is_serialized: true,
                is_evictable,
            },
            Compressed {
                uncompressed_size, ..
            } => HeapValueHeaderFields {
                buffer_size: slice.len(),
                uncompressed_size: *uncompressed_size as usize,
                is_serialized: true,
                is_evictable,
            },
        };

        HeapValue {
            header: header.into(),
            data: NonNull::from(buffer).cast(),
        }
    }

    pub fn uncompressed_size(&self) -> usize {
        use SerializedValue::*;
        match self {
            BStr(data) => data.len(),
            Serialized(buf) => buf.len,
            &Compressed {
                uncompressed_size, ..
            } => uncompressed_size,
        }
    }

    pub fn compressed_size(&self) -> usize {
        use SerializedValue::*;
        match self {
            BStr(data) => data.len(),
            Serialized(buf) => buf.len,
            Compressed { data, .. } => data.len(),
        }
    }
}

#[cfg(test)]
mod tests {
    use rand::prelude::*;

    use super::*;

    #[test]
    fn test_heap_value_header() {
        const NUM_TESTS: usize = 100;
        let mut rng = StdRng::from_seed([0; 32]);

        for _ in 0..NUM_TESTS {
            let buffer_size = (rng.gen::<u32>() & ((1 << 31) - 1)) as usize;
            let uncompressed_size = if rng.gen_bool(0.5) {
                buffer_size
            } else {
                (rng.gen::<u32>() & ((1 << 31) - 1)) as usize
            };
            let is_serialized = rng.gen_bool(0.5);
            let is_evictable = rng.gen_bool(0.5);

            let header = HeapValueHeaderFields {
                buffer_size,
                uncompressed_size,
                is_serialized,
                is_evictable,
            };
            let header: HeapValueHeader = header.into();

            assert_eq!(header.buffer_size(), buffer_size);
            assert_eq!(header.uncompressed_size(), uncompressed_size);
            assert_eq!(header.is_serialized(), is_serialized);
            assert_eq!(header.is_compressed(), buffer_size != uncompressed_size);
            assert_eq!(header.is_evictable(), is_evictable);
        }
    }

    #[test]
    fn test_to_heap_value() {
        fn test_once(x: SerializedValue<'_>) {
            let x = x.maybe_compress();
            let mut buffer = vec![0_u8; x.as_slice().len()];

            let heap_value = x.to_heap_value_in(true, &mut buffer);
            assert!(heap_value.header.is_evictable());
            assert_eq!(heap_value.as_slice(), x.as_slice());

            let heap_value = x.to_heap_value_in(false, &mut buffer);
            assert!(!heap_value.header.is_evictable());
            assert_eq!(heap_value.as_slice(), x.as_slice());
        }

        fn malloc_buf(buf: &[u8]) -> MallocBuf {
            extern "C" {
                fn malloc(size: usize) -> *mut u8;
            }
            let ptr = unsafe { malloc(buf.len()) };
            assert_ne!(ptr, std::ptr::null_mut());
            let slice = unsafe { std::slice::from_raw_parts_mut(ptr, buf.len()) };
            slice.copy_from_slice(buf);
            MallocBuf {
                ptr,
                len: buf.len(),
            }
        }

        test_once(SerializedValue::BStr(&[
            0xdb, 0x7f, 0x13, 0xa6, 0xab, 0x0e, 0x51, 0x74, 0x2b,
        ]));

        let buf = malloc_buf(&[0xdb, 0x7f, 0x13, 0xa6, 0xab, 0x0e, 0x51, 0x74, 0x2b]);
        test_once(SerializedValue::Serialized(buf));

        let buf = malloc_buf(&vec![95; 1024]);
        test_once(SerializedValue::Serialized(buf));
    }
}
