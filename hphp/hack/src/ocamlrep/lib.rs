// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::fmt::{self, Debug};
use std::mem;
use std::ops::{Index, IndexMut};

const STRING_TAG: u8 = 252;
const DOUBLE_TAG: u8 = 253;

#[derive(Clone)]
pub struct Block(Box<[Value]>);

impl Block {
    pub fn with_size(size: usize) -> Block {
        Block::with_size_and_tag(size, 0)
    }

    pub fn with_size_and_tag(size: usize, tag: u8) -> Block {
        if size == 0 {
            panic!()
        }
        let header_bytes = size << 10 | (tag as usize);
        let header = Value(header_bytes);
        let mut block = vec![Value::int(0); size + 1].into_boxed_slice();
        block[0] = header;
        Block(block)
    }

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
}

impl Index<usize> for Block {
    type Output = Value;

    fn index(&self, index: usize) -> &Self::Output {
        &self.0[index + 1]
    }
}

impl IndexMut<usize> for Block {
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        &mut self.0[index + 1]
    }
}

impl Debug for Block {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if self.tag() == STRING_TAG {
            write!(f, "{:?}", self.as_str().unwrap())
        } else if self.tag() == DOUBLE_TAG {
            write!(f, "{:?}", self.as_float().unwrap())
        } else {
            write!(f, "{:?}", &self.0[1..])
        }
    }
}

impl Into<Value> for Block {
    fn into(self) -> Value {
        let ptr = unsafe { self.0.as_ptr().offset(1) };
        mem::forget(self);
        Value(unsafe { mem::transmute(ptr) })
    }
}

#[repr(transparent)]
pub struct Value(usize);

impl Value {
    fn is_immediate(&self) -> bool {
        self.0 & 1 == 1
    }

    pub fn int(value: isize) -> Value {
        Value(((value as usize) << 1) | 1)
    }

    fn as_int(&self) -> isize {
        if !self.is_immediate() {
            panic!()
        }
        (self.0 as isize) >> 1
    }

    // This method is unsafe because it aliases the contents of the block.
    // For use by Clone/Drop only.
    unsafe fn as_block(&self) -> Option<Block> {
        if self.is_immediate() {
            return None;
        }
        let ptr: *const Value = mem::transmute(self.0);
        let header = ptr.offset(-1);
        let size = ((*header).0 >> 10) + 1;
        let slice = std::slice::from_raw_parts(header, size);
        let block: Block = mem::transmute(slice);
        Some(block)
    }
}

impl Into<ocaml::Value> for Value {
    fn into(self) -> ocaml::Value {
        ocaml::Value::new(unsafe { mem::transmute(self) })
    }
}

impl Clone for Value {
    fn clone(&self) -> Self {
        match unsafe { self.as_block() } {
            None => Value(self.0),
            Some(block1) => {
                let block2 = block1.clone();
                mem::forget(block1);
                block2.into()
            }
        }
    }
}

impl Drop for Value {
    fn drop(&mut self) {
        mem::drop(unsafe { self.as_block() })
    }
}

impl Debug for Value {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match unsafe { self.as_block() } {
            None => write!(f, "{}", self.as_int()),
            Some(block) => {
                write!(f, "{:?}", block)?;
                mem::forget(block);
                Ok(())
            }
        }
    }
}

impl From<bool> for Value {
    fn from(x: bool) -> Value {
        Value::int(x as isize)
    }
}

impl From<char> for Value {
    fn from(x: char) -> Value {
        Value::int(x as isize)
    }
}

impl From<isize> for Value {
    fn from(x: isize) -> Value {
        Value::int(x)
    }
}

impl From<usize> for Value {
    fn from(x: usize) -> Value {
        Value::int(x as isize)
    }
}

impl From<u64> for Value {
    fn from(x: u64) -> Value {
        Value::int(x as isize)
    }
}

impl From<i64> for Value {
    fn from(x: i64) -> Value {
        Value::int(x as isize)
    }
}

impl From<u32> for Value {
    fn from(x: u32) -> Value {
        Value::int(x as isize)
    }
}

impl From<i32> for Value {
    fn from(x: i32) -> Value {
        Value::int(x as isize)
    }
}

impl<T: Into<Value>> From<Option<T>> for Value {
    fn from(opt: Option<T>) -> Self {
        match opt {
            None => Value::int(0),
            Some(val) => {
                let mut block = Block::with_size(1);
                block[0] = val.into();
                block.into()
            }
        }
    }
}

impl<T: Into<Value>> From<Vec<T>> for Value {
    fn from(v: Vec<T>) -> Self {
        if v.len() == 0 {
            Value::int(0)
        } else {
            let mut current_block = Block::with_size(2);

            let mut iter = v.into_iter();
            let first_val = iter.next().unwrap();

            for val in iter.rev() {
                current_block[0] = val.into();

                let mut new_block = Block::with_size(2);
                new_block[1] = current_block.into();
                current_block = new_block;
            }

            current_block[0] = first_val.into();
            current_block.into()
        }
    }
}

impl From<String> for Value {
    fn from(s: String) -> Self {
        s.as_str().into()
    }
}

