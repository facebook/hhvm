// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::fmt::{self, Debug};
use std::mem;
use std::ops::{Index, IndexMut};

use crate::value::Value;

pub const STRING_TAG: u8 = 252;
pub const DOUBLE_TAG: u8 = 253;

#[repr(transparent)]
pub struct BlockBuilder<'arena: 'builder, 'builder>(pub(crate) &'builder mut [Value<'arena>]);

impl<'a, 'b> BlockBuilder<'a, 'b> {
    pub fn new(size: usize, tag: u8, block: &'b mut [Value<'a>]) -> Self {
        if size == 0 {
            panic!()
        }
        let header_bytes = size << 10 | (tag as usize);
        let header = Value::bits(header_bytes);
        block[0] = header;
        BlockBuilder(block)
    }

    pub fn build(self) -> Value<'a> {
        Value::bits(unsafe { mem::transmute(self.0.as_ptr().offset(1)) })
    }
}

impl<'a, 'b> Index<usize> for BlockBuilder<'a, 'b> {
    type Output = Value<'a>;

    fn index(&self, index: usize) -> &Self::Output {
        &self.0[index + 1]
    }
}

impl IndexMut<usize> for BlockBuilder<'_, '_> {
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        &mut self.0[index + 1]
    }
}

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Block<'arena>(pub(crate) &'arena [Value<'arena>]);

impl<'a> Block<'a> {
    fn header_bits(&self) -> usize {
        self.0[0].0
    }

    pub fn size(&self) -> usize {
        self.header_bits() >> 10
    }

    pub fn tag(&self) -> u8 {
        self.header_bits() as u8
    }

    pub fn as_str(&self) -> Option<Cow<str>> {
        if self.tag() != STRING_TAG {
            return None;
        }
        let slice = unsafe {
            let size = self.size() * mem::size_of::<usize>();
            let ptr: *mut u8 = mem::transmute(self.0.as_ptr().offset(1));
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
        if self.tag() == STRING_TAG || self.tag() == DOUBLE_TAG {
            return None;
        }
        Some(&self.0[1..])
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
