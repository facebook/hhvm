// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(allocator_api)]

use std::alloc::{Allocator, Layout};
use std::convert::TryInto;
use std::hash::BuildHasherDefault;
use std::ptr::NonNull;

use lz4::liblz4;
use nohash_hasher::NoHashHasher;
use once_cell::unsync::OnceCell;

use shmrs::chashmap::{CMap, CMapRef};
use shmrs::mapalloc::MapAlloc;

use ocamlrep::{Value, STRING_TAG};
use ocamlrep_ocamlpool::catch_unwind;

type HashBuilder = BuildHasherDefault<NoHashHasher<u64>>;
type HackMap = CMapRef<'static, u64, HeapValue, HashBuilder>;

thread_local! {
  static CMAP: OnceCell<HackMap> = OnceCell::new();
}

extern "C" {
    fn caml_input_value_from_block(data: *const u8, size: usize) -> usize;
    fn caml_alloc_initialized_string(size: usize, data: *const u8) -> usize;
    fn caml_output_value_to_malloc(value: usize, flags: usize, ptr: *mut usize, len: *mut usize);

    // TODO(hverr): Switch to Rust buffer allocation.
    fn free(data: *const u8);
    fn malloc(size: libc::size_t) -> *mut u8;
}

#[derive(Clone, Copy)]
struct HeapValueHeader(u64);

impl HeapValueHeader {
    fn new(buffer_size: usize, uncompressed_size: usize, is_serialized: bool) -> Self {
        let buffer_size: u32 = buffer_size.try_into().unwrap();
        let uncompressed_size: u32 = uncompressed_size.try_into().unwrap();
        // Make sure the MSB are 0 because we will need 1 additional bits for
        // another field and 1 bit for OCaml's internals.
        assert_eq!(buffer_size & (1 << 31), 0);
        assert_eq!(uncompressed_size & (1 << 31), 0);

        let mut result: u64 = 0;
        result |= buffer_size as u64;
        result |= (uncompressed_size as u64) << 31;
        result |= (is_serialized as u64) << 62;
        Self(result)
    }

    /// Size of the buffer attached to this value.
    fn buffer_size(&self) -> usize {
        (self.0 & ((1 << 31) - 1)) as usize
    }

    /// Size if the buffer were uncompressed.
    fn uncompressed_size(&self) -> usize {
        ((self.0 >> 31) & ((1 << 31) - 1)) as usize
    }

    /// Was the buffer serialized, or does it contain a raw OCaml string?
    fn is_serialized(&self) -> bool {
        ((self.0 >> 62) & 1) == 1
    }

