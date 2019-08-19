// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::fmt::{self, Debug};
use std::marker::PhantomData;
use std::mem;
use std::ops::{Index, IndexMut};

pub mod arena;

pub use arena::Arena;

const STRING_TAG: u8 = 252;
const DOUBLE_TAG: u8 = 253;

pub trait IntoOcamlRep {
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a>;
}

#[repr(transparent)]
pub struct BlockBuilder<'arena: 'builder, 'builder>(&'builder mut [Value<'arena>]);

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
pub struct Block<'arena>(&'arena [Value<'arena>]);

impl<'a> Block<'a> {
    fn header_bits(&self) -> usize {
        self.0[0].0
    }

    fn size(&self) -> usize {
        self.header_bits() >> 10
    }

    fn tag(&self) -> u8 {
        self.header_bits() as u8
    }

    fn as_str(&self) -> Option<Cow<str>> {
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

    fn as_float(&self) -> Option<f64> {
        if self.tag() != DOUBLE_TAG {
            return None;
        }
        Some(f64::from_bits(self.0[1].0 as u64))
    }

    fn as_values(&self) -> Option<&[Value]> {
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

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Value<'arena>(usize, PhantomData<&'arena ()>);

impl<'a> Value<'a> {
    fn is_immediate(&self) -> bool {
        self.0 & 1 == 1
    }

    pub fn int(value: isize) -> Value<'static> {
        Value(((value as usize) << 1) | 1, PhantomData)
    }

    fn bits(value: usize) -> Value<'a> {
        Value(value, PhantomData)
    }

    fn as_int(&self) -> isize {
        if !self.is_immediate() {
            panic!()
        }
        (self.0 as isize) >> 1
    }

    fn as_block(&self) -> Option<Block<'a>> {
        if self.is_immediate() {
            return None;
        }
        let block = unsafe {
            let ptr: *const Value = mem::transmute(self.0);
            let header = ptr.offset(-1);
            let size = ((*header).0 >> 10) + 1;
            std::slice::from_raw_parts(header, size)
        };
        Some(Block(block))
    }

    /// This method is unsafe because it decouples the value from the lifetime
    /// of the arena. Take care that the returned value does not outlive the
    /// arena.
    pub unsafe fn as_usize(&self) -> usize {
        self.0
    }
}

impl Debug for Value<'_> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self.as_block() {
            None => write!(f, "{}", self.as_int()),
            Some(block) => write!(f, "{:?}", block),
        }
    }
}

impl IntoOcamlRep for bool {
    fn into_ocamlrep<'a>(self, _arena: &mut Arena<'a>) -> Value<'a> {
        Value::int(self as isize)
    }
}

impl IntoOcamlRep for char {
    fn into_ocamlrep<'a>(self, _arena: &mut Arena<'a>) -> Value<'a> {
        Value::int(self as isize)
    }
}

impl IntoOcamlRep for isize {
    fn into_ocamlrep<'a>(self, _arena: &mut Arena<'a>) -> Value<'a> {
        Value::int(self)
    }
}

impl IntoOcamlRep for usize {
    fn into_ocamlrep<'a>(self, _arena: &mut Arena<'a>) -> Value<'a> {
        Value::int(self as isize)
    }
}

impl IntoOcamlRep for u64 {
    fn into_ocamlrep<'a>(self, _arena: &mut Arena<'a>) -> Value<'a> {
        Value::int(self as isize)
    }
}

impl IntoOcamlRep for i64 {
    fn into_ocamlrep<'a>(self, _arena: &mut Arena<'a>) -> Value<'a> {
        Value::int(self as isize)
    }
}

impl IntoOcamlRep for u32 {
    fn into_ocamlrep<'a>(self, _arena: &mut Arena<'a>) -> Value<'a> {
        Value::int(self as isize)
    }
}

impl IntoOcamlRep for i32 {
    fn into_ocamlrep<'a>(self, _arena: &mut Arena<'a>) -> Value<'a> {
        Value::int(self as isize)
    }
}

impl<T: IntoOcamlRep> IntoOcamlRep for Option<T> {
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        match self {
            None => Value::int(0),
            Some(val) => {
                let val = arena.add(val);
                let mut block = arena.block_with_size(1);
                block[0] = val;
                block.build()
            }
        }
    }
}

impl<T: IntoOcamlRep> IntoOcamlRep for Vec<T> {
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let mut hd = arena.add(());
        for val in self.into_iter().rev() {
            let val = arena.add(val);
            let mut current_block = arena.block_with_size(2);
            current_block[0] = val;
            current_block[1] = hd;
            hd = current_block.build();
        }
        hd
    }
}

impl IntoOcamlRep for String {
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        self.as_str().into_ocamlrep(arena)
    }
}

impl IntoOcamlRep for &str {
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let bytes_in_word = mem::size_of::<usize>();
        let blocks_length = 1 + (self.len() / bytes_in_word);
        let padding: usize = bytes_in_word - 1 - (self.len() % bytes_in_word);
        let mut block = arena.block_with_size_and_tag(blocks_length, STRING_TAG);

        block[blocks_length - 1] = Value::bits(padding << ((bytes_in_word - 1) * 8));

        let slice: &mut [u8] = unsafe {
            let ptr: *mut u8 = mem::transmute(block.0.as_ptr().offset(1));
            std::slice::from_raw_parts_mut(ptr, self.len())
        };
        slice.copy_from_slice(self.as_bytes());

