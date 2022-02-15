// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(allocator_api)]

use std::alloc::Layout;
use std::convert::TryInto;
use std::hash::BuildHasherDefault;
use std::mem::MaybeUninit;
use std::ptr::NonNull;

use lz4::liblz4;
use nohash_hasher::NoHashHasher;
use once_cell::unsync::OnceCell;

use shmrs::chashmap::{CMap, CMapRef, CMapValue, NUM_SHARDS};
use shmrs::filealloc::FileAlloc;

use ocamlrep::{ptr::UnsafeOcamlPtr, Value, STRING_TAG};
use ocamlrep_ocamlpool::catch_unwind;

extern "C" {
    fn caml_input_value_from_block(data: *const u8, size: usize) -> usize;
    fn caml_alloc_initialized_string(size: usize, data: *const u8) -> usize;
    fn caml_output_value_to_malloc(value: usize, flags: usize, ptr: *mut usize, len: *mut usize);

    fn free(data: *const u8);
    fn malloc(size: libc::size_t) -> *mut u8;
}

/// The u64s are the first 8 bytes of a shasum. As such, they should have
/// enough entropy to be used directly as keys into the hashmap.
///
/// Note that the underlying hash table implementation (hashbrown) requires
/// entropy in both the upper and the lower bits of this u64. If this
/// assumption ever changes, performance will suffer.
type HashBuilder = BuildHasherDefault<NoHashHasher<u64>>;

thread_local! {
  static SEGMENT: OnceCell<ShmemSegmentRef<'static>> = OnceCell::new();
}

/// Shared memory segment that is used to store the sharded hashtable.
///
/// The shared memory pointer that we obtain from `mmap` is directly cast
/// to this struct. Its fields are all `MaybeUninit` because we can't
/// initialize them all at once.
struct ShmemSegment<'shm> {
    file_alloc: MaybeUninit<FileAlloc>,
    table: MaybeUninit<CMap<'shm, u64, HeapValue, HashBuilder>>,
}

/// A reference to an attached shared memory segment.
///
/// This struct is merely a reference to the shared memory segment and
/// the data it contains. As such, it is process-local.
///
/// Obtained by calling `initialize` or `attach` on `ShmemSegment`.
struct ShmemSegmentRef<'shm> {
    table: CMapRef<'shm, u64, HeapValue, HashBuilder>,
}

