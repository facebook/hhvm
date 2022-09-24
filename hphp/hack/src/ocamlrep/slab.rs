// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Utilities for storing OCaml values in contiguous, movable byte strings.
//!
//! This module provides utilities for writing a tree of OCaml values into any
//! slice of memory. The term "slab" is used to mean a word-aligned slice of
//! memory containing a tree of OCaml values (reachable from a single root
//! value). When a slab is copied to a new address, pointers in its values
//! (which must all be pointers to other values within the slab) must be updated
//! to reflect the slab's new location in memory. This fixup operation is called
//! "rebasing".

use std::cell::RefCell;
use std::collections::HashSet;
use std::fmt;
use std::fmt::Debug;
use std::mem;
use std::sync::atomic::AtomicUsize;
use std::sync::atomic::Ordering;

use bytes::buf::UninitSlice;

use crate::block::Header;
use crate::block::NO_SCAN_TAG;
use crate::Allocator;
use crate::BlockBuilder;
use crate::MemoizationCache;
use crate::OpaqueValue;
use crate::SlabIntegrityError;
use crate::ToOcamlRep;
use crate::Value;

// The first three words in a slab are for the base pointer, the offset (in
// words) of the root value from that base, and the magic number.
const SLAB_METADATA_WORDS: usize = 3;

// This secret number marks properly initialized slabs.
const SLAB_MAGIC_NUMBER: usize = 0x51A851A8;

const WORD_SIZE: usize = mem::size_of::<OpaqueValue<'_>>();

// A contiguous memory region holding a tree of OCaml values reachable from a
// single root value.
//
// The first three values are reserved for Slab metadata. The remaining values
// make up a "block-data region" for the allocated value tree.
//
// The metadata words are:
//
//   - The base pointer, which indicates the address at which the Slab was
//     stored when its pointers were last updated (in a "rebase" operation).
//
//   - The root value offset, which is the offset (in words) from the base
//     pointer to the root value of the allocated tree.
//
//   - A magic number, written only after the value tree is allocated. The
//     presence of the magic number indicates that this is a valid Slab.
//
// Example:
//
// Recall that memory is byte addressable. On a 64-bit architecture, two "words"
// that are adjacent to each other have 8 bytes between them. So, if the
// `value` p is the memory address x then, the next word pointed to by p + 1 is
// the address x + 8.
//
// `Some "a"` rebased to to address 0x00
// =====================================
//
// | address | value                         | comment                      |
// | ------- | ----------------------------- | ---------------------------- |
// | 0x00    | 0x00                          | base pointer                 |
// | 0x08    | 4                             | root value offset            |
// | 0x10    | 0x51A851A8                    | magic number                 |
// | 0x18    | Header { size: 1, tag: 0 }    | Some's tag is 0              |
// | 0x20    | 0x30                          | pointer (base ptr + 6 words) |
// | 0x28    | Header { size: 1, tag: 252 }, | STRING_TAG = 252             |
// | 0x30    | 0x0600000000000061,           | "a" (0x06=num padding bytes) |
type Slab<'a> = [OpaqueValue<'a>];

// Private convenience methods for Slab (it's a bit easier to define them as
// extension methods for slices than to define Slab as an unsized wrapper type).
trait SlabTrait {
    fn from_bytes(bytes: &[u8]) -> &Self;
    fn from_bytes_mut(bytes: &mut [u8]) -> &mut Self;
    // SAFETY: the caller must guarantee that this slab will not be used to read from unitialized
    // memory.
    unsafe fn from_uninit_slice_mut(bytes: &mut UninitSlice) -> &mut Self;

    // metadata accessors
    fn base(&self) -> usize;
    fn root_value_offset(&self) -> usize;
    fn is_initialized(&self) -> bool;

    // metadata setters
    fn set_base(&mut self, base: usize);
    fn set_root_value_offset(&mut self, offset: usize);
    fn mark_initialized(&mut self);

    fn current_address(&self) -> usize;
    fn needs_rebase(&self) -> bool;
    fn value(&self) -> Option<Value<'_>>;
    unsafe fn rebase_to(&mut self, new_base: usize);
    fn check_initialized(&self) -> Result<(), SlabIntegrityError>;
    fn check_integrity(&self) -> Result<(), SlabIntegrityError>;
}

// When embedding a Slab in a byte string, we need to include leading padding
// bytes so that the Slab is word-aligned.
#[inline]
fn leading_padding(ptr: *const u8, len: usize) -> usize {
    let misalignment = ptr as usize % WORD_SIZE;
    let padding = (WORD_SIZE - misalignment) % WORD_SIZE;
    std::cmp::min(padding, len)
}