        block.build()
    }
}

impl IntoOcamlRep for f64 {
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size_and_tag(1, DOUBLE_TAG);
        block[0] = Value::bits(self.to_bits() as usize);
        block.build()
    }
}

impl IntoOcamlRep for () {
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        0isize.into_ocamlrep(arena)
    }
}

impl<T: IntoOcamlRep> IntoOcamlRep for Box<T> {
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        (*self).into_ocamlrep(arena)
    }
}

impl<T0, T1> IntoOcamlRep for (T0, T1)
where
    T0: IntoOcamlRep,
    T1: IntoOcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let v0 = arena.add(self.0);
        let v1 = arena.add(self.1);
        let mut block = arena.block_with_size(2);
        block[0] = v0;
        block[1] = v1;
        block.build()
    }
}

impl<T0, T1, T2> IntoOcamlRep for (T0, T1, T2)
where
    T0: IntoOcamlRep,
    T1: IntoOcamlRep,
    T2: IntoOcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let v0 = arena.add(self.0);
        let v1 = arena.add(self.1);
        let v2 = arena.add(self.2);
        let mut block = arena.block_with_size(3);
        block[0] = v0;
        block[1] = v1;
        block[2] = v2;
        block.build()
    }
}

impl<T0, T1, T2, T3> IntoOcamlRep for (T0, T1, T2, T3)
where
    T0: IntoOcamlRep,
    T1: IntoOcamlRep,
    T2: IntoOcamlRep,
    T3: IntoOcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let v0 = arena.add(self.0);
        let v1 = arena.add(self.1);
        let v2 = arena.add(self.2);
        let v3 = arena.add(self.3);
        let mut block = arena.block_with_size(4);
        block[0] = v0;
        block[1] = v1;
        block[2] = v2;
        block[3] = v3;
        block.build()
    }
}

impl<T0, T1, T2, T3, T4> IntoOcamlRep for (T0, T1, T2, T3, T4)
where
    T0: IntoOcamlRep,
    T1: IntoOcamlRep,
    T2: IntoOcamlRep,
    T3: IntoOcamlRep,
    T4: IntoOcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let v0 = arena.add(self.0);
        let v1 = arena.add(self.1);
        let v2 = arena.add(self.2);
        let v3 = arena.add(self.3);
        let v4 = arena.add(self.4);
        let mut block = arena.block_with_size(5);
        block[0] = v0;
        block[1] = v1;
        block[2] = v2;
        block[3] = v3;
        block[4] = v4;
        block.build()
    }
}

impl<T0, T1, T2, T3, T4, T5> IntoOcamlRep for (T0, T1, T2, T3, T4, T5)
where
    T0: IntoOcamlRep,
    T1: IntoOcamlRep,
    T2: IntoOcamlRep,
    T3: IntoOcamlRep,
    T4: IntoOcamlRep,
    T5: IntoOcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let v0 = arena.add(self.0);
        let v1 = arena.add(self.1);
        let v2 = arena.add(self.2);
        let v3 = arena.add(self.3);
        let v4 = arena.add(self.4);
        let v5 = arena.add(self.5);
        let mut block = arena.block_with_size(6);
        block[0] = v0;
        block[1] = v1;
        block[2] = v2;
        block[3] = v3;
        block[4] = v4;
        block[5] = v5;
        block.build()
    }
}

impl<T0, T1, T2, T3, T4, T5, T6> IntoOcamlRep for (T0, T1, T2, T3, T4, T5, T6)
where
    T0: IntoOcamlRep,
    T1: IntoOcamlRep,
    T2: IntoOcamlRep,
    T3: IntoOcamlRep,
    T4: IntoOcamlRep,
    T5: IntoOcamlRep,
    T6: IntoOcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let v0 = arena.add(self.0);
        let v1 = arena.add(self.1);
        let v2 = arena.add(self.2);
        let v3 = arena.add(self.3);
        let v4 = arena.add(self.4);
        let v5 = arena.add(self.5);
        let v6 = arena.add(self.6);
        let mut block = arena.block_with_size(7);
        block[0] = v0;
        block[1] = v1;
        block[2] = v2;
        block[3] = v3;
        block[4] = v4;
        block[5] = v5;
        block[6] = v6;
        block.build()
    }
}

impl<T0, T1, T2, T3, T4, T5, T6, T7> IntoOcamlRep for (T0, T1, T2, T3, T4, T5, T6, T7)
where
    T0: IntoOcamlRep,
    T1: IntoOcamlRep,
    T2: IntoOcamlRep,
    T3: IntoOcamlRep,
    T4: IntoOcamlRep,
    T5: IntoOcamlRep,
    T6: IntoOcamlRep,
    T7: IntoOcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &mut Arena<'a>) -> Value<'a> {
        let v0 = arena.add(self.0);
        let v1 = arena.add(self.1);
        let v2 = arena.add(self.2);
        let v3 = arena.add(self.3);
        let v4 = arena.add(self.4);
        let v5 = arena.add(self.5);
        let v6 = arena.add(self.6);
        let v7 = arena.add(self.7);
        let mut block = arena.block_with_size(8);
        block[0] = v0;
        block[1] = v1;
        block[2] = v2;
        block[3] = v3;
        block[4] = v4;
        block[5] = v5;
        block[6] = v6;
        block[7] = v7;
        block.build()
    }
}
