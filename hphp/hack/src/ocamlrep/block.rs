// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashMap;
use std::fmt::{self, Debug};
use std::marker::PhantomData;
use std::ops::Index;

use crate::{Allocator, OpaqueValue, Value};

/// Blocks with tags greater than or equal to NO_SCAN_TAG contain binary data,
/// and are not scanned by the garbage collector. Likewise, we must avoid
/// interpreting the fields of blocks with such tags as Values.
pub const CLOSURE_TAG: u8 = 247;
pub const NO_SCAN_TAG: u8 = 251;
pub const STRING_TAG: u8 = 252;
pub const DOUBLE_TAG: u8 = 253;
pub const CUSTOM_TAG: u8 = 255;

/// A recently-allocated, not-yet-finalized Block.
///
/// Morally equivalent to a `&'b mut [OpaqueValue<'a>]` slice where `'a: 'b`,
/// but `Allocator`s may choose to use some non-pointer value (such as an offset
/// into a `Vec`) as the address.
pub struct BlockBuilder<'a> {
    address: usize,
    size: usize,
    _phantom: PhantomData<&'a mut [OpaqueValue<'a>]>,
}

impl<'a> BlockBuilder<'a> {
    /// `address` may be a pointer or an offset (the `Allocator` which invokes
    /// `BlockBuilder::new` determines the meaning of `BlockBuilder` addresses).
    /// `size` must be greater than 0 and denotes the number of fields in the
    /// block.
    #[inline(always)]
    pub fn new(address: usize, size: usize) -> Self {
        if size == 0 {
            panic!()
        }
        BlockBuilder {
            address,
            size,
            _phantom: PhantomData,
        }
    }

    /// The address passed to `BlockBuilder::new`. May be a pointer or an offset
    /// (the `Allocator` which invokes `BlockBuilder::new` determines the
    /// meaning of `BlockBuilder` addresses).
    #[inline(always)]
    pub fn address(&self) -> usize {
        self.address
    }

    /// The number of fields in this block.
    #[inline(always)]
    pub fn size(&self) -> usize {
        self.size
    }

    #[inline(always)]
    pub fn build(self) -> OpaqueValue<'a> {
        unsafe { OpaqueValue::from_bits(self.address) }
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

    pub fn as_value(&self) -> Value {
        unsafe { Value::from_ptr(&self.0[1]) }
    }

    pub fn as_values(&self) -> Option<&[Value]> {
        if self.tag() >= NO_SCAN_TAG {
            return None;
        }
        Some(&self.0[1..])
    }

    /// Helper for `Value::clone_with_allocator`.
    pub(crate) fn clone_with<'b, A: Allocator>(
        &self,
        alloc: &'b A,
        seen: &mut HashMap<usize, OpaqueValue<'b>>,
    ) -> OpaqueValue<'b> {
        let mut block = alloc.block_with_size_and_tag(self.size(), self.tag());
        match self.as_values() {
            Some(fields) => {
                for (i, field) in fields.into_iter().enumerate() {
                    let field = field.clone_with(alloc, seen);
                    alloc.set_field(&mut block, i, field)
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
                        alloc.block_ptr_mut(&mut block) as *mut usize,
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
            write!(f, "{:?}", self.as_value().as_str().unwrap())
        } else if self.tag() == DOUBLE_TAG {
            write!(f, "{:?}", self.as_value().as_float().unwrap())
        } else {
            write!(f, "{}{:?}", self.tag(), self.as_values().unwrap())
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
