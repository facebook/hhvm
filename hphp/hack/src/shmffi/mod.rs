// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(allocator_api)]

use std::convert::TryInto;
use std::hash::BuildHasherDefault;

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
    fn caml_alloc_initialized_string(data: *const u8, size: usize) -> usize;
    fn caml_output_value_to_malloc(value: usize, flags: usize, ptr: *mut usize, len: *mut usize);

    // TODO(hverr): Switch to Rust buffer allocation.
    fn free(data: *const u8);
    fn malloc(size: libc::size_t) -> *mut u8;
}

struct HeapValue {
    is_serialized: bool,
    is_compressed: bool,
    uncompressed_size: i32,
    data: Vec<u8, MapAlloc<'static>>,
}

impl HeapValue {
    /// Convert the heap value into an OCaml object.
    ///
    /// Safety: this allocates in the OCaml heap, and thus enters the runtime.
    /// It may deallocate each and every object you haven't registered as a
    /// root.
    unsafe fn to_ocaml_value(&self) -> usize {
        if !self.is_serialized {
            caml_alloc_initialized_string(self.data.as_ptr(), self.data.len())
        } else if !self.is_compressed {
            caml_input_value_from_block(self.data.as_ptr(), self.data.len())
        } else {
            // TODO(hverr): Make thus more Rust-idiomatic
            let data = malloc(self.uncompressed_size as libc::size_t);
            let uncompressed_size = liblz4::LZ4_decompress_safe(
                self.data.as_ptr() as *const i8,
                data as *mut i8,
                self.data.len().try_into().unwrap(),
                self.uncompressed_size,
            );
            assert!(self.uncompressed_size == uncompressed_size);

            let result = caml_input_value_from_block(data, uncompressed_size as libc::size_t);
            free(data);
            result
        }
    }

    fn clone_in(&self, alloc: MapAlloc<'static>) -> HeapValue {
        let mut data = Vec::with_capacity_in(self.data.len(), alloc);
        data.resize(self.data.len(), 0);
        data.as_mut_slice().copy_from_slice(&self.data);

        HeapValue {
            is_serialized: self.is_serialized,
            is_compressed: self.is_compressed,
            uncompressed_size: self.uncompressed_size,
            data,
        }
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

    fn to_heap_value_in(&self, alloc: MapAlloc<'static>) -> HeapValue {
        let slice = self.as_slice();
        let mut data = Vec::with_capacity_in(slice.len(), alloc);
        data.resize(slice.len(), 0);
        data.as_mut_slice().copy_from_slice(slice);

        use SerializedValue::*;
        match self {
            String(..) => HeapValue {
                is_serialized: false,
                is_compressed: false,
                uncompressed_size: 0,
                data,
            },
            Serialized { .. } => HeapValue {
                is_serialized: true,
                is_compressed: false,
                uncompressed_size: 0,
                data,
            },
            Compressed {
                uncompressed_size, ..
            } => HeapValue {
                is_serialized: true,
                is_compressed: true,
                uncompressed_size: *uncompressed_size,
                data,
            },
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
        with(|cmap| {
            cmap.write_map(&hash, |map| {
                // Safety: allocator will not escape critical section.
                //
                // TODO(hverr): make allocators thread-safe.
                let allocator = unsafe { map.allocator() };
                let heap_value = compressed.to_heap_value_in(allocator);
                map.insert(hash, heap_value);
            })
        });
        compressed.free();

        // TODO(hverr): Return correct values
        let ret: (isize, isize, isize) = (0, 0, 0);
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
    let size = with(|cmap| cmap.read_map(&hash, |map| map[&hash].data.len()));
    Value::int(size as isize).to_bits()
}

#[no_mangle]
pub extern "C" fn shmffi_move(hash1: u64, hash2: u64) {
    with(|cmap| {
        let value = cmap.write_map(&hash1, |map1| map1.remove(&hash1).unwrap());
        cmap.write_map(&hash2, |map2| {
            // Safety: does not escape critical section!
            let allocator = unsafe { map2.allocator() };
            let cloned_value = value.clone_in(allocator);
            map2.insert(hash2, cloned_value);
        });
    });
}

#[no_mangle]
pub extern "C" fn shmffi_remove(hash: u64) -> usize {
    let size = with(|cmap| cmap.write_map(&hash, |map| map.remove(&hash).unwrap().data.len()));
    Value::int(size as isize).to_bits()
}
