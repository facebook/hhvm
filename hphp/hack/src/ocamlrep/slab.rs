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
use std::collections::{HashMap, HashSet};
use std::fmt::{self, Debug};
use std::mem::{self, MaybeUninit};

use crate::block::NO_SCAN_TAG;
use crate::{OpaqueValue, SlabIntegrityError, Value};

// The first three words in a slab are for the base pointer, the offset (in
// words) of the root value from that base, and the magic number.
const SLAB_METADATA_WORDS: usize = 3;

// This secret number marks properly initialized slabs.
const SLAB_MAGIC_NUMBER: usize = 0x51A851A8;

const WORD_SIZE: usize = mem::size_of::<OpaqueValue>();

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
// Example (for the value `Some "a"`, rebased to address 0x00):
//
// | address | value                         | comment                      |
// | ------- | ----------------------------- | ---------------------------- |
// | 0x00    | 0x00                          | base pointer                 |
// | 0x08    | 4                             | root value offset            |
// | 0x10    | 0x51A851A8                    | magic number                 |
// | 0x18    | Header { size: 1, tag: 0 }    | Some's tag is 0              |
// | 0x20    | 0x30                          | pointer (base ptr + 6 words) |
// | 0x28    | Header { size: 1, tag: 252 }, | STRING_TAG = 252             |
// | 0x30    | 0x0600000000000061,           | "a" (0x06 = padding byte)    |
type Slab<'a> = [OpaqueValue<'a>];

// Private convenience methods for Slab (since Slab does not have a fixed size,
// we cannot put it in a wrapper struct and define these methods on that struct,
// but we can define trait methods instead).
trait SlabTrait {
    fn from_bytes(bytes: &[u8]) -> &Self;
    fn from_bytes_mut(bytes: &mut [MaybeUninit<u8>]) -> &mut Self;

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
    fn value(&self) -> Option<Value>;
    unsafe fn rebase_to(&mut self, new_base: usize);
    fn check_integrity(&self) -> Result<(), SlabIntegrityError>;
}

// When embedding a Slab in a byte string, we need to include leading padding
// bytes so that the Slab is word-aligned.
#[inline]
fn leading_padding(bytes: &[impl Byte]) -> usize {
    let misalignment = bytes.as_ptr() as usize % WORD_SIZE;
    let padding = (WORD_SIZE - misalignment) % WORD_SIZE;
    std::cmp::min(padding, bytes.len())
}

trait Byte {}
impl Byte for u8 {}
impl Byte for MaybeUninit<u8> {}

impl<'a> SlabTrait for Slab<'a> {
    fn from_bytes(bytes: &[u8]) -> &Self {
        let padding = leading_padding(bytes);
        let ptr = bytes[padding..].as_ptr() as *const OpaqueValue<'a>;
        unsafe { std::slice::from_raw_parts(ptr, bytes.len() / WORD_SIZE) }
    }