impl<'a> SlabTrait for Slab<'a> {
    fn from_bytes(bytes: &[u8]) -> &Self {
        let padding = leading_padding(bytes.as_ptr(), bytes.len());
        let ptr = bytes[padding..].as_ptr() as *const OpaqueValue<'a>;
        unsafe { std::slice::from_raw_parts(ptr, bytes.len() / WORD_SIZE) }
    }

    fn from_bytes_mut(bytes: &mut [u8]) -> &mut Self {
        let padding = leading_padding(bytes.as_ptr(), bytes.len());
        let ptr = bytes[padding..].as_mut_ptr() as *mut OpaqueValue<'a>;
        unsafe { std::slice::from_raw_parts_mut(ptr, bytes.len() / WORD_SIZE) }
    }

    unsafe fn from_uninit_slice_mut(bytes: &mut UninitSlice) -> &mut Self {
        // SAFETY: leading_padding does not read from the pointer (this points at unitialized
        // memory and should not be read from).
        let padding = leading_padding(bytes.as_mut_ptr(), bytes.len());
        let ptr = bytes[padding..].as_mut_ptr() as *mut OpaqueValue<'a>;
        std::slice::from_raw_parts_mut(ptr, bytes.len() / WORD_SIZE)
    }

    fn base(&self) -> usize {
        self[0].to_bits()
    }

    fn root_value_offset(&self) -> usize {
        self[1].to_bits()
    }

    fn is_initialized(&self) -> bool {
        self[2].to_bits() == SLAB_MAGIC_NUMBER
    }

    fn set_base(&mut self, base: usize) {
        self[0] = unsafe { OpaqueValue::from_bits(base) };
    }

    fn set_root_value_offset(&mut self, offset: usize) {
        self[1] = unsafe { OpaqueValue::from_bits(offset) };
    }

    fn mark_initialized(&mut self) {
        self[2] = unsafe { OpaqueValue::from_bits(SLAB_MAGIC_NUMBER) };
    }

    fn current_address(&self) -> usize {
        self.as_ptr() as usize
    }

    /// Return false if the slab's base address is equal to its current location
    /// in memory (and thus its internal pointers are up-to-date, and it is safe
    /// to interpret the root value as an OCaml value). Otherwise, return true.
    fn needs_rebase(&self) -> bool {
        self.base() != self.current_address()
    }

    /// Return the root value stored in this slab, if the slab does not need
    /// rebasing. Otherwise, return None. Panics if the slab is not initialized.
    fn value(&self) -> Option<Value<'_>> {
        if !self.is_initialized() {
            panic!("slab not initialized");
        }
        if self.needs_rebase() {
            return None;
        }
        unsafe {
            Some(Value::from_bits(
                &self[self.root_value_offset()] as *const _ as usize,
            ))
        }
    }

    /// # Safety
    ///
    /// Undefined behavior if `self` is not a valid slab
    /// (i.e., `self.check_integrity()` would return `Err`).
    unsafe fn rebase_to(&mut self, new_base: usize) {
        let diff = new_base as isize - self.base() as isize;
        rebase_slab_value(&mut self[SLAB_METADATA_WORDS..], diff);
        self.set_base(new_base);
    }

    fn check_initialized(&self) -> Result<(), SlabIntegrityError> {
        use SlabIntegrityError::*;
        let len = self.len();

        // Since we don't store immediate values in slabs, the smallest possible
        // slab contains the metadata plus a block of size 1, which uses 1 word
        // for the header and one for its field.
        if len < SLAB_METADATA_WORDS + 2 {
            return Err(TooSmall(len));
        }

        if !self.is_initialized() {
            return Err(NotInitialized);
        }

        let root_offset = self.root_value_offset();
        if root_offset < SLAB_METADATA_WORDS + 1 || root_offset >= len {
            return Err(InvalidRootValueOffset(root_offset));
        }

        let base = self.base();
        if base % WORD_SIZE != 0 {
            return Err(InvalidBasePointer(base));
        }

        Ok(())
    }

    fn check_integrity(&self) -> Result<(), SlabIntegrityError> {
        use SlabIntegrityError::*;

        self.check_initialized()?;

        let len = self.len();
        let base = self.base();
        let root_offset = self.root_value_offset();

        // The set of offsets to valid blocks, according to the header data.
        let mut value_offsets = HashSet::new();

        // The set of offsets found in pointers in block fields.
        let mut offsets_from_pointers = HashSet::new();

        let mut i = SLAB_METADATA_WORDS;
        while i < len {
            let header = self[i].as_header();
            let size = header.size();
            if i + size + 1 > len {
                return Err(InvalidBlockSize(size));
            }
            value_offsets.insert(i + 1);
            if header.tag() >= NO_SCAN_TAG {
                // Skip binary blocks--they can't contain pointers
                i += size + 1;
            } else {
                // Skip header, then validate pointer fields
                i += 1;
                for value in self[i..(i + size)].iter().filter(|v| !v.is_immediate()) {
                    let ptr = value.to_bits();
                    if ptr < base || ptr % WORD_SIZE != 0 {
                        return Err(InvalidPointer(ptr));
                    }
                    let offset = (ptr - base) / WORD_SIZE;
                    if offset < SLAB_METADATA_WORDS + 1 || offset >= len {
                        return Err(InvalidPointer(ptr));
                    }
                    offsets_from_pointers.insert(offset);
                }
                // Advance to next header
                i += size;
            }
        }

        if !value_offsets.contains(&root_offset) {
            return Err(InvalidRootValueOffset(root_offset));
        }

        if let Some(ptr) = offsets_from_pointers.difference(&value_offsets).next() {
            return Err(InvalidPointer(*ptr));
        }

        Ok(())
    }
}

