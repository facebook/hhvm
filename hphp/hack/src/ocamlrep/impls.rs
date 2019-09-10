// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::TryInto;
use std::mem;
use std::path::PathBuf;
use std::rc::Rc;

use crate::arena::Arena;
use crate::block;
use crate::value::Value;
use crate::IntoOcamlRep;

impl IntoOcamlRep for () {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(0)
    }
}

impl IntoOcamlRep for isize {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self)
    }
}

impl IntoOcamlRep for usize {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl IntoOcamlRep for i64 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl IntoOcamlRep for u64 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl IntoOcamlRep for i32 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl IntoOcamlRep for u32 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl IntoOcamlRep for bool {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.into())
    }
}

impl IntoOcamlRep for char {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        if self as u32 > 255 {
            panic!("char out of range: {}", self.to_string())
        }
        Value::int(self as isize)
    }
}

impl IntoOcamlRep for f64 {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size_and_tag(1, block::DOUBLE_TAG);
        block[0] = Value::bits(self.to_bits() as usize);
        block.build()
    }
}

impl<T: IntoOcamlRep> IntoOcamlRep for Box<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(*self)
    }
}

impl<T: IntoOcamlRep + Clone> IntoOcamlRep for Rc<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(self.as_ref().clone())
    }
}

impl<T: IntoOcamlRep> IntoOcamlRep for Option<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        match self {
            None => Value::int(0),
            Some(val) => {
                let mut block = arena.block_with_size(1);
                block[0] = arena.add(val);
                block.build()
            }
        }
    }
}

impl<T: IntoOcamlRep> IntoOcamlRep for Vec<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut hd = arena.add(());
        for val in self.into_iter().rev() {
            let mut current_block = arena.block_with_size(2);
            current_block[0] = arena.add(val);
            current_block[1] = hd;
            hd = current_block.build();
        }
        hd
    }
}

impl IntoOcamlRep for PathBuf {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(self.to_str().unwrap())
    }
}

impl IntoOcamlRep for String {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(self.as_str())
    }
}

impl IntoOcamlRep for &str {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let bytes_in_word = mem::size_of::<usize>();
        let blocks_length = 1 + (self.len() / bytes_in_word);
        let padding: usize = bytes_in_word - 1 - (self.len() % bytes_in_word);
        let mut block = arena.block_with_size_and_tag(blocks_length, block::STRING_TAG);

        block[blocks_length - 1] = Value::bits(padding << ((bytes_in_word - 1) * 8));

        let slice: &mut [u8] = unsafe {
            let ptr: *mut u8 = mem::transmute(block.0.as_ptr().offset(1));
            std::slice::from_raw_parts_mut(ptr, self.len())
        };
        slice.copy_from_slice(self.as_bytes());

        block.build()
    }
}

impl<T0, T1> IntoOcamlRep for (T0, T1)
where
    T0: IntoOcamlRep,
    T1: IntoOcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size(2);
        block[0] = self.0.into_ocamlrep(arena);
        block[1] = self.1.into_ocamlrep(arena);
        block.build()
    }
}

impl<T0, T1, T2> IntoOcamlRep for (T0, T1, T2)
where
    T0: IntoOcamlRep,
    T1: IntoOcamlRep,
    T2: IntoOcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size(3);
        block[0] = self.0.into_ocamlrep(arena);
        block[1] = self.1.into_ocamlrep(arena);
        block[2] = self.2.into_ocamlrep(arena);
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
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size(4);
        block[0] = self.0.into_ocamlrep(arena);
        block[1] = self.1.into_ocamlrep(arena);
        block[2] = self.2.into_ocamlrep(arena);
        block[3] = self.3.into_ocamlrep(arena);
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
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size(5);
        block[0] = self.0.into_ocamlrep(arena);
        block[1] = self.1.into_ocamlrep(arena);
        block[2] = self.2.into_ocamlrep(arena);
        block[3] = self.3.into_ocamlrep(arena);
        block[4] = self.4.into_ocamlrep(arena);
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
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size(6);
        block[0] = self.0.into_ocamlrep(arena);
        block[1] = self.1.into_ocamlrep(arena);
        block[2] = self.2.into_ocamlrep(arena);
        block[3] = self.3.into_ocamlrep(arena);
        block[4] = self.4.into_ocamlrep(arena);
        block[5] = self.5.into_ocamlrep(arena);
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
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size(7);
        block[0] = self.0.into_ocamlrep(arena);
        block[1] = self.1.into_ocamlrep(arena);
        block[2] = self.2.into_ocamlrep(arena);
        block[3] = self.3.into_ocamlrep(arena);
        block[4] = self.4.into_ocamlrep(arena);
        block[5] = self.5.into_ocamlrep(arena);
        block[6] = self.6.into_ocamlrep(arena);
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
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size(8);
        block[0] = self.0.into_ocamlrep(arena);
        block[1] = self.1.into_ocamlrep(arena);
        block[2] = self.2.into_ocamlrep(arena);
        block[3] = self.3.into_ocamlrep(arena);
        block[4] = self.4.into_ocamlrep(arena);
        block[5] = self.5.into_ocamlrep(arena);
        block[6] = self.6.into_ocamlrep(arena);
        block[7] = self.7.into_ocamlrep(arena);
        block.build()
    }
}
