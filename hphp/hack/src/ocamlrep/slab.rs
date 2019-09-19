// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::fmt::{self, Debug};
use std::mem;

use crate::block::NO_SCAN_TAG;
use crate::{OpaqueValue, Value};

// The first three words in a slab are for the base pointer, the offset (in
// words) of the root value from that base, and the magic number.
const SLAB_METADATA_WORDS: usize = 3;

// This secret number marks properly initialized slabs.
const SLAB_MAGIC_NUMBER: usize = 0x51A851A8;

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
    fn new(value_size_in_words: usize) -> Box<Self>;

    // metadata accessors
    fn base(&self) -> usize;
    fn root_value_offset(&self) -> usize;
    fn is_initialized(&self) -> bool;

    // metadata setters
    fn set_base(&mut self, base: usize);
    fn set_root_value_offset(&mut self, offset: usize);
    fn mark_initialized(&mut self);

    fn current_address(&self) -> usize;
    fn rebase(&mut self);
    fn rebase_and_get_value(&mut self) -> Value;
    fn value(&self) -> Option<Value>;
    unsafe fn rebase_to(&mut self, new_base: usize);
}

impl<'a> SlabTrait for Slab<'a> {
    fn new(value_size_in_words: usize) -> Box<Self> {
        let size = SLAB_METADATA_WORDS + value_size_in_words;
        vec![unsafe { OpaqueValue::from_bits(0) }; size].into_boxed_slice()
    }

    fn base(&self) -> usize {
        unsafe { self[0].to_bits() }
    }

    fn root_value_offset(&self) -> usize {
        unsafe { self[1].to_bits() }
    }

    fn is_initialized(&self) -> bool {
        unsafe { self[2].to_bits() == SLAB_MAGIC_NUMBER }
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

    fn rebase(&mut self) {
        unsafe { self.rebase_to(self.current_address()) }
    }

    fn rebase_and_get_value(&mut self) -> Value {
        if self.base() != self.current_address() {
            self.rebase();
        }
        self.value().unwrap()
    }

    fn value(&self) -> Option<Value> {
        if !self.is_initialized() {
            panic!("slab not initialized");
        }
        if self.base() != self.current_address() {
            return None;
        }
        unsafe {
            Some(Value::from_bits(
                &self[self.root_value_offset()] as *const _ as usize,
            ))
        }
    }

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

    let needs_rebase = slab.base() != slab.current_address();
    let word_size = mem::size_of::<OpaqueValue>();
    let mut blocks = vec![];
    let mut i = SLAB_METADATA_WORDS;
    while i < slab.len() {
        let offset = (i + 1) * word_size;
        let offset_or_address = if needs_rebase {
            offset
        } else {
            slab.current_address() + offset
        };
        let size = slab[i].as_header().size() + 1;
        blocks.push(DebugBlock(&slab[i..(i + size)], offset_or_address));
        i += size;
    }
    let root_value = slab.base() + slab.root_value_offset() * word_size;
    f.debug_struct(name)
        .field("base", &DebugPtr(slab.base()))
        .field("current_address", &DebugPtr(slab.current_address()))
        .field("root_value", &DebugPtr(root_value))
        .field("blocks", &blocks)
        .finish()
}

struct SlabBuilder<'slab: 'builder, 'builder> {
    slab: &'builder mut Slab<'slab>,
    next_block_index: RefCell<usize>,
}

impl<'s, 'b> SlabBuilder<'s, 'b> {
    fn new(slab: &'b mut Slab<'s>) -> Self {
        slab.set_base(slab.current_address());
        SlabBuilder {
            slab,
            next_block_index: RefCell::new(SLAB_METADATA_WORDS),
        }
    }

    fn build_from_value<'a>(slab: &'a mut Slab<'s>, value: Value<'_>) {
        let mut builder = SlabBuilder::new(slab);
        unsafe { builder.copy_value(value) };
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
        let block = value.as_block().unwrap();
        let header = block.header();
        let size = header.size();
        let dest = self.alloc(size + 1);
        // A tag >= NO_SCAN_TAG indicates that the block contains binary data
        // rather than pointers or immediate integers.
        if header.tag() >= NO_SCAN_TAG {
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
        }
    }
}

fn words_reachable(value: Value<'_>) -> usize {
    let block = match value.as_block() {
        None => return 0,
        Some(b) => b,
    };
    let size = block.size();
    let mut words = size + 1;
    if block.tag() < NO_SCAN_TAG {
        for i in 0..size {
            words += words_reachable(block[i]);
        }
    }
    words
}

pub struct OwnedSlab(Box<Slab<'static>>);

impl OwnedSlab {
    pub fn from_value(value: Value<'_>) -> Option<Self> {
        if value.is_immediate() {
            return None;
        }
        let mut slab = Slab::new(words_reachable(value));
        SlabBuilder::build_from_value(&mut slab, value);
        Some(OwnedSlab(slab))
    }

    pub unsafe fn from_ocaml(value: usize) -> Option<Self> {
        Self::from_value(Value::from_bits(value))
    }

    pub fn value<'a>(&'a self) -> Value<'a> {
        // The boxed slice can't be moved, so we should never need to rebase.
        self.0.value().unwrap()
    }
}

impl Debug for OwnedSlab {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        debug_slab("OwnedSlab", &self.0, f)
    }
}