/// # Safety
///
/// `slab_without_metadata` must be a valid slab (i.e., if the metadata words
/// were included, `slab.check_integrity()` would return `Ok(())`).
unsafe fn rebase_slab_value(slab_without_metadata: &mut [OpaqueValue<'_>], diff: isize) {
    let mut ptr = slab_without_metadata.as_mut_ptr();
    let end = ptr.add(slab_without_metadata.len());
    while ptr < end {
        let header = (*ptr).as_header();
        let size = header.size();
        if header.tag() >= NO_SCAN_TAG {
            // Skip binary blocks--they can't contain pointers
            ptr = ptr.add(size + 1);
        } else {
            // Skip header, then rebase pointer fields
            ptr = ptr.add(1);
            assert!(ptr.add(size) <= end);
            for _ in 0..size {
                (*ptr).add_ptr_offset(diff);
                ptr = ptr.add(1);
            }
        }
    }
}

fn debug_slab(name: &'static str, slab: &Slab<'_>, f: &mut fmt::Formatter<'_>) -> fmt::Result {
    struct DebugBlock<'a>(&'a [OpaqueValue<'a>], usize);
    impl Debug for DebugBlock<'_> {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            let offset_or_address = self.1;
            if self.0[0].as_header().tag() >= NO_SCAN_TAG {
                let value = unsafe { Value::from_bits(&self.0[1] as *const _ as usize) };
                write!(f, "0x{:x}: {:?}", offset_or_address, value)
            } else {
                write!(f, "0x{:x}: {:?}", offset_or_address, &self.0[1..])
            }
        }
    }
    struct DebugPtr(usize);
    impl Debug for DebugPtr {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            write!(f, "0x{:x}", self.0)
        }
    }

    if let Err(err) = slab.check_integrity() {
        f.debug_struct(name)
            .field("current_address", &DebugPtr(slab.current_address()))
            .field("invalid", &err)
            .field("data", &slab)
            .finish()
    } else {
        let mut blocks = vec![];
        let mut i = SLAB_METADATA_WORDS;
        while i < slab.len() {
            let offset = (i + 1) * WORD_SIZE;
            let offset_or_address = if slab.needs_rebase() {
                offset
            } else {
                slab.current_address() + offset
            };
            let size = slab[i].as_header().size() + 1;
            blocks.push(DebugBlock(&slab[i..(i + size)], offset_or_address));
            i += size;
        }
        let root_value = slab.root_value_offset() * WORD_SIZE;
        f.debug_struct(name)
            .field("base", &DebugPtr(slab.base()))
            .field("current_address", &DebugPtr(slab.current_address()))
            .field("root_value_offset", &DebugPtr(root_value))
            .field("blocks", &blocks)
            .finish()
    }
}

// The generation number is used solely to identify which arena a cached value
// belongs to in `RcOc`.
//
// We use usize::max_value() / 4 here to avoid colliding with ocamlpool and
// Arena generation numbers (ocamlpool starts at 0, and Arena starts at
// usize::max_value() / 2). This generation trick isn't sound with the use of
// multiple generation counters, but this mitigation should make it extremely
// difficult to mix up values allocated with ocamlpool, Arena, and SlabAllocator
// in practice (one would have to serialize the same value with multiple
// Allocators, and only after increasing the generation of one by an absurd
// amount).
//
// If we add more allocators, we might want to rethink this strategy.
static NEXT_GENERATION: AtomicUsize = AtomicUsize::new(usize::max_value() / 4);

