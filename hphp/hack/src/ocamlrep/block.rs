// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::collections::HashMap;
use std::fmt::{self, Debug};
use std::ops::Index;

use crate::{Allocator, Value};

/// Blocks with tags greater than or equal to NO_SCAN_TAG contain binary data,
/// and are not scanned by the garbage collector. Likewise, we must avoid
/// interpreting the fields of blocks with such tags as Values.
pub const NO_SCAN_TAG: u8 = 251;
pub const STRING_TAG: u8 = 252;
pub const DOUBLE_TAG: u8 = 253;

/// A recently-allocated, not-yet-finalized Block.
#[repr(transparent)]
pub struct BlockBuilder<'a>(pub(crate) &'a mut [Value<'a>]);

impl<'a> BlockBuilder<'a> {
    #[inline(always)]
    pub fn new(block: &'a mut [Value<'a>]) -> Self {
        if block.len() == 0 {
            panic!()
        }
        BlockBuilder(block)
    }

    /// The number of fields in this block.
    #[inline(always)]
    pub fn size(&self) -> usize {
        self.0.len()
    }

    #[inline(always)]
    pub fn build(self) -> Value<'a> {
        unsafe { Value::from_bits(self.0.as_ptr() as usize) }
    }

    /// Return a pointer to the first field in the block.
    #[inline(always)]
    pub fn as_mut_ptr(&mut self) -> *mut Value<'a> {
        self.0.as_mut_ptr()
    }
}

/// The contents of an OCaml block, consisting of a header and one or more
/// fields of type [`Value`](struct.Value.html).
#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Block<'arena>(pub(crate) &'arena [Value<'arena>]);

impl<'a> Block<'a> {
    pub(crate) fn header(&self) -> Header {
        Header(self.0[0].0)
    }

    pub fn size(&self) -> usize {
        self.header().size()
    }

    pub fn tag(&self) -> u8 {
        self.header().tag()
    }

    pub fn as_str(&self) -> Option<Cow<str>> {
        if self.tag() != STRING_TAG {
            return None;
        }
        let slice = unsafe {
            let size = self.size() * std::mem::size_of::<Value>();
            let ptr = self.0.as_ptr().offset(1) as *mut u8;
            let last_byte = ptr.offset(size as isize - 1);
            let padding = *last_byte;
            let size = size - padding as usize - 1;
            std::slice::from_raw_parts(ptr, size)
        };
        Some(String::from_utf8_lossy(slice))
    }

    pub fn as_float(&self) -> Option<f64> {
        if self.tag() != DOUBLE_TAG {
            return None;
        }
        Some(f64::from_bits(self.0[1].0 as u64))
    }

    pub fn as_values(&self) -> Option<&[Value]> {
        if self.tag() >= NO_SCAN_TAG {
            return None;
        }
        Some(&self.0[1..])
    }

    pub(crate) fn header_ptr(&self) -> *const Value<'a> {
        self.0.as_ptr()
    }

    /// Helper for `Value::clone_with_allocator`.
    pub(crate) fn clone_with<'b, A: Allocator>(
        &self,
        alloc: &'b A,
        seen: &mut HashMap<usize, Value<'b>>,
    ) -> Value<'b> {
        let mut block = alloc.block_with_size_and_tag(self.size(), self.tag());
        match self.as_values() {
            Some(fields) => {
                for (i, field) in fields.into_iter().enumerate() {
                    let field = field.clone_with(alloc, seen);
                    A::set_field(&mut block, i, field)
                }
            }
            None => {
                // Safety: Both pointers must be valid, aligned, and
                // non-overlapping. Both pointers are the heads of blocks which
                // came from some Allocator. Allocators are required to allocate
                // blocks with usize-aligned pointers, and those blocks are
                // required to be valid for reads and writes for the number of
                // usize-sized fields reported in the size in their header.
                // Allocators are also required to allocate non-overlapping
                // blocks.
                unsafe {
                    std::ptr::copy_nonoverlapping(
                        self.0.as_ptr().offset(1) as *const usize,
                        block.as_mut_ptr() as *mut usize,
                        self.size(),
                    )
                }
            }
        }
        block.build()
    }
}

impl<'a> Index<usize> for Block<'a> {
    type Output = Value<'a>;

    fn index(&self, index: usize) -> &Self::Output {
        &self.0[index + 1]
    }
}

impl Debug for Block<'_> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if self.tag() == STRING_TAG {
            write!(f, "{:?}", self.as_str().unwrap())
        } else if self.tag() == DOUBLE_TAG {
            write!(f, "{:?}", self.as_float().unwrap())
        } else {
            write!(f, "{:?}", self.as_values().unwrap())
        }
    }
}

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Header(usize);

impl Header {
    pub(crate) fn new(size: usize, tag: u8) -> Self {
        let bits = size << 10 | (tag as usize);
        Header(bits)
    }

    pub fn size(self) -> usize {
        self.0 >> 10
    }

    pub fn tag(self) -> u8 {
        self.0 as u8
    }

    pub(crate) fn from_bits(bits: usize) -> Self {
        Header(bits)
    }

    pub(crate) fn to_bits(self) -> usize {
        self.0
    }
}

impl Debug for Header {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.debug_struct("Header")
            .field("size", &self.size())
            .field("tag", &self.tag())
            .finish()
    }
}