impl<'shm> ShmemSegment<'shm> {
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
    ) -> ShmemSegmentRef<'shm> {
        let (self_ptr, next_free_byte) =
            Self::maybe_unint_ptr_and_next_free_byte(file_start, file_size);

        // Safety: Doing this cast assumes:
        //  - The lifetime matches.
        //  - We are the sole users of the underlying memory.
        let segment: &'shm mut MaybeUninit<Self> = &mut *self_ptr;
        segment.write(ShmemSegment {
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

        ShmemSegmentRef { table }
    }

    /// Attach to an already initialized shared memory segment.
    ///
    /// Safety:
    ///  - The segment at this pointer must already be initialized by a
    ///    different process (or by the calling process itself).
    pub unsafe fn attach(file_start: *mut libc::c_void, file_size: usize) -> ShmemSegmentRef<'shm> {
        let (self_ptr, _) = Self::maybe_unint_ptr_and_next_free_byte(file_start, file_size);

        let segment: &'shm MaybeUninit<Self> = &*self_ptr;
        let segment = segment.assume_init_ref();

        let table = CMap::attach(&segment.table);
        ShmemSegmentRef { table }
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
struct HeapValueHeader(u64);

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

    /// Is the value evictable?
    fn is_evictable(&self) -> bool {
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
struct HeapValue {
    header: HeapValueHeader,
    data: NonNull<u8>,
}

impl HeapValue {
    /// Convert the heap value into an OCaml object.
    ///
    /// Safety: this allocates in the OCaml heap, and thus enters the runtime.
    /// It may deallocate each and every object you haven't registered as a
    /// root. It may even reallocate (i.e. move from the young generation to
    /// the old) values *inside* registered nodes). There's no guarantee that
    /// every object reachable from a root won't move!
    unsafe fn to_ocaml_value(&self) -> usize {
        if !self.header.is_serialized() {
            caml_alloc_initialized_string(self.header.buffer_size(), self.data.as_ptr())
        } else if !self.header.is_compressed() {
            caml_input_value_from_block(self.data.as_ptr(), self.header.buffer_size())
        } else {
            // Ideally, we'd have this a bit more Rust-idiomatic, but
            // unfortunately, we can't get rid of all the malloc free-ing
            // due to our interaction with OCaml.
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

    fn as_slice(&self) -> &[u8] {
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
/// Each `SerializedValue` is bound by a lifetime 'a because it
/// might reference an OCaml value on the OCaml heap.
enum SerializedValue<'a> {
    /// An OCaml STRING value on the OCaml heap.
    CamlString(Value<'a>),
    /// An OCaml STRING value on the Rust heap (owned by Rust)
    OwnedString { ptr: *mut u8, len: usize },
    /// An OCaml serialized (marshaled) value.
    Serialized { ptr: *mut u8, len: usize },
    /// An OCaml serialized (marshaled) value, which was then
    /// compressed.
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
            CamlString(value)
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
            CamlString(value) => value.as_byte_string().unwrap(),
            &OwnedString { ptr, len } | &Serialized { ptr, len } | &Compressed { ptr, len, .. } => unsafe {
                std::slice::from_raw_parts(ptr as *const u8, len)
            },
        }
    }

    fn maybe_compress(self) -> Self {
        use SerializedValue::*;
        match self {
            CamlString(value) => CamlString(value),
            OwnedString { ptr, len } => OwnedString { ptr, len },
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
            CamlString(..) => {}
            OwnedString { ptr, .. } | Serialized { ptr, .. } | Compressed { ptr, .. } => unsafe {
                free(ptr);
            },
        }
    }

    fn layout_for_buffer(&self) -> Layout {
        Layout::from_size_align(self.as_slice().len(), 1).unwrap()
    }

    fn to_heap_value_in(&self, is_evictable: bool, buffer: &mut [u8]) -> HeapValue {
        let slice = self.as_slice();
        buffer.copy_from_slice(slice);

        use SerializedValue::*;
        let header = match self {
            OwnedString { .. } | CamlString(..) => HeapValueHeaderFields {
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

    fn from_heap_value(heap_value: &HeapValue) -> (Self, bool) {
        let buffer_size = heap_value.header.buffer_size();
        let uncompressed_size = heap_value.header.uncompressed_size().try_into().unwrap();
        let is_evictable = heap_value.header.is_evictable();

        let ptr = unsafe { malloc(buffer_size) };
        assert_ne!(ptr, std::ptr::null_mut());
        let data = unsafe { std::slice::from_raw_parts_mut(ptr, buffer_size) };
        data.clone_from_slice(heap_value.as_slice());
        let len = data.len();

        use SerializedValue::*;
        let serialized_value = if !heap_value.header.is_serialized() {
            assert!(!heap_value.header.is_compressed());
            OwnedString { ptr, len }
        } else if !heap_value.header.is_compressed() {
            Serialized { ptr, len }
        } else {
            Compressed {
                ptr,
                len,
                uncompressed_size,
            }
        };
        (serialized_value, is_evictable)
    }

    pub fn uncompressed_size(&self) -> usize {
        use SerializedValue::*;
        match self {
            OwnedString { .. } | CamlString(..) => self.as_slice().len(),
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
            OwnedString { .. } | CamlString(..) => self.as_slice().len(),
            Serialized { ptr: _, len } => *len,
            Compressed {
                ptr: _,
                uncompressed_size: _,
                len,
            } => *len,
        }
    }
}

fn with<R>(f: impl FnOnce(&ShmemSegmentRef<'static>) -> R) -> R {
    SEGMENT.with(|cell| f(cell.get().unwrap()))
}

#[no_mangle]
pub extern "C" fn shmffi_init(
    mmap_address: *mut libc::c_void,
    file_size: libc::size_t,
    max_evictable_bytes: libc::ssize_t,
) {
    // max_evictable_bytes might be negative to indicate that
    // evictability is disabled.
    //
    // We'll initialize the maps anyways, but with zero-capacity allocators.
    let max_evictable_bytes = std::cmp::max(0, max_evictable_bytes) as libc::size_t;
    catch_unwind(|| {
        SEGMENT.with(move |cell| {
            assert!(cell.get().is_none());
            cell.get_or_init(move ||
            // Safety:
            //  - We are the only one initializing!
            unsafe {
                ShmemSegment::initialize(
                    mmap_address,
                    file_size,
                    max_evictable_bytes / NUM_SHARDS,
                )
            });
        });

        0
    });
}

#[no_mangle]
pub extern "C" fn shmffi_attach(mmap_address: *mut libc::c_void, file_size: libc::size_t) {
    catch_unwind(|| {
        SEGMENT.with(move |cell| {
            assert!(cell.get().is_none());
            cell.get_or_init(move ||
            // Safety:
            //  - Should be already initialized by the master process.
            unsafe {
                ShmemSegment::attach(mmap_address, file_size)
            });
        });

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
                Some(compressed.layout_for_buffer()),
                evictable,
                |buffer| compressed.to_heap_value_in(evictable, buffer),
            )
        });
        compressed.free();

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

#[no_mangle]
pub extern "C" fn shmffi_get_and_deserialize(hash: u64) -> usize {
    catch_unwind(|| {
        with(|segment| {
            let result = match segment.table.get(&hash) {
                None => None,
                Some(heap_value) => {
                    // Safety: we are not holding on to unrooted OCaml values.
                    //
                    // This value itself is unrooted, but we are not calling into
                    // the OCalm runtime after this. The option that will be allocated
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
            .get(&hash)
            .map(|value| value.header.buffer_size())
    });
    let size = size.unwrap_or(0);
    Value::int(size as isize).to_bits()
}

#[no_mangle]
pub extern "C" fn shmffi_move(hash1: u64, hash2: u64) {
    with(|segment| {
        let (serialized_value, evictable) = segment.table.inspect_and_remove(&hash1, |value| {
            SerializedValue::from_heap_value(value.unwrap())
        });
        segment.table.insert(
            hash2,
            Some(serialized_value.layout_for_buffer()),
            evictable,
            |buffer| serialized_value.to_heap_value_in(evictable, buffer),
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
    fn test_heap_value_from_into() {
        fn assert_serialized_value_eq(x: &SerializedValue<'_>, y: &SerializedValue<'_>) {
            use SerializedValue::*;
            let (x_ptr, x_len, y_ptr, y_len) = match (x, y) {
                (CamlString(..), CamlString(..)) => unimplemented!(),
                (
                    &OwnedString {
                        ptr: x_ptr,
                        len: x_len,
                    },
                    &OwnedString {
                        ptr: y_ptr,
                        len: y_len,
                    },
                ) => (x_ptr, x_len, y_ptr, y_len),
                (
                    &Serialized {
                        ptr: x_ptr,
                        len: x_len,
                    },
                    &Serialized {
                        ptr: y_ptr,
                        len: y_len,
                    },
                ) => (x_ptr, x_len, y_ptr, y_len),
                (
                    &Compressed {
                        ptr: x_ptr,
                        len: x_len,
                        uncompressed_size: x_uncompressed_size,
                    },
                    &Compressed {
                        ptr: y_ptr,
                        len: y_len,
                        uncompressed_size: y_uncompressed_size,
                    },
                ) => {
                    assert_eq!(x_uncompressed_size, y_uncompressed_size);
                    (x_ptr, x_len, y_ptr, y_len)
                }
                (_, _) => panic!(),
            };
            assert_eq!(x_len, y_len);
            let x_slice = unsafe { std::slice::from_raw_parts(x_ptr, x_len) };
            let y_slice = unsafe { std::slice::from_raw_parts(y_ptr, y_len) };
            assert_eq!(x_slice, y_slice);
        }

        fn test_once(x: SerializedValue<'_>) {
            let x = x.maybe_compress();
            let mut buffer = vec![0_u8; x.as_slice().len()];

            let heap_value = x.to_heap_value_in(true, &mut buffer);
            let (y, evictable) = SerializedValue::from_heap_value(&heap_value);
            assert_serialized_value_eq(&x, &y);
            assert!(evictable);

            let heap_value = x.to_heap_value_in(false, &mut buffer);
            let (y, evictable) = SerializedValue::from_heap_value(&heap_value);
            assert_serialized_value_eq(&x, &y);
            assert!(!evictable);
        }

        fn buf_to_ptr(buf: &[u8]) -> (*mut u8, usize) {
            let ptr = unsafe { malloc(buf.len()) };
            assert_ne!(ptr, std::ptr::null_mut());
            let slice = unsafe { std::slice::from_raw_parts_mut(ptr, buf.len()) };
            slice.copy_from_slice(buf);
            (ptr, buf.len())
        }

        let (ptr, len) = buf_to_ptr(&[0xdb, 0x7f, 0x13, 0xa6, 0xab, 0x0e, 0x51, 0x74, 0x2b]);
        test_once(SerializedValue::OwnedString { ptr, len });

        let (ptr, len) = buf_to_ptr(&[0xdb, 0x7f, 0x13, 0xa6, 0xab, 0x0e, 0x51, 0x74, 0x2b]);
        test_once(SerializedValue::Serialized { ptr, len });

        let (ptr, len) = buf_to_ptr(&vec![95; 1024]);
        test_once(SerializedValue::Serialized { ptr, len });
    }
}