struct SlabAllocator {
    generation: usize,
    data: RefCell<Vec<OpaqueValue<'static>>>,
    cache: MemoizationCache,
}

impl SlabAllocator {
    /// Create a new `SlabAllocator` with 4KB of capacity preallocated.
    fn new() -> Self {
        Self::with_capacity(1024 * 4)
    }

    /// Create a new `SlabAllocator` with `capacity_in_bytes` preallocated.
    fn with_capacity(capacity_in_bytes: usize) -> Self {
        let generation = NEXT_GENERATION.fetch_add(1, Ordering::SeqCst);
        let capacity_in_words = std::cmp::max(2, capacity_in_bytes / WORD_SIZE);
        let mut data = Vec::with_capacity(capacity_in_words);
        for _ in 0..SLAB_METADATA_WORDS {
            data.push(unsafe { OpaqueValue::from_bits(0) });
        }
        Self {
            generation,
            data: RefCell::new(data),
            cache: MemoizationCache::new(),
        }
    }

    #[inline]
    fn alloc<'a>(&'a self, size: usize, tag: u8) -> BlockBuilder<'a> {
        let size_with_header = 1 + size;
        let mut data = self.data.borrow_mut();
        data.reserve(size_with_header);
        let start = data.len();
        for _ in 0..size_with_header {
            data.push(OpaqueValue::int(0));
        }
        let header = Header::new(size, tag);
        // SAFETY: We need to ensure this Header is never observed as a Value.
        // That means this module must never expose our backing Vec (including
        // after it is wrapped by OwnedSlab).
        data[start] = unsafe { OpaqueValue::from_bits(header.to_bits()) };
        BlockBuilder::new((start + 1) * WORD_SIZE, size)
    }
}

impl Allocator for SlabAllocator {
    #[inline(always)]
    fn generation(&self) -> usize {
        self.generation
    }

    fn block_with_size_and_tag(&self, size: usize, tag: u8) -> BlockBuilder<'_> {
        self.alloc(size, tag)
    }

    #[inline(always)]
    fn set_field<'a>(&self, block: &mut BlockBuilder<'a>, index: usize, value: OpaqueValue<'a>) {
        assert!(index < block.size());
        // SAFETY: `alloc` ensures that the block has `block.size()` fields. The
        // above assertion ensures that we aren't out-of-bounds.
        unsafe { *self.block_ptr_mut(block).add(index) = value }
    }

    unsafe fn block_ptr_mut<'a>(&self, block: &mut BlockBuilder<'a>) -> *mut OpaqueValue<'a> {
        let mut data = self.data.borrow_mut();
        // Interpret the block address as a byte offset into our `data` vec.
        let first_field = &mut data[block.address() / WORD_SIZE];
        std::mem::transmute::<*mut OpaqueValue<'static>, *mut OpaqueValue<'a>>(
            first_field as *mut _,
        )
    }

    fn memoized<'a>(
        &'a self,
        ptr: usize,
        size: usize,
        f: impl FnOnce(&'a Self) -> OpaqueValue<'a>,
    ) -> OpaqueValue<'a> {
        let bits = self.cache.memoized(ptr, size, || f(self).to_bits());
        // SAFETY: The only memoized values in the cache are those computed in
        // the closure on the previous line. Since f returns OpaqueValue<'a>, any
        // cached bits must represent a valid OpaqueValue<'a>,
        unsafe { OpaqueValue::from_bits(bits) }
    }

    fn add_root<'a, T: ToOcamlRep + ?Sized>(&'a self, value: &'a T) -> OpaqueValue<'a> {
        self.cache.with_cache(|| value.to_ocamlrep(self))
    }
}

fn with_slab_allocator(f: impl Fn(&SlabAllocator) -> OpaqueValue<'_>) -> Option<OwnedSlab> {
    let alloc = SlabAllocator::new();
    let root_value_byte_offset = {
        let value = f(&alloc);
        if value.is_immediate() {
            return None;
        }
        value.to_bits()
    };
    let mut data = alloc.data.into_inner();
    // Add an extra word of padding, so that we have enough room to realign the
    // slab contents if it's copied (as a byte string) to an address which isn't
    // aligned to WORD_SIZE.
    data.push(unsafe { OpaqueValue::from_bits(0) });
    let mut slab = data.into_boxed_slice();
    slab.set_root_value_offset(root_value_byte_offset / WORD_SIZE);
    slab.mark_initialized();
    // SlabAllocator starts its offsets at zero, so there is no need to rebase
    // to 0 to satisfy OwnedSlab's invariant.
    Some(OwnedSlab(slab))
}

