// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::{btree_map, btree_set, BTreeMap, BTreeSet};
use std::convert::TryInto;
use std::mem;
use std::path::PathBuf;
use std::rc::Rc;

use crate::arena::Arena;
use crate::block;
use crate::value::Value;
use crate::OcamlRep;

impl OcamlRep for () {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(0)
    }
}

impl OcamlRep for isize {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self)
    }
}

impl OcamlRep for usize {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl OcamlRep for i64 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl OcamlRep for u64 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl OcamlRep for i32 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl OcamlRep for u32 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }
}

impl OcamlRep for bool {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.into())
    }
}

impl OcamlRep for char {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        if self as u32 > 255 {
            panic!("char out of range: {}", self.to_string())
        }
        Value::int(self as isize)
    }
}

impl OcamlRep for f64 {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size_and_tag(1, block::DOUBLE_TAG);
        block[0] = Value::bits(self.to_bits() as usize);
        block.build()
    }
}

impl<T: OcamlRep> OcamlRep for Box<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(*self)
    }
}

impl<T: OcamlRep + Clone> OcamlRep for Rc<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(self.as_ref().clone())
    }
}

impl<T: OcamlRep> OcamlRep for Option<T> {
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

impl<T: OcamlRep> OcamlRep for Vec<T> {
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

impl<K: OcamlRep, V: OcamlRep> OcamlRep for BTreeMap<K, V> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        if self.is_empty() {
            return Value::int(0);
        }
        let len = self.len();
        let mut iter = self.into_iter();
        let (res, _) = btree_map_helper(&mut iter, arena, len);
        res
    }
}

fn btree_map_helper<'a, K: OcamlRep, V: OcamlRep>(
    iter: &mut btree_map::IntoIter<K, V>,
    arena: &Arena<'a>,
    size: usize,
) -> (Value<'a>, usize) {
    if size == 0 {
        return (Value::int(0), 0);
    }
    let (left, left_height) = btree_map_helper(iter, arena, size / 2);
    let (key, val) = iter.next().unwrap();
    let (right, right_height) = btree_map_helper(iter, arena, size - 1 - size / 2);
    let height = std::cmp::max(left_height, right_height) + 1;
    let mut block = arena.block_with_size(5);
    block[0] = left;
    block[1] = arena.add(key);
    block[2] = arena.add(val);
    block[3] = right;
    block[4] = arena.add(height);
    (block.build(), height)
}

impl<T: OcamlRep> OcamlRep for BTreeSet<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        if self.is_empty() {
            return Value::int(0);
        }
        let len = self.len();
        let mut iter = self.into_iter();
        let (res, _) = btree_set_helper(&mut iter, arena, len);
        res
    }
}

fn btree_set_helper<'a, T: OcamlRep>(
    iter: &mut btree_set::IntoIter<T>,
    arena: &Arena<'a>,
    size: usize,
) -> (Value<'a>, usize) {
    if size == 0 {
        return (Value::int(0), 0);
    }
    let (left, left_height) = btree_set_helper(iter, arena, size / 2);
    let val = iter.next().unwrap();
    let (right, right_height) = btree_set_helper(iter, arena, size - 1 - size / 2);
    let height = std::cmp::max(left_height, right_height) + 1;
    let mut block = arena.block_with_size(4);
    block[0] = left;
    block[1] = arena.add(val);
    block[2] = right;
    block[3] = arena.add(height);
    (block.build(), height)
}

impl OcamlRep for PathBuf {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(self.to_str().unwrap())
    }
}

impl OcamlRep for String {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(self.as_str())
    }
}

impl OcamlRep for &str {
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

impl<T0, T1> OcamlRep for (T0, T1)
where
    T0: OcamlRep,
    T1: OcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size(2);
        block[0] = self.0.into_ocamlrep(arena);
        block[1] = self.1.into_ocamlrep(arena);
        block.build()
    }
}

impl<T0, T1, T2> OcamlRep for (T0, T1, T2)
where
    T0: OcamlRep,
    T1: OcamlRep,
    T2: OcamlRep,
{
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size(3);
        block[0] = self.0.into_ocamlrep(arena);
        block[1] = self.1.into_ocamlrep(arena);
        block[2] = self.2.into_ocamlrep(arena);
        block.build()
    }
}

impl<T0, T1, T2, T3> OcamlRep for (T0, T1, T2, T3)
where
    T0: OcamlRep,
    T1: OcamlRep,
    T2: OcamlRep,
    T3: OcamlRep,
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

impl<T0, T1, T2, T3, T4> OcamlRep for (T0, T1, T2, T3, T4)
where
    T0: OcamlRep,
    T1: OcamlRep,
    T2: OcamlRep,
    T3: OcamlRep,
    T4: OcamlRep,
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

impl<T0, T1, T2, T3, T4, T5> OcamlRep for (T0, T1, T2, T3, T4, T5)
where
    T0: OcamlRep,
    T1: OcamlRep,
    T2: OcamlRep,
    T3: OcamlRep,
    T4: OcamlRep,
    T5: OcamlRep,
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

impl<T0, T1, T2, T3, T4, T5, T6> OcamlRep for (T0, T1, T2, T3, T4, T5, T6)
where
    T0: OcamlRep,
    T1: OcamlRep,
    T2: OcamlRep,
    T3: OcamlRep,
    T4: OcamlRep,
    T5: OcamlRep,
    T6: OcamlRep,
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

impl<T0, T1, T2, T3, T4, T5, T6, T7> OcamlRep for (T0, T1, T2, T3, T4, T5, T6, T7)
where
    T0: OcamlRep,
    T1: OcamlRep,
    T2: OcamlRep,
    T3: OcamlRep,
    T4: OcamlRep,
    T5: OcamlRep,
    T6: OcamlRep,
    T7: OcamlRep,
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