impl From<&str> for Value {
    fn from(s: &str) -> Self {
        let bytes_in_word = mem::size_of::<usize>();
        let blocks_length = 1 + (s.len() / bytes_in_word);
        let padding: usize = bytes_in_word - 1 - (s.len() % bytes_in_word);
        let mut block = Block::with_size_and_tag(blocks_length, STRING_TAG);

        block[blocks_length - 1] = Value(padding << ((bytes_in_word - 1) * 8));

        let ptr: *mut u8 = unsafe { mem::transmute(block.0.as_ptr().offset(1)) };
        let slice: &mut [u8] = unsafe { std::slice::from_raw_parts_mut(ptr, s.len()) };
        slice.copy_from_slice(s.as_bytes());

        block.into()
    }
}

impl From<f64> for Value {
    fn from(f: f64) -> Self {
        let mut block = Block::with_size_and_tag(1, DOUBLE_TAG);
        block[0] = Value(f.to_bits() as usize);
        block.into()
    }
}

impl From<()> for Value {
    fn from(_: ()) -> Self {
        0isize.into()
    }
}

impl<T: Into<Value>> From<Box<T>> for Value {
    fn from(b: Box<T>) -> Self {
        (*b).into()
    }
}

impl<T0, T1> From<(T0, T1)> for Value
where
    T0: Into<Value>,
    T1: Into<Value>,
{
    fn from(x: (T0, T1)) -> Self {
        let mut block = Block::with_size(2);
        block[0] = x.0.into();
        block[1] = x.1.into();
        block.into()
    }
}

impl<T0, T1, T2> From<(T0, T1, T2)> for Value
where
    T0: Into<Value>,
    T1: Into<Value>,
    T2: Into<Value>,
{
    fn from(x: (T0, T1, T2)) -> Self {
        let mut block = Block::with_size(3);
        block[0] = x.0.into();
        block[1] = x.1.into();
        block[2] = x.2.into();
        block.into()
    }
}

impl<T0, T1, T2, T3> From<(T0, T1, T2, T3)> for Value
where
    T0: Into<Value>,
    T1: Into<Value>,
    T2: Into<Value>,
    T3: Into<Value>,
{
    fn from(x: (T0, T1, T2, T3)) -> Self {
        let mut block = Block::with_size(4);
        block[0] = x.0.into();
        block[1] = x.1.into();
        block[2] = x.2.into();
        block[3] = x.3.into();
        block.into()
    }
}

impl<T0, T1, T2, T3, T4> From<(T0, T1, T2, T3, T4)> for Value
where
    T0: Into<Value>,
    T1: Into<Value>,
    T2: Into<Value>,
    T3: Into<Value>,
    T4: Into<Value>,
{
    fn from(x: (T0, T1, T2, T3, T4)) -> Self {
        let mut block = Block::with_size(5);
        block[0] = x.0.into();
        block[1] = x.1.into();
        block[2] = x.2.into();
        block[3] = x.3.into();
        block[4] = x.4.into();
        block.into()
    }
}

impl<T0, T1, T2, T3, T4, T5> From<(T0, T1, T2, T3, T4, T5)> for Value
where
    T0: Into<Value>,
    T1: Into<Value>,
    T2: Into<Value>,
    T3: Into<Value>,
    T4: Into<Value>,
    T5: Into<Value>,
{
    fn from(x: (T0, T1, T2, T3, T4, T5)) -> Self {
        let mut block = Block::with_size(6);
        block[0] = x.0.into();
        block[1] = x.1.into();
        block[2] = x.2.into();
        block[3] = x.3.into();
        block[4] = x.4.into();
        block[5] = x.5.into();
        block.into()
    }
}

impl<T0, T1, T2, T3, T4, T5, T6> From<(T0, T1, T2, T3, T4, T5, T6)> for Value
where
    T0: Into<Value>,
    T1: Into<Value>,
    T2: Into<Value>,
    T3: Into<Value>,
    T4: Into<Value>,
    T5: Into<Value>,
    T6: Into<Value>,
{
    fn from(x: (T0, T1, T2, T3, T4, T5, T6)) -> Self {
        let mut block = Block::with_size(7);
        block[0] = x.0.into();
        block[1] = x.1.into();
        block[2] = x.2.into();
        block[3] = x.3.into();
        block[4] = x.4.into();
        block[5] = x.5.into();
        block[6] = x.6.into();
        block.into()
    }
}

impl<T0, T1, T2, T3, T4, T5, T6, T7> From<(T0, T1, T2, T3, T4, T5, T6, T7)> for Value
where
    T0: Into<Value>,
    T1: Into<Value>,
    T2: Into<Value>,
    T3: Into<Value>,
    T4: Into<Value>,
    T5: Into<Value>,
    T6: Into<Value>,
    T7: Into<Value>,
{
    fn from(x: (T0, T1, T2, T3, T4, T5, T6, T7)) -> Self {
        let mut block = Block::with_size(8);
        block[0] = x.0.into();
        block[1] = x.1.into();
        block[2] = x.2.into();
        block[3] = x.3.into();
        block[4] = x.4.into();
        block[5] = x.5.into();
        block[6] = x.6.into();
        block[7] = x.7.into();
        block.into()
    }
}