pub fn to_slab<T: ToOcamlRep>(value: &T) -> Option<OwnedSlab> {
    // SAFETY: I'm not sure how else to express that the borrow of `T` we have
    // in `value` is sure to outlive the `SlabAllocator` we borrow in our
    // closure here, so transmute away the lifetime.
    with_slab_allocator(|alloc| unsafe {
        std::mem::transmute::<OpaqueValue<'_>, OpaqueValue<'_>>(alloc.add_root(value))
    })
}

/// Copy the slab stored in `src` into `dest`, then fix up the slab's internal
/// pointers in `dest`.
///
/// Padding bytes in `dest` (bytes before the first or after the last aligned
/// word) will not be modified.
///
/// Returns `Err` if `src` does not contain a valid slab.
///
/// # Panics
///
/// This function will panic if `src` and `dest` have different lengths.
pub fn copy_slab(src: &[u8], dest: &mut UninitSlice) -> Result<(), SlabIntegrityError> {
    let dest_addr = dest.as_mut_ptr() as usize;

    // SAFETY: leading_padding does not write from the pointer
    let new_base = dest_addr + leading_padding(dest.as_mut_ptr(), dest.len());

    let src_slab = Slab::from_bytes(src);

    // SAFETY: UninitSlice expects us to not write unitialized Bytes, or use this to read. This
    // code actually does read, but only after having initialized the memory by copying into the
    // UninitSlice.
    let dest_slab = unsafe { Slab::from_uninit_slice_mut(dest) };

    src_slab.check_initialized()?;

    // memcpy `src_slab` into `dest_slab`. Panic if they differ in length.
    dest_slab.copy_from_slice(src_slab);

    // Safety: we checked that `src_slab` is a valid slab, and slabs remain
    // valid after a memcpy (i.e., slabs needing a rebase are still valid).
    unsafe {
        dest_slab.rebase_to(new_base);
    }

    Ok(())
}

/// Copy the slab stored in `src` into `dest`, then fix up the slab's internal
/// pointers in `dest`. Return a `Value` referencing the slab root value in
/// `dest`.
///
/// Returns `Err` if `src` does not contain a valid slab.
///
/// # Panics
///
/// This function will panic unless `dest` has length
/// `src.value_size_in_words()`.
pub fn copy_and_rebase_value<'a>(src: SlabReader<'_>, dest: &'a mut [usize]) -> Value<'a> {
    let src_slab = Slab::from_bytes(src.0);

    // Safety: OpaqueValue has the same size and alignment as usize
    let dest_slab = unsafe {
        std::slice::from_raw_parts_mut(dest.as_mut_ptr() as *mut OpaqueValue<'a>, dest.len())
    };

    // memcpy `src_slab` into `dest_slab`. Panic if they differ in length.
    dest_slab.copy_from_slice(&src_slab[SLAB_METADATA_WORDS..]);

    // Safety: we checked that `src_slab` is a valid slab, and slabs remain
    // valid after a memcpy (i.e., slabs needing a rebase are still valid).
    unsafe {
        let diff = dest_slab.as_ptr() as isize
            - src_slab.base() as isize
            - (SLAB_METADATA_WORDS * WORD_SIZE) as isize;
        rebase_slab_value(dest_slab, diff);
    }

    // Safety: We just rebased dest_slab to its current address, so its root
    // value is a valid Value. We didn't copy the metadata, so there's no
    // root_value_offset for us to read--we need to use the one in `src_slab`.
    // The Value will mutably borrow `dest`, so it won't be possible for the
    // caller to mutate `dest`'s contents while our Value exists.
    unsafe {
        Value::from_bits(
            dest_slab
                .as_ptr()
                .add(src_slab.root_value_offset() - SLAB_METADATA_WORDS) as usize,
        )
    }
}

/// A contiguous memory region containing a tree of OCaml values.
#[derive(Clone, serde::Serialize, serde::Deserialize)]
pub struct OwnedSlab(
    // This 'static lifetime is a lie. Slab's lifetime parameter represents the
    // lifetime of the OpaqueValues inside it. Since these values are expected
    // only to reference other values within the same slab, the lifetime of the
    // values is the same as the lifetime of the backing memory for the slab.
    // When handing out references to values inside this slab, we must take care
    // that they borrow `self`, so that they don't outlive this backing memory.
    //
    // Rebased to address 0, so each "pointer" value in the slab represents an
    // offset from the slab's start address.
    #[serde(serialize_with = "serialize_slab")]
    #[serde(deserialize_with = "deserialize_slab")]
    Box<Slab<'static>>,
);

impl OwnedSlab {
    pub fn from_value(value: Value<'_>) -> Option<Self> {
        with_slab_allocator(|alloc| value.clone_with_allocator(alloc))
    }

    pub unsafe fn from_ocaml(value: usize) -> Option<Self> {
        Self::from_value(Value::from_bits(value))
    }

    pub fn size_in_bytes(&self) -> usize {
        self.0.len() * WORD_SIZE
    }

    pub fn as_bytes(&self) -> &[u8] {
        let ptr = self.0.as_ptr() as *const u8;
        // SAFETY: should be safe to view the underlying memory as bytes
        // (although the exact behavior will be dependent on platform
        // endianness)
        unsafe { std::slice::from_raw_parts(ptr, self.size_in_bytes()) }
    }

    pub fn as_reader(&self) -> SlabReader<'_> {
        // SAFETY: `self.0` is a valid Slab
        unsafe { SlabReader::from_slab(&self.0).unwrap() }
    }

    #[cfg(test)]
    fn from_slice(slice: &[usize]) -> Result<Self, SlabIntegrityError> {
        Ok(Self(slab_from_words(slice.into())?))
    }

    pub fn rebase(mut self) -> RebasedSlab {
        // SAFETY: `self.0` is a valid Slab
        unsafe { self.0.rebase_to(self.0.current_address()) };
        RebasedSlab(self.0)
    }

    pub fn leak(self) -> Value<'static> {
        let slab = self.rebase();
        Box::leak(slab.0).value().unwrap()
    }
}