    /// Was the buffer compressed?
    fn is_compressed(&self) -> bool {
        self.uncompressed_size() != self.buffer_size()
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
struct HeapValue {
    header: HeapValueHeader,
    data: NonNull<u8>,
}

impl HeapValue {
    /// Convert the heap value into an OCaml object.
    ///
    /// Safety: this allocates in the OCaml heap, and thus enters the runtime.
    /// It may deallocate each and every object you haven't registered as a
    /// root.
    unsafe fn to_ocaml_value(&self) -> usize {
        if !self.header.is_serialized() {
            caml_alloc_initialized_string(self.header.buffer_size(), self.data.as_ptr())
        } else if !self.header.is_compressed() {
            caml_input_value_from_block(self.data.as_ptr(), self.header.buffer_size())
        } else {
            // TODO(hverr): Make thus more Rust-idiomatic
            let data = malloc(self.header.uncompressed_size());
            let uncompressed_size = liblz4::LZ4_decompress_safe(
                self.data.as_ptr() as *const i8,
                data as *mut i8,
                self.header.buffer_size().try_into().unwrap(),
                self.header.uncompressed_size().try_into().unwrap(),
            );
            assert!(self.header.uncompressed_size() == uncompressed_size as usize);

            let result = caml_input_value_from_block(data, uncompressed_size as libc::size_t);
            free(data);
            result
        }
    }

    fn clone_in(&self, alloc: &MapAlloc<'static>) -> HeapValue {
        let layout = Layout::from_size_align(self.header.buffer_size(), 1).unwrap();
        let mut data = alloc.allocate(layout).unwrap();
        // Safety: we are the only ones with access to the allocated chunk.
        unsafe {
            data.as_mut().copy_from_slice(self.as_slice())
        };

        HeapValue {
            header: self.header,
            data: data.cast(),
        }
    }

    fn as_slice(&self) -> &[u8] {
        let len = self.header.buffer_size();
        // Safety: We own the data. The return value cannot outlive `self`.
        unsafe { std::slice::from_raw_parts(self.data.as_ptr(), len) }
    }
}

enum SerializedValue<'a> {
    String(Value<'a>),
    Serialized {
        ptr: *mut u8,
        len: usize,
    },
    Compressed {
        ptr: *mut u8,
        len: usize,
        uncompressed_size: i32,
    },
}

impl<'a> SerializedValue<'a> {
    fn from(value: Value<'a>) -> Self {
        use SerializedValue::*;

        // We are entering the OCaml runtime, is there a risk
        // that `value` (or other values) get deallocated?
        // I don't think so: caml_output_value_to_malloc shouldn't
        // allocate on the OCaml heap, and thus not trigger the GC.
        if value
            .as_block()
            .map_or(false, |blck| blck.tag() == STRING_TAG)
        {
            String(value)
        } else {
            let mut ptr: usize = 0;
            let mut len: usize = 0;
            unsafe {
                caml_output_value_to_malloc(
                    value.to_bits(),
                    Value::int(0).to_bits(),
                    (&mut ptr) as *mut usize,
                    (&mut len) as *mut usize,
                )
            };
            Serialized {
                ptr: ptr as *mut u8,
                len,
            }
        }
    }

    fn as_slice(&self) -> &[u8] {
        use SerializedValue::*;
        match self {
            String(value) => value.as_byte_string().unwrap(),
            &Serialized { ptr, len } | &Compressed { ptr, len, .. } => unsafe {
                std::slice::from_raw_parts(ptr as *const u8, len)
            },
        }
    }

    fn maybe_compress(self) -> Self {
        use SerializedValue::*;
        match self {
            String(value) => String(value),
            Compressed {
                ptr,
                len,
                uncompressed_size,
            } => Compressed {
                ptr,
                len,
                uncompressed_size,
            },
            Serialized { ptr, len } => unsafe {
                let uncompressed_size: i32 = len.try_into().unwrap();
                let max_compression_size = liblz4::LZ4_compressBound(uncompressed_size);
                let compressed_data = malloc(max_compression_size as libc::size_t);
                let compressed_size = liblz4::LZ4_compress_default(
                    ptr as *const i8,
                    compressed_data as *mut i8,
                    uncompressed_size,
                    max_compression_size,
                );
                if compressed_size == 0 || compressed_size >= uncompressed_size {
                    free(compressed_data);
                    Serialized { ptr, len }
                } else {
                    free(ptr);
                    Compressed {
                        ptr: compressed_data,
                        len: compressed_size as usize,
                        uncompressed_size,
                    }
                }
            },
        }
    }

    fn free(self) {
        use SerializedValue::*;
        match self {
            String(..) => {}
            Serialized { ptr, .. } | Compressed { ptr, .. } => unsafe {
                free(ptr);
            },
        }
    }

    fn to_heap_value_in(&self, alloc: &MapAlloc<'static>) -> HeapValue {
        let slice = self.as_slice();

        let layout = Layout::from_size_align(slice.len(), 1).unwrap();
        let mut data = alloc.allocate(layout).unwrap();
        // Safety: we are the only ones with access to the allocated chunk.
        unsafe {
            data.as_mut().copy_from_slice(slice)
        };

        use SerializedValue::*;
        let header = match self {
            String(..) => HeapValueHeader::new(slice.len(), slice.len(), false),
            Serialized { .. } => HeapValueHeader::new(slice.len(), slice.len(), true),
            Compressed {
                uncompressed_size, ..
            } => HeapValueHeader::new(slice.len(), *uncompressed_size as usize, true),
        };

        HeapValue {
            header,
            data: data.cast(),
        }
    }

    pub fn uncompressed_size(&self) -> usize {
        use SerializedValue::*;
        match self {
            String(..) => self.as_slice().len(),
            Serialized { ptr: _, len } => *len,
            Compressed {
                ptr: _,
                uncompressed_size,
                len: _,
            } => *uncompressed_size as usize,
        }
    }

    pub fn compressed_size(&self) -> usize {
        use SerializedValue::*;
        match self {
            String(..) => self.as_slice().len(),
            Serialized { ptr: _, len } => *len,
            Compressed {
                ptr: _,
                uncompressed_size: _,
                len,
            } => *len,
        }
    }
}

fn with<R>(f: impl FnOnce(&HackMap) -> R) -> R {
    CMAP.with(|cell| f(cell.get().unwrap()))
}

#[no_mangle]
pub extern "C" fn shmffi_init(mmap_address: *mut libc::c_void, file_size: libc::size_t) {
    catch_unwind(|| {
        CMAP.with(move |cell| {
            assert!(cell.get().is_none());
            cell.get_or_init(move ||
            // Safety:
            //  - We are the only one initializing!
            unsafe {
                CMap::initialize_with_hasher(
                    BuildHasherDefault::default(),
                    mmap_address,
                    file_size,
                )
            });
        });

        0
    });
}

#[no_mangle]
pub extern "C" fn shmffi_attach(mmap_address: *mut libc::c_void, file_size: libc::size_t) {
    catch_unwind(|| {
        CMAP.with(move |cell| {
            assert!(cell.get().is_none());
            cell.get_or_init(move ||
            // Safety:
            //  - Should be already initialized by the master process.
            unsafe {
                CMap::attach(mmap_address, file_size)
            });
        });

        0
    });
}

#[no_mangle]
pub extern "C" fn shmffi_add(hash: u64, data: usize) -> usize {
    catch_unwind(|| {
        let data = unsafe { Value::from_bits(data) };
        let serialized = SerializedValue::from(data);
        let compressed = serialized.maybe_compress();
        let compressed_size = compressed.compressed_size();
        let uncompressed_size = compressed.uncompressed_size();
        with(|cmap| {
            cmap.write_map(&hash, |map, allocator| {
                let heap_value = compressed.to_heap_value_in(allocator);
                map.insert(hash, heap_value);
            })
        });
        compressed.free();

        // TODO(hverr): We don't have access to "total_size" (which includes
        // alignment overhead), remove the third field.
        let ret: (isize, isize, isize) = (
            compressed_size as isize,
            uncompressed_size as isize,
            compressed_size as isize,
        );
        unsafe { ocamlrep_ocamlpool::to_ocaml(&ret) }
    })
}

#[no_mangle]
pub extern "C" fn shmffi_get_and_deserialize(hash: u64) -> usize {
    catch_unwind(|| {
        with(|cmap| {
            cmap.read_map(&hash, |map| {
                let heap_value = &map[&hash];

                // Safety: we are not holding on to unrooted OCaml values.
                unsafe { heap_value.to_ocaml_value() }
            })
        })
    })
}

#[no_mangle]
pub extern "C" fn shmffi_mem(hash: u64) -> usize {
    catch_unwind(|| {
        let flag = with(|cmap| cmap.read_map(&hash, |map| map.contains_key(&hash)));
        Value::int(flag as isize).to_bits()
    })
}

#[no_mangle]
pub extern "C" fn shmffi_mem_status(hash: u64) -> usize {
    let flag = with(|cmap| cmap.read_map(&hash, |map| map.contains_key(&hash)));
    // From hh_shared.c: 1 = present, -1 = not present
    let result = if flag { 1 } else { -1 };
    Value::int(result).to_bits()
}

#[no_mangle]
pub extern "C" fn shmffi_get_size(hash: u64) -> usize {
    let size = with(|cmap| cmap.read_map(&hash, |map| map[&hash].header.buffer_size()));
    Value::int(size as isize).to_bits()
}

#[no_mangle]
pub extern "C" fn shmffi_move(hash1: u64, hash2: u64) {
    with(|cmap| {
        let value = cmap.write_map(&hash1, |map1, _allocator| map1.remove(&hash1).unwrap());
        cmap.write_map(&hash2, |map2, allocator| {
            let cloned_value = value.clone_in(allocator);
            map2.insert(hash2, cloned_value);
        });
    });
}

#[no_mangle]
pub extern "C" fn shmffi_remove(hash: u64) -> usize {
    let size = with(|cmap| {
        cmap.write_map(&hash, |map, _allocator| {
            let heap_value = map.remove(&hash).unwrap();
            heap_value.as_slice().len()
        })
    });
    Value::int(size as isize).to_bits()
}

#[no_mangle]
pub extern "C" fn shmffi_allocated_bytes() -> usize {
    catch_unwind(|| {
        let bytes = with(|cmap| cmap.allocated_bytes());
        Value::int(bytes as isize).to_bits()
    })
}

#[no_mangle]
pub extern "C" fn shmffi_num_entries() -> usize {
    catch_unwind(|| {
        let num_entries = with(|cmap| cmap.len());
        Value::int(num_entries as isize).to_bits()
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    use rand::prelude::*;

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

            let header = HeapValueHeader::new(buffer_size, uncompressed_size, is_serialized);

            assert_eq!(header.buffer_size(), buffer_size);
            assert_eq!(header.uncompressed_size(), uncompressed_size);
            assert_eq!(header.is_serialized(), is_serialized);
            assert_eq!(header.is_compressed(), buffer_size != uncompressed_size);
        }
    }
}
