// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::{btree_map, btree_set, BTreeMap, BTreeSet};
use std::convert::TryInto;
use std::path::PathBuf;
use std::rc::Rc;

use crate::{block, from};
use crate::{Arena, FromError, OcamlRep, Value};

fn expect_block_with_size_and_tag<'a>(
    value: Value<'a>,
    size: usize,
    tag: u8,
) -> Result<block::Block<'a>, FromError> {
    let block = from::expect_block(value)?;
    from::expect_block_size(block, size)?;
    from::expect_block_tag(block, tag)?;
    Ok(block)
}

impl OcamlRep for () {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(0)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        match from::expect_int(value)? {
            0 => Ok(()),
            x => Err(FromError::ExpectedUnit(x)),
        }
    }
}

impl OcamlRep for isize {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        from::expect_int(value)
    }
}

impl OcamlRep for usize {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for i64 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for u64 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for i32 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for u32 {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for bool {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        Value::int(self.into())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        match from::expect_int(value)? {
            0 => Ok(false),
            1 => Ok(true),
            x => Err(FromError::ExpectedBool(x)),
        }
    }
}

impl OcamlRep for char {
    fn into_ocamlrep<'a>(self, _arena: &Arena<'a>) -> Value<'a> {
        if self as u32 > 255 {
            panic!("char out of range: {}", self.to_string())
        }
        Value::int(self as isize)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let c = from::expect_int(value)?;
        if 0 <= c && c <= 255 {
            Ok(c as u8 as char)
        } else {
            Err(FromError::ExpectedChar(c))
        }
    }
}

impl OcamlRep for f64 {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        let mut block = arena.block_with_size_and_tag(1, block::DOUBLE_TAG);
        block[0] = unsafe { Value::from_bits(self.to_bits() as usize) };
        block.build()
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = expect_block_with_size_and_tag(value, 1, block::DOUBLE_TAG)?;
        Ok(f64::from_bits(block[0].0 as u64))
    }
}

impl<T: OcamlRep> OcamlRep for Box<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(*self)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(Box::new(T::from_ocamlrep(value)?))
    }
}

impl<T: OcamlRep + Clone> OcamlRep for Rc<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        arena.add(self.as_ref().clone())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        // NB: We don't get any sharing this way.
        Ok(Rc::new(T::from_ocamlrep(value)?))
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

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        if value.is_immediate() {
            let _ = from::expect_nullary_variant(value, 0)?;
            Ok(None)
        } else {
            let block = expect_block_with_size_and_tag(value, 1, 0)?;
            Ok(Some(from::field(block, 0)?))
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

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let mut vec = vec![];
        let mut hd = value;
        while !hd.is_immediate() {
            let block = from::expect_tuple(hd, 2)?;
            vec.push(from::field(block, 0)?);
            hd = block[1];
        }
        let hd = hd.as_int().unwrap();
        if hd != 0 {
            return Err(FromError::ExpectedUnit(hd));
        }
        Ok(vec)
    }
}

impl<K: OcamlRep + Ord, V: OcamlRep> OcamlRep for BTreeMap<K, V> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        if self.is_empty() {
            return Value::int(0);
        }
        let len = self.len();
        let mut iter = self.into_iter();
        let (res, _) = btree_map_to_ocamlrep(&mut iter, arena, len);
        res
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let mut map = BTreeMap::new();
        btree_map_from_ocamlrep(&mut map, value)?;
        Ok(map)
    }
}

fn btree_map_to_ocamlrep<'a, K: OcamlRep, V: OcamlRep>(
    iter: &mut btree_map::IntoIter<K, V>,
    arena: &Arena<'a>,
    size: usize,
) -> (Value<'a>, usize) {
    if size == 0 {
        return (Value::int(0), 0);
    }
    let (left, left_height) = btree_map_to_ocamlrep(iter, arena, size / 2);
    let (key, val) = iter.next().unwrap();
    let (right, right_height) = btree_map_to_ocamlrep(iter, arena, size - 1 - size / 2);
    let height = std::cmp::max(left_height, right_height) + 1;
    let mut block = arena.block_with_size(5);
    block[0] = left;
    block[1] = arena.add(key);
    block[2] = arena.add(val);
    block[3] = right;
    block[4] = arena.add(height);
    (block.build(), height)
}

fn btree_map_from_ocamlrep<K: OcamlRep + Ord, V: OcamlRep>(
    map: &mut BTreeMap<K, V>,
    value: Value<'_>,
) -> Result<(), FromError> {
    if value.is_immediate() {
        let _ = from::expect_nullary_variant(value, 0)?;
        return Ok(());
    }
    let block = expect_block_with_size_and_tag(value, 5, 0)?;
    btree_map_from_ocamlrep(map, block[0])?;
    let key: K = from::field(block, 1)?;
    let val: V = from::field(block, 2)?;
    map.insert(key, val);
    btree_map_from_ocamlrep(map, block[3])?;
    Ok(())
}