impl Debug for OwnedSlab {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        debug_slab("OwnedSlab", &self.0, f)
    }
}

fn serialize_slab<S: serde::Serializer>(
    slab: &Slab<'static>,
    serializer: S,
) -> Result<S::Ok, S::Error> {
    // SAFETY: `usize` and `OpaqueValue` have the same size and alignment
    let slice: &[usize] = unsafe { std::slice::from_raw_parts(slab.as_ptr().cast(), slab.len()) };
    serde::Serialize::serialize(slice, serializer)
}

fn deserialize_slab<'de, D: serde::Deserializer<'de>>(
    deserializer: D,
) -> Result<Box<Slab<'static>>, D::Error> {
    let words: Box<[usize]> = serde::Deserialize::deserialize(deserializer)?;
    let slab = slab_from_words(words)
        .map_err(|e| serde::de::Error::custom(format!("invalid slab: {}", e)))?;
    Ok(slab)
}

pub unsafe fn buf_into_value<'a>(buf: &'a mut [u8]) -> Option<Value<'a>> {
    let slab = Slab::from_bytes_mut(buf);
    slab.rebase_to(slab.current_address());
    slab.value()
}

fn slab_from_words(words: Box<[usize]>) -> Result<Box<Slab<'static>>, SlabIntegrityError> {
    let mut slab = unsafe { std::mem::transmute::<Box<[usize]>, Box<Slab<'static>>>(words) };
    // Do a (relatively expensive) integrity check, since we have no
    // guarantee that the `Box<[usize]>` is a valid slab (since this
    // function is used in deserialization).
    slab.check_integrity()?;
    if slab.base() != 0 {
        unsafe {
            slab.rebase_to(0);
        }
    }
    Ok(slab)
}

/// A contiguous memory region containing a tree of OCaml values.
/// Rebased to its current address, allowing reads of the contained value.
#[derive(Clone)]
pub struct RebasedSlab(Box<Slab<'static>>);

impl RebasedSlab {
    pub fn value(&self) -> Value<'_> {
        // Invariant: self.0 is always rebased to its current address.
        self.as_reader().value().unwrap()
    }

    fn as_reader(&self) -> SlabReader<'_> {
        // SAFETY: `self.0` is a valid Slab
        unsafe { SlabReader::from_slab(&self.0).unwrap() }
    }
}

/// A contiguous memory region containing a tree of OCaml values.
#[derive(Copy, Clone)]
pub struct SlabReader<'a>(&'a [u8]);