    fn from_bytes_mut(bytes: &mut [MaybeUninit<u8>]) -> &mut Self {
        let padding = leading_padding(bytes);
        let ptr = bytes[padding..].as_mut_ptr() as *mut OpaqueValue<'a>;
        unsafe { std::slice::from_raw_parts_mut(ptr, bytes.len() / WORD_SIZE) }
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
    fn value(&self) -> Option<Value> {
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
        let ptr = self.as_mut_ptr();
        let end = ptr.add(self.len());
        let mut ptr = ptr.add(SLAB_METADATA_WORDS);
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
        self.set_base(new_base);
    }

    fn check_integrity(&self) -> Result<(), SlabIntegrityError> {
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

fn debug_slab(name: &'static str, slab: &Slab<'_>, f: &mut fmt::Formatter) -> fmt::Result {
    struct DebugBlock<'a>(&'a [OpaqueValue<'a>], usize);
    impl Debug for DebugBlock<'_> {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
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
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
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

struct SlabBuilder<'slab: 'builder, 'builder> {
    slab: &'builder mut Slab<'slab>,
    seen: HashMap<usize, OpaqueValue<'slab>>,
    next_block_index: RefCell<usize>,
}

impl<'s, 'b> SlabBuilder<'s, 'b> {
    fn new(slab: &'b mut Slab<'s>) -> Self {
        slab.set_base(slab.current_address());
        SlabBuilder {
            slab,
            seen: HashMap::new(),
            next_block_index: RefCell::new(SLAB_METADATA_WORDS),
        }
    }

    // Unsafe because of the precondition that the slab has precisely the size
    // SLAB_METADATA_WORDS + words_reachable(value). Failure to meet this
    // precondition results in undefined behavior.
    unsafe fn build_from_value<'a>(slab: &'a mut Slab<'s>, value: Value<'_>) {
        SlabBuilder::new(slab).copy_value(value);
        // copy_value begins by allocating the root value, so it will be located
        // at the beginning of the block data region. Add one for the header.
        slab.set_root_value_offset(SLAB_METADATA_WORDS + 1);
        slab.mark_initialized();
    }

    fn alloc<'a>(&'a mut self, size: usize) -> *mut OpaqueValue<'s> {
        let start = *self.next_block_index.borrow();
        let end = start + size;
        *self.next_block_index.borrow_mut() = end;
        let slice = &mut self.slab[start..end];
        slice.as_mut_ptr()
    }

    unsafe fn copy_value<'a>(&'a mut self, value: Value<'_>) -> OpaqueValue<'s> {
        if value.is_immediate() {
            return OpaqueValue::from_bits(value.to_bits());
        }
        if let Some(copied_value) = self.seen.get(&value.to_bits()) {
            return *copied_value;
        }
        let block = value.as_block().unwrap();
        let header = block.header();
        let size = header.size();
        let dest = self.alloc(size + 1);
        // A tag >= NO_SCAN_TAG indicates that the block contains binary data
        // rather than pointers or immediate integers.
        let copied_value = if header.tag() >= NO_SCAN_TAG {
            // Copy header and binary data
            let src: *const OpaqueValue = mem::transmute(block.header_ptr());
            std::ptr::copy_nonoverlapping(src, dest, size + 1);
            OpaqueValue::from_bits(dest.add(1) as usize)
        } else {
            // Copy header
            *dest = OpaqueValue::from_bits(header.to_bits());
            // Copy fields
            for i in 0..size {
                *dest.add(i + 1) = self.copy_value(block[i]);
            }
            OpaqueValue::from_bits(dest.add(1) as usize)
        };
        self.seen.insert(value.to_bits(), copied_value);
        copied_value
    }
}

fn words_reachable<'a>(seen: &mut HashSet<Value<'a>>, value: Value<'a>) -> usize {
    let block = match value.as_block() {
        None => return 0,
        Some(b) => b,
    };
    if seen.contains(&value) {
        return 0;
    }
    seen.insert(value);
    let size = block.size();
    let mut words = size + 1;
    if block.tag() < NO_SCAN_TAG {
        for i in 0..size {
            words += words_reachable(seen, block[i]);
        }
    }
    words
}

/// Copy the slab stored in `src` into `dest`, then fix up the slab's internal
/// pointers in `dest`.
///
/// Pointers in `dest` will be rebased with respect to `dest_addr`. This is so
/// that a process which copies a slab into shared memory may rebase the slab
/// for reading in another process, which maps that shared memory segment at a
/// different address. `dest_addr` should be the address of the first byte in
/// `dest` in that reader process.
///
/// When copying a slab for reading within the same process, use the value
/// `dest.as_ptr() as usize` for `dest_addr`.
///
/// Padding bytes in `dest` (bytes before the first or after the last aligned
/// word) will not be modified.
///
/// Returns `Err` if `src` does not contain a valid slab.
///
/// # Panics
///
/// This function will panic if `src` and `dest` have different lengths.
pub fn copy_slab(
    src: &[u8],
    dest: &mut [MaybeUninit<u8>],
    dest_addr: usize,
) -> Result<(), SlabIntegrityError> {
    let new_base = dest_addr + leading_padding(dest);

    let src_slab = Slab::from_bytes(src);
    let dest_slab = Slab::from_bytes_mut(dest);

    // TODO: Do a less expensive sanity check once we're confident in correctness
    src_slab.check_integrity()?;

    // memcpy `src_slab` into `dest_slab`. Panic if they differ in length.
    dest_slab.copy_from_slice(src_slab);

    // Safety: we checked that `src_slab` is a valid slab, and slabs remain
    // valid after a memcpy (i.e., slabs needing a rebase are still valid).
    unsafe {
        dest_slab.rebase_to(new_base);
    }

    Ok(())
}

