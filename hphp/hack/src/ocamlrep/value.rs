// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::collections::HashMap;
use std::fmt;
use std::fmt::Debug;
use std::marker::PhantomData;

use crate::block;
use crate::block::Block;
use crate::block::Header;
use crate::Allocator;

#[inline(always)]
pub const fn is_ocaml_int(value: usize) -> bool {
    value & 1 == 1
}

#[inline(always)]
pub const fn isize_to_ocaml_int(value: isize) -> usize {
    ((value as usize) << 1) | 1
}

#[inline(always)]
pub const fn ocaml_int_to_isize(value: usize) -> isize {
    (value as isize) >> 1
}

/// A value, as represented by OCaml. Valid, immutable, and immovable for
/// lifetime `'a`.
///
/// Either a tagged integer value or a pointer to a [`Block`](struct.Block.html)
/// containing fields or binary data.
#[repr(transparent)]
#[derive(Clone, Copy, Hash, PartialEq, Eq)]
pub struct Value<'a>(pub(crate) usize, PhantomData<&'a ()>);

impl<'a> Value<'a> {
    #[inline(always)]
    pub const fn is_int(self) -> bool {
        is_ocaml_int(self.0)
    }

    #[inline(always)]
    pub const fn is_block(self) -> bool {
        !self.is_int()
    }

    #[inline(always)]
    pub const fn int(value: isize) -> Value<'static> {
        Value(isize_to_ocaml_int(value), PhantomData)
    }

    #[inline(always)]
    pub fn as_int(self) -> Option<isize> {
        if self.is_int() {
            Some(ocaml_int_to_isize(self.0))
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn as_block(self) -> Option<Block<'a>> {
        if self.is_int() {
            return None;
        }
        let block = unsafe {
            let ptr = self.0 as *const Value<'_>;
            let header = ptr.offset(-1);
            let size = Header::from_bits((*header).to_bits()).size() + 1;
            std::slice::from_raw_parts(header, size)
        };
        Some(Block(block))
    }

    #[inline(always)]
    pub fn as_float(self) -> Option<f64> {
        let block = self.as_block()?;
        if block.tag() != block::DOUBLE_TAG {
            return None;
        }
        Some(f64::from_bits(block[0].0 as u64))
    }

    #[inline(always)]
    pub fn as_double_array(self) -> Option<&'a [f64]> {
        let block = self.as_block()?;
        if block.tag() != block::DOUBLE_ARRAY_TAG {
            return None;
        }
        Some(unsafe {
            std::slice::from_raw_parts(block.0.as_ptr().add(1) as *const f64, block.size())
        })
    }

    #[inline(always)]
    pub fn as_byte_string(self) -> Option<&'a [u8]> {
        let block = self.as_block()?;
        if block.tag() != block::STRING_TAG {
            return None;
        }
        let slice = unsafe {
            let size = block.size() * std::mem::size_of::<Value<'_>>();
            let ptr = self.0 as *mut u8;
            let last_byte = ptr.offset(size as isize - 1);
            let padding = *last_byte;
            let size = size - padding as usize - 1;
            std::slice::from_raw_parts(ptr, size)
        };
        Some(slice)
    }

    #[inline(always)]
    pub fn as_str(self) -> Option<Cow<'a, str>> {
        let slice = self.as_byte_string()?;
        Some(String::from_utf8_lossy(slice))
    }

    #[inline(always)]
    pub fn field(self, index: usize) -> Option<Value<'a>> {
        self.field_ref(index).copied()
    }

    #[inline(always)]
    pub fn field_ref(self, index: usize) -> Option<&'a Value<'a>> {
        self.as_block()?.as_values()?.get(index)
    }

    /// Given a pointer to the first field of a [`Block`](struct.Block.html),
    /// create a pointer `Value` referencing that `Block`.
    ///
    /// # Safety
    ///
    /// This method is unsafe because it requires that the pointed-to Value is
    /// the first field of a block, which must be preceded by a valid Header
    /// correctly describing the block's size and tag (i.e., value.offset(-1)
    /// should point to that Header). To be used only with pointers returned by
    /// Arena allocation methods (e.g.,
    /// [`Allocator::block_with_size_and_tag`](trait.Allocator.html#tymethod.block_with_size_and_tag).
    #[inline(always)]
    pub unsafe fn from_ptr(value: *const Value<'a>) -> Value<'a> {
        Value(value as usize, PhantomData)
    }

    /// # Safety
    ///
    /// The lifetime 'a returned is arbitrarily chosen, and does not necessarily
    /// reflect the actual lifetime of the data. If the given value is a
    /// pointer, it is up to the caller to ensure that for the duration of this
    /// lifetime, the block this pointer points to, and every value reachable
    /// from it, does not get written to.
    #[inline(always)]
    pub const unsafe fn from_bits(value: usize) -> Value<'a> {
        Value(value, PhantomData)
    }

    /// Convert this value to a usize, which can be handed to the OCaml runtime
    /// to be used as an OCaml value. Take care that the returned value does
    /// not outlive the arena.
    #[inline(always)]
    pub const fn to_bits(self) -> usize {
        self.0
    }

    /// Helper for `Value::clone_with_allocator`.
    pub(crate) fn clone_with<'b>(
        self,
        alloc: &'b impl Allocator,
        seen: &mut HashMap<usize, Value<'b>>,
    ) -> Value<'b> {
        match self.as_block() {
            None => Value(self.0, PhantomData),
            Some(block) => {
                if let Some(&copied_value) = seen.get(&self.0) {
                    return copied_value;
                }
                let copied_value = block.clone_with(alloc, seen);
                seen.insert(self.0, copied_value);
                copied_value
            }
        }
    }

    /// Recursively clone this `Value` using the given `Allocator`. Structural
    /// sharing is preserved (i.e., values which are physically equal before the
    /// clone will be physically equal after the clone).
    pub fn clone_with_allocator(self, alloc: &impl Allocator) -> Value<'_> {
        self.clone_with(alloc, &mut HashMap::new())
    }
}

impl Debug for Value<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.as_block() {
            None => write!(f, "{}", self.as_int().unwrap()),
            Some(block) => write!(f, "{:?}", block),
        }
    }
}