impl<'a> SlabReader<'a> {
    /// Return a SlabReader for the slab embedded in the given byte slice.
    ///
    /// # Safety
    ///
    /// The caller must only invoke this function on byte slices which were
    /// initialized by slab APIs (e.g., `OwnedSlab::as_bytes`, `copy_slab`).
    pub unsafe fn from_bytes(bytes: &'a [u8]) -> Result<Self, SlabIntegrityError> {
        let slab = Slab::from_bytes(bytes);
        slab.check_initialized()?;
        Ok(SlabReader(bytes))
    }

    /// Return a SlabReader for the given slice.
    ///
    /// # Safety
    ///
    /// The caller must only invoke this function on slices which were
    /// initialized by slab APIs (e.g., `OwnedSlab::as_slice`).
    pub unsafe fn from_words(words: &'a [usize]) -> Result<Self, SlabIntegrityError> {
        let slab =
            std::slice::from_raw_parts(words.as_ptr() as *const OpaqueValue<'a>, words.len());
        slab.check_initialized()?;
        Ok(SlabReader(std::slice::from_raw_parts(
            words.as_ptr() as *const u8,
            words.len() * WORD_SIZE,
        )))
    }

    /// Safety: `slab` must be a valid slab
    unsafe fn from_slab(slab: &'a Slab<'static>) -> Result<Self, SlabIntegrityError> {
        let words: &[usize] = std::slice::from_raw_parts(slab.as_ptr().cast(), slab.len());
        Self::from_words(words)
    }

    pub fn size_in_words(&self) -> usize {
        Slab::from_bytes(self.0).len()
    }

    pub fn size_in_bytes(&self) -> usize {
        self.size_in_words() * WORD_SIZE
    }

    pub fn value_size_in_words(&self) -> usize {
        Slab::from_bytes(self.0).len() - SLAB_METADATA_WORDS
    }

    pub fn as_bytes(&self) -> &[u8] {
        let ptr = Slab::from_bytes(self.0).as_ptr() as *const u8;
        unsafe { std::slice::from_raw_parts(ptr, self.size_in_bytes()) }
    }

    pub fn as_slice(&self) -> &[usize] {
        let slab = Slab::from_bytes(self.0);
        // SAFETY: `usize` has the same size and alignment as `OpaqueValue`. The
        // returned slice borrows `self`, so the associated memory will not be
        // mutated while the slice exists.
        unsafe { std::slice::from_raw_parts(slab.as_ptr() as *const usize, slab.len()) }
    }

    pub fn value(&self) -> Option<Value<'a>> {
        Slab::from_bytes(self.0).value()
    }

    pub fn value_offset_in_bytes(&self) -> usize {
        leading_padding(self.0.as_ptr(), self.0.len())
            + Slab::from_bytes(self.0).root_value_offset() * WORD_SIZE
    }
}