/// A contiguous memory region containing a tree of OCaml values.
pub struct OwnedSlab(Vec<u8>);

impl OwnedSlab {
    pub fn from_value(value: Value<'_>) -> Option<Self> {
        if value.is_immediate() {
            return None;
        }
        let value_size_in_words = words_reachable(&mut HashSet::new(), value);
        let size_in_words = SLAB_METADATA_WORDS + value_size_in_words;
        // This padding lets us move the slab around within the byte string in
        // order to align it properly, since cachelib does not provide any
        // alignment guarantees. It isn't really necessary for an OwnedSlab, but
        // it makes life easier until we have a Vec-based Arena.
        let padding = WORD_SIZE - 1;
        let size_in_bytes = size_in_words * WORD_SIZE + padding;
        let mut vec = vec![MaybeUninit::new(0u8); size_in_bytes];
        let slab = Slab::from_bytes_mut(&mut vec);
        unsafe { SlabBuilder::build_from_value(slab, value) };
        // Slab has been initialized.
        let vec = unsafe { mem::transmute::<Vec<MaybeUninit<u8>>, Vec<u8>>(vec) };
        Some(OwnedSlab(vec))
    }

    pub unsafe fn from_ocaml(value: usize) -> Option<Self> {
        Self::from_value(Value::from_bits(value))
    }

    pub fn value<'a>(&'a self) -> Value<'a> {
        // The contents of our Vec won't be moved while we own it (since we
        // don't modify it), so we should never need to rebase.
        Slab::from_bytes(&self.0).value().unwrap()
    }

    pub fn into_vec(self) -> Vec<u8> {
        self.0
    }

    pub fn leak(self) -> Value<'static> {
        Slab::from_bytes(Box::leak(self.0.into_boxed_slice()))
            .value()
            .unwrap()
    }
}

impl Debug for OwnedSlab {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        debug_slab("OwnedSlab", Slab::from_bytes(&self.0), f)
    }
}

/// A contiguous memory region containing a tree of OCaml values.
pub struct SlabReader<'a>(&'a [u8]);

impl<'a> SlabReader<'a> {
    /// Return a SlabReader for the slab embedded in the given byte slice.
    ///
    /// # Safety
    ///
    /// The caller must only invoke this function on byte slices which were
    /// initialized by slab APIs (e.g., `OwnedSlab::to_vec`, `copy_slab`).
    pub unsafe fn from_bytes(bytes: &'a [u8]) -> Result<Self, SlabIntegrityError> {
        // TODO: Do a less expensive sanity check once we're confident in correctness
        Slab::from_bytes(bytes).check_integrity()?;
        Ok(SlabReader(bytes))
    }

    pub fn value(&self) -> Option<Value> {
        Slab::from_bytes(self.0).value()
    }

    pub fn value_offset_in_bytes(&self) -> usize {
        leading_padding(self.0) + Slab::from_bytes(self.0).root_value_offset() * WORD_SIZE
    }
}

impl Debug for SlabReader<'_> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        debug_slab("SlabReader", &Slab::from_bytes(self.0), f)
    }
}

#[cfg(test)]
mod test_integrity_check {
    use super::*;
    use crate::Arena;
    use SlabIntegrityError::*;

    const MIN_SIZE_IN_BYTES: usize = (SLAB_METADATA_WORDS + 2) * WORD_SIZE;

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

    const TUPLE_42_A_SIZE_IN_WORDS: usize = SLAB_METADATA_WORDS + 5;
    const TUPLE_42_A_SIZE_IN_BYTES: usize = TUPLE_42_A_SIZE_IN_WORDS * WORD_SIZE;

    fn write_tuple_42_a(mut slab: &mut Slab) {
        let arena = Arena::new();
        let value = arena.add(&(42, "a".to_string()));
        unsafe { SlabBuilder::build_from_value(&mut slab, value) };
    }

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
        let mut bytes = vec![MaybeUninit::new(0u8); TUPLE_42_A_SIZE_IN_BYTES];
        let mut slab = Slab::from_bytes_mut(&mut bytes);
        write_tuple_42_a(&mut slab);
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
        let mut bytes = vec![MaybeUninit::new(0u8); TUPLE_42_A_SIZE_IN_BYTES];
        let mut slab = Slab::from_bytes_mut(&mut bytes);
        write_tuple_42_a(&mut slab);
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