impl<T: OcamlRep + Ord> OcamlRep for BTreeSet<T> {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        if self.is_empty() {
            return Value::int(0);
        }
        let len = self.len();
        let mut iter = self.into_iter();
        let (res, _) = btree_set_to_ocamlrep(&mut iter, arena, len);
        res
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let mut set = BTreeSet::new();
        btree_set_from_ocamlrep(&mut set, value)?;
        Ok(set)
    }
}

fn btree_set_to_ocamlrep<'a, T: OcamlRep>(
    iter: &mut btree_set::IntoIter<T>,
    arena: &Arena<'a>,
    size: usize,
) -> (Value<'a>, usize) {
    if size == 0 {
        return (Value::int(0), 0);
    }
    let (left, left_height) = btree_set_to_ocamlrep(iter, arena, size / 2);
    let val = iter.next().unwrap();
    let (right, right_height) = btree_set_to_ocamlrep(iter, arena, size - 1 - size / 2);
    let height = std::cmp::max(left_height, right_height) + 1;
    let mut block = arena.block_with_size(4);
    block[0] = left;
    block[1] = arena.add(val);
    block[2] = right;
    block[3] = arena.add(height);
    (block.build(), height)
}

fn btree_set_from_ocamlrep<T: OcamlRep + Ord>(
    set: &mut BTreeSet<T>,
    value: Value<'_>,
) -> Result<(), FromError> {
    if value.is_immediate() {
        let _ = from::expect_nullary_variant(value, 0)?;
        return Ok(());
    }
    let block = expect_block_with_size_and_tag(value, 4, 0)?;
    btree_set_from_ocamlrep(set, block[0])?;
    set.insert(from::field(block, 1)?);
    btree_set_from_ocamlrep(set, block[2])?;
    Ok(())
}

impl OcamlRep for PathBuf {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        str_to_ocamlrep(self.to_str().unwrap(), arena)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(PathBuf::from(str_from_ocamlrep(value)?))
    }
}

impl OcamlRep for String {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a> {
        str_to_ocamlrep(self.as_str(), arena)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(String::from(str_from_ocamlrep(value)?))
    }
}

fn str_to_ocamlrep<'a>(s: &str, arena: &Arena<'a>) -> Value<'a> {
    let bytes_in_word = std::mem::size_of::<Value>();
    let blocks_length = 1 + (s.len() / bytes_in_word);
    let padding: usize = bytes_in_word - 1 - (s.len() % bytes_in_word);
    let mut block = arena.block_with_size_and_tag(blocks_length, block::STRING_TAG);

    block[blocks_length - 1] = unsafe { Value::from_bits(padding << ((bytes_in_word - 1) * 8)) };

    let slice: &mut [u8] = unsafe {
        let ptr = block.0.as_ptr().add(1) as *mut u8;
        std::slice::from_raw_parts_mut(ptr, s.len())
    };
    slice.copy_from_slice(s.as_bytes());

    block.build()
}

fn str_from_ocamlrep<'a>(value: Value<'a>) -> Result<&'a str, FromError> {
    let block = from::expect_block(value)?;
    from::expect_block_tag(block, block::STRING_TAG)?;
    let block_size_in_bytes = block.size() * std::mem::size_of::<Value>();
    let slice = unsafe {
        let ptr = block.0.as_ptr().add(1) as *const u8;
        let padding = *ptr.add(block_size_in_bytes - 1);
        let len = block_size_in_bytes - padding as usize - 1;
        std::slice::from_raw_parts(ptr, len)
    };
    Ok(std::str::from_utf8(slice)?)
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

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 2)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        Ok((f0, f1))
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

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 3)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        let f2: T2 = from::field(block, 2)?;
        Ok((f0, f1, f2))
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

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 4)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        let f2: T2 = from::field(block, 2)?;
        let f3: T3 = from::field(block, 3)?;
        Ok((f0, f1, f2, f3))
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

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 5)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        let f2: T2 = from::field(block, 2)?;
        let f3: T3 = from::field(block, 3)?;
        let f4: T4 = from::field(block, 4)?;
        Ok((f0, f1, f2, f3, f4))
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

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 6)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        let f2: T2 = from::field(block, 2)?;
        let f3: T3 = from::field(block, 3)?;
        let f4: T4 = from::field(block, 4)?;
        let f5: T5 = from::field(block, 5)?;
        Ok((f0, f1, f2, f3, f4, f5))
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

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 7)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        let f2: T2 = from::field(block, 2)?;
        let f3: T3 = from::field(block, 3)?;
        let f4: T4 = from::field(block, 4)?;
        let f5: T5 = from::field(block, 5)?;
        let f6: T6 = from::field(block, 6)?;
        Ok((f0, f1, f2, f3, f4, f5, f6))
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

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 8)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        let f2: T2 = from::field(block, 2)?;
        let f3: T3 = from::field(block, 3)?;
        let f4: T4 = from::field(block, 4)?;
        let f5: T5 = from::field(block, 5)?;
        let f6: T6 = from::field(block, 6)?;
        let f7: T7 = from::field(block, 7)?;
        Ok((f0, f1, f2, f3, f4, f5, f6, f7))
    }
}