impl Debug for SlabReader<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        debug_slab("SlabReader", Slab::from_bytes(self.0), f)
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::FromOcamlRep;

    pub const MIN_SIZE_IN_BYTES: usize = (SLAB_METADATA_WORDS + 2) * WORD_SIZE;

    // Some test cases use a slab containing the tuple (42, "a").
    // When rebased to 0x00, the slab we write for this value has this layout:
    //
    // | address | value                         | comment                    |
    // | ------- | ----------------------------- | -------------------------- |
    // | 0x00    | 0x00                          | base pointer               |
    // | 0x08    | 0x04                          | root value offset          |
    // | 0x10    | 0x51a851a8                    | magic number               |
    // | 0x18    | Header { size: 2, tag: 0 }    | tuple tag is 0             |
    // | 0x20    | 0x85                          | ocaml representation of 42 |
    // | 0x28    | 0x38                          | pointer to "a"             |
    // | 0x30    | Header { size: 1, tag: 252 }, | STRING_TAG = 252           |
    // | 0x38    | 0x0600000000000061,           | "a" (0x06 = padding byte)  |

    pub const TUPLE_42_A_SIZE_IN_WORDS: usize = SLAB_METADATA_WORDS + 5;
    pub const TUPLE_42_A_SIZE_IN_BYTES: usize = TUPLE_42_A_SIZE_IN_WORDS * WORD_SIZE;

    pub fn alloc_tuple_42_a() -> OwnedSlab {
        to_slab(&(42, "a".to_string())).unwrap()
    }

    pub fn write_tuple_42_a(slab: &mut Slab<'_>) {
        let tuple_slab = alloc_tuple_42_a();
        // Copy everything except the last word, which is an empty padding word
        // which provides space for the slab to be realigned when embedded in a
        // byte slice.
        slab.copy_from_slice(&tuple_slab.0[..tuple_slab.0.len() - 1]);
        unsafe { slab.rebase_to(slab.current_address()) };
    }

    #[test]
    fn copy_and_rebase_val() {
        let mut tuple_slab = vec![0usize; TUPLE_42_A_SIZE_IN_WORDS];
        let mut tuple_val = vec![0usize; TUPLE_42_A_SIZE_IN_WORDS - SLAB_METADATA_WORDS];
        let value = unsafe {
            write_tuple_42_a(std::slice::from_raw_parts_mut(
                tuple_slab.as_mut_ptr() as *mut _,
                tuple_slab.len(),
            ));
            let tuple_slab = SlabReader::from_words(tuple_slab.as_slice()).unwrap();
            copy_and_rebase_value(tuple_slab, tuple_val.as_mut_slice())
        };
        assert_eq!(
            <(isize, String)>::from_ocamlrep(value),
            Ok((42, "a".to_string()))
        );
    }

    #[test]
    fn to_and_from_slice() {
        let tuple_slab = alloc_tuple_42_a();
        let tuple_reader = tuple_slab.as_reader();
        let mut bytes = vec![0u8; tuple_reader.size_in_bytes() + WORD_SIZE];
        // Iterate over all possible alignments for our byte slice, embed the
        // slab in the slice (with appropriate padding), and attempt to convert
        // it to a list of usizes and back to an OwnedSlab.
        for offset in 0..WORD_SIZE {
            let bytes = &mut bytes[offset..offset + tuple_reader.size_in_bytes()];
            let slice =
                unsafe { UninitSlice::from_raw_parts_mut(bytes.as_ptr() as *mut _, bytes.len()) };
            copy_slab(tuple_reader.as_bytes(), slice).unwrap();
            unsafe {
                let reader = SlabReader::from_bytes(bytes).unwrap();
                let owned_slab = OwnedSlab::from_slice(reader.as_slice()).unwrap();
                assert_eq!(
                    <(isize, String)>::from_ocamlrep(owned_slab.rebase().value()),
                    Ok((42, "a".to_string()))
                );
            }
        }
    }
}

#[cfg(test)]
mod test_integrity_check {
    use SlabIntegrityError::*;

    use super::test::*;
    use super::*;

    #[test]
    fn bad_size() {
        for size in 0..MIN_SIZE_IN_BYTES {
            let bytes = vec![0u8; size];
            let result = Slab::from_bytes(&bytes).check_integrity();
            assert_eq!(result, Err(TooSmall(size / WORD_SIZE)));
        }
    }

    #[test]
    fn not_initialized() {
        let bytes = vec![0u8; MIN_SIZE_IN_BYTES];
        let result = Slab::from_bytes(&bytes).check_integrity();
        assert_eq!(result, Err(NotInitialized));
    }

    #[test]
    fn bad_root_value() {
        let mut bytes = vec![0u8; TUPLE_42_A_SIZE_IN_BYTES];
        let slab = Slab::from_bytes_mut(&mut bytes);
        write_tuple_42_a(slab);
        let tuple_offset = slab.root_value_offset();

        for offset in 0..=TUPLE_42_A_SIZE_IN_WORDS {
            slab.set_root_value_offset(offset);
            // The only block values in the slab are the tuple and the string,
            // which is allocated 3 words after the tuple.
            let valid_offsets = vec![tuple_offset, tuple_offset + 3];
            if valid_offsets.iter().any(|&o| o == offset) {
                assert_eq!(slab.check_integrity(), Ok(()));
            } else {
                assert_eq!(slab.check_integrity(), Err(InvalidRootValueOffset(offset)));
            }
        }
    }

    #[test]
    fn bad_base_ptr() {
        let mut bytes = vec![0u8; TUPLE_42_A_SIZE_IN_BYTES];
        let slab = Slab::from_bytes_mut(&mut bytes);
        write_tuple_42_a(slab);
        assert!(slab.check_integrity().is_ok());

        let original_base = slab.base();
        slab.set_base(0);
        assert!(slab.check_integrity().is_err());
        slab.set_base(original_base);

        unsafe { slab.rebase_to(0) };
        assert_eq!(slab.base(), 0);
        assert!(slab.check_integrity().is_ok());

        slab.set_base(1);
        assert!(slab.check_integrity().is_err());

        slab.set_base(8);
        assert!(slab.check_integrity().is_err());
    }
}
