// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(allocator_api)]

use std::alloc::Layout;
use std::convert::TryInto;
use std::io::Write;
use std::ptr::NonNull;
use std::sync::OnceLock;

use ocaml_blob::HeapValue;
use ocaml_blob::HeapValueHeader;
use ocaml_blob::SerializedValue;
use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::Allocator;
use ocamlrep::Value;
use ocamlrep_ocamlpool::catch_unwind;
use shmrs::chashmap::MINIMUM_EVICTABLE_BYTES_PER_SHARD;
use shmrs::chashmap::NUM_SHARDS;
use shmrs::segment::ShmemTableSegment;
use shmrs::segment::ShmemTableSegmentRef;

pub static SEGMENT: OnceLock<ShmemTableSegmentRef<'static, HeapValue>> = OnceLock::new();

pub fn with<R>(f: impl FnOnce(&ShmemTableSegmentRef<'static, HeapValue>) -> R) -> R {
    f(SEGMENT.get().unwrap())
}

#[no_mangle]
pub extern "C" fn shmffi_init(
    mmap_address: *mut libc::c_void,
    file_size: libc::size_t,
    max_evictable_bytes: libc::ssize_t,
) {
    // The `max_evictable_bytes` argument to the `shmffi_init` function
    // might be negative to indicate that evictability is disabled.
    //
    // We'll initialize the maps anyways, but with minimum-capacity allocators.
    let max_evictable_bytes = std::cmp::max(
        (NUM_SHARDS * MINIMUM_EVICTABLE_BYTES_PER_SHARD)
            .try_into()
            .unwrap(),
        max_evictable_bytes,
    ) as libc::size_t;
    catch_unwind(|| {
        if SEGMENT
            .set(
                // Safety:
                //  - We are the only one initializing!
                unsafe {
                    ShmemTableSegment::initialize(
                        mmap_address,
                        file_size,
                        max_evictable_bytes / NUM_SHARDS,
                    )
                },
            )
            .is_err()
        {
            panic!("Unexpected prior value in SEGMENT");
        }
        0
    });
}

#[no_mangle]
pub extern "C" fn shmffi_attach(mmap_address: *mut libc::c_void, file_size: libc::size_t) {
    catch_unwind(|| {
        if SEGMENT
            .set(
                // Safety:
                //  - Should be already initialized by the master process.
                unsafe { ShmemTableSegment::attach(mmap_address, file_size) },
            )
            .is_err()
        {
            panic!("Unexpected prior value in SEGMENT");
        }
        0
    });
}

#[no_mangle]
pub extern "C" fn shmffi_add(evictable: bool, hash: u64, data: usize) -> usize {
    catch_unwind(|| {
        let data = unsafe { Value::from_bits(data) };
        let serialized = SerializedValue::from(data);
        let compressed = serialized.maybe_compress();
        let compressed_size = compressed.compressed_size();
        let uncompressed_size = compressed.uncompressed_size();
        let did_insert = with(|segment| {
            segment.table.insert(
                hash,
                Some(Layout::from_size_align(compressed.as_slice().len(), 1).unwrap()),
                evictable,
                |buffer| compressed.to_heap_value_in(evictable, buffer),
            )
        });

        // TODO(hverr): We don't have access to "total_size" (which includes
        // alignment overhead), remove the third field.
        let ret: (isize, isize, isize) = if did_insert {
            (
                compressed_size as isize,
                uncompressed_size as isize,
                compressed_size as isize,
            )
        } else {
            (-1, -1, -1)
        };
        unsafe { ocamlrep_ocamlpool::to_ocaml(&ret) }
    })
}

/// Writes the given data directly into the shared memory.
/// `data` should be an OCaml `bytes` object returned by
/// `shmffi_get_raw` or `shmffi_serialize_raw`.
#[no_mangle]
pub extern "C" fn shmffi_add_raw(hash: u64, ocaml_bytes: usize) -> usize {
    catch_unwind(|| {
        let ocaml_bytes = unsafe { Value::from_bits(ocaml_bytes) };
        let ocaml_bytes = ocaml_bytes.as_byte_string().unwrap();
        let header_size = HeapValueHeader::RAW_SIZE;
        let header = HeapValueHeader::from_raw(ocaml_bytes[0..header_size].try_into().unwrap());
        with(|segment| {
            segment.table.insert(
                hash,
                Some(Layout::from_size_align(header.buffer_size(), 1).unwrap()),
                header.is_evictable(),
                |buffer| {
                    buffer.copy_from_slice(&ocaml_bytes[header_size..]);
                    HeapValue {
                        header,
                        data: NonNull::from(buffer).cast(),
                    }
                },
            );
        });

        // Returns unit
        ocamlrep::Value::int(0).to_bits()
    })
}

#[no_mangle]
pub extern "C" fn shmffi_get_and_deserialize(hash: u64) -> usize {
    catch_unwind(|| {
        with(|segment| {
            let result = match segment.table.read(&hash).get() {
                None => None,
                Some(heap_value) => {
                    // Safety: we are not holding on to unrooted OCaml values.
                    //
                    // This value itself is unrooted, but we are not calling into
                    // the OCaml runtime after this. The option that will be allocated
                    // later is allocated via ocamlpool, which cannot trigger the GC.
                    let deserialized_value = unsafe { heap_value.to_ocaml_value() };

                    // Safety: the value is only used to wrap it in an option.
                    //
                    // Because we use ocamlpool below, the GC won't run while this
                    // value exists.
                    let deserialized_value = unsafe { UnsafeOcamlPtr::new(deserialized_value) };

                    Some(deserialized_value)
                }
            };

            // Safety: we don't call into the OCaml runtime, so there's no
            // risk of us GC'ing the deserialized value.
            unsafe { ocamlrep_ocamlpool::to_ocaml(&result) }
        })
    })
}

/// Looks up the entry with the corresponding hash and return
/// its raw form as an OCaml `bytes option`, without deserializing.
/// The result is meant to be passed to `shmffi_deserialize_raw`
/// and `shmffi_add_raw`, potentially over the network.
#[no_mangle]
pub extern "C" fn shmffi_get_raw(hash: u64) -> usize {
    catch_unwind(|| {
        with(|segment| {
            let result = segment.table.read(&hash).get().map(|heap_value| {
                // Encode header and data
                let header_size = HeapValueHeader::RAW_SIZE;
                let value_slice = heap_value.as_slice();
                let ocaml_bytes_len = header_size + value_slice.len();
                // Safety: we assume that the OCaml runtime is suspended by a
                // call into this function, and we're not calling into the OCaml
                // runtime from any other thread. We are calling into the OCaml
                // runtime here, but via ocamlpool, which cannot trigger a GC.
                let pool = unsafe { ocamlrep_ocamlpool::Pool::new() };
                let mut byte_string = pool.byte_string_with_len(ocaml_bytes_len);
                byte_string.write_all(&heap_value.header.to_raw()).unwrap();
                byte_string.write_all(value_slice).unwrap();
                // Safety: Because we only interact with the runtime via
                // ocamlpool below, the GC won't run while this value exists.
                unsafe { UnsafeOcamlPtr::new(byte_string.build().to_bits()) }
            });

            // Safety: we don't call into the OCaml runtime, so there's no
            // risk of us GC'ing the deserialized value.
            unsafe { ocamlrep_ocamlpool::to_ocaml(&result) }
        })
    })
}

/// Takes an OCaml `bytes` object as returned by `shmffi_serialize_raw`
/// or `shmffi_get_raw`, and deserialize it back into an OCaml value.
#[no_mangle]
pub extern "C" fn shmffi_deserialize_raw(ocaml_bytes: usize) -> usize {
    catch_unwind(|| {
        // First we have to copy the OCaml buffer contents to a native buffer,
        // because deserializing with to_ocaml_value can cause OCaml GC to activate
        let ocaml_bytes = unsafe { Value::from_bits(ocaml_bytes) };
        let bytes_copy = ocaml_bytes.as_byte_string().unwrap().to_vec();

        unsafe {
            // Construct a HeapValue so we can use to_ocaml_value
            // Safety: bytes_copy will outlive `heap_value`
            let header_size = HeapValueHeader::RAW_SIZE;
            let header = HeapValueHeader::from_raw(bytes_copy[0..header_size].try_into().unwrap());
            let heap_value = HeapValue {
                header,
                data: NonNull::from(&bytes_copy[header_size]),
            };

            // Deserialize into destination type
            // Safety: We made sure to copy ocaml_bytes to a native buffer before deserializing
            heap_value.to_ocaml_value()
        }
    })
}

/// Takes an OCaml value and serializes it into a form suitable
/// for sending over the network, and for usage with `shmffi_deserialize_raw`
/// and `shmffi_add_raw`. Returns an OCaml `bytes` object.
#[no_mangle]
pub extern "C" fn shmffi_serialize_raw(data: usize) -> usize {
    catch_unwind(|| {
        // Serialize and compress
        let data = unsafe { Value::from_bits(data) };
        let serialized = SerializedValue::from(data);
        let compressed = serialized.maybe_compress();

        let header = compressed.make_header(false /* evictable */);

        // Encode header and data
        let header_size = HeapValueHeader::RAW_SIZE;
        let value_slice = compressed.as_slice();
        let ocaml_bytes_len = header_size + value_slice.len();
        // Safety: we assume that the OCaml runtime is suspended by a
        // call into this function, and we're not calling into the OCaml
        // runtime from any other thread. We are calling into the OCaml
        // runtime here, but via ocamlpool, which cannot trigger a GC.
        let pool = unsafe { ocamlrep_ocamlpool::Pool::new() };
        let mut byte_string = pool.byte_string_with_len(ocaml_bytes_len);
        byte_string.write_all(&header.to_raw()).unwrap();
        byte_string.write_all(value_slice).unwrap();
        byte_string.build().to_bits()
    })
}

#[no_mangle]
pub extern "C" fn shmffi_mem(hash: u64) -> usize {
    catch_unwind(|| {
        let flag = with(|segment| segment.table.contains_key(&hash));
        Value::int(flag as isize).to_bits()
    })
}

#[no_mangle]
pub extern "C" fn shmffi_mem_status(hash: u64) -> usize {
    let flag = with(|segment| segment.table.contains_key(&hash));
    // From hh_shared.c: 1 = present, -1 = not present
    let result = if flag { 1 } else { -1 };
    Value::int(result).to_bits()
}

#[no_mangle]
pub extern "C" fn shmffi_get_size(hash: u64) -> usize {
    let size = with(|segment| {
        segment
            .table
            .read(&hash)
            .get()
            .map(|value| value.header.buffer_size())
    });
    let size = size.unwrap_or(0);
    Value::int(size as isize).to_bits()
}

#[no_mangle]
pub extern "C" fn shmffi_move(hash1: u64, hash2: u64) {
    with(|segment| {
        let (header, data) = segment.table.inspect_and_remove(&hash1, |value| {
            let value = value.unwrap();
            (value.header, <Box<[u8]>>::from(value.as_slice()))
        });
        segment.table.insert(
            hash2,
            Some(Layout::from_size_align(data.len(), 1).unwrap()),
            header.is_evictable(),
            |buffer| {
                buffer.copy_from_slice(&data);
                HeapValue {
                    header,
                    data: std::ptr::NonNull::new(buffer.as_mut_ptr()).unwrap(),
                }
            },
        );
    });
}

#[no_mangle]
pub extern "C" fn shmffi_remove(hash: u64) -> usize {
    let size = with(|segment| {
        segment
            .table
            .inspect_and_remove(&hash, |value| value.unwrap().as_slice().len())
    });
    Value::int(size as isize).to_bits()
}

#[no_mangle]
pub extern "C" fn shmffi_allocated_bytes() -> usize {
    catch_unwind(|| {
        let bytes = with(|segment| segment.table.allocated_bytes());
        Value::int(bytes as isize).to_bits()
    })
}

#[no_mangle]
pub extern "C" fn shmffi_num_entries() -> usize {
    catch_unwind(|| {
        let num_entries = with(|segment| segment.table.len());
        Value::int(num_entries as isize).to_bits()
    })
}
