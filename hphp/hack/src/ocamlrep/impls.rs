// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::{Borrow, Cow};
use std::cell::RefCell;
use std::collections::{btree_map, btree_set, BTreeMap, BTreeSet};
use std::convert::TryInto;
use std::ffi::OsString;
use std::path::PathBuf;
use std::rc::Rc;

use crate::{block, from};
use crate::{Allocator, FromError, OcamlRep, Value};

const WORD_SIZE: usize = std::mem::size_of::<Value<'_>>();

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
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
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
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int(*self)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        from::expect_int(value)
    }
}

impl OcamlRep for usize {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for i64 {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for u64 {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for i32 {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for u32 {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl OcamlRep for bool {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).into())
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
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        if *self as u32 > 255 {
            panic!("char out of range: {}", self.to_string())
        }
        Value::int(*self as isize)
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
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size_and_tag(1, block::DOUBLE_TAG);
        A::set_field(&mut block, 0, unsafe {
            Value::from_bits(self.to_bits() as usize)
        });
        block.build()
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = expect_block_with_size_and_tag(value, 1, block::DOUBLE_TAG)?;
        Ok(f64::from_bits(block[0].0 as u64))
    }
}

impl<T: OcamlRep> OcamlRep for Box<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(&**self)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(Box::new(T::from_ocamlrep(value)?))
    }
}

impl<T: OcamlRep> OcamlRep for Rc<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(self.as_ref())
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        // NB: We don't get any sharing this way.
        Ok(Rc::new(T::from_ocamlrep(value)?))
    }
}

impl<T: OcamlRep> OcamlRep for RefCell<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(1);
        A::set_field(&mut block, 0, alloc.add(&*self.borrow()));
        block.build()
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 1)?;
        let value: T = from::field(block, 0)?;
        Ok(RefCell::new(value))
    }
}

impl<T: OcamlRep> OcamlRep for Option<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        match self {
            None => Value::int(0),
            Some(val) => {
                let mut block = alloc.block_with_size(1);
                A::set_field(&mut block, 0, alloc.add(val));
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

impl<T: OcamlRep, E: OcamlRep> OcamlRep for Result<T, E> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        match self {
            Ok(val) => {
                let mut block = alloc.block_with_size(1);
                A::set_field(&mut block, 0, alloc.add(val));
                block.build()
            }
            Err(val) => {
                let mut block = alloc.block_with_size_and_tag(1, 1);
                A::set_field(&mut block, 0, alloc.add(val));
                block.build()
            }
        }
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_block(value)?;
        match block.tag() {
            0 => Ok(Ok(from::field(block, 0)?)),
            1 => Ok(Err(from::field(block, 0)?)),
            t => Err(FromError::BlockTagOutOfRange { max: 1, actual: t }),
        }
    }
}

impl<T: OcamlRep> OcamlRep for Vec<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut hd = alloc.add(&());
        for val in self.iter().rev() {
            let mut block = alloc.block_with_size(2);
            A::set_field(&mut block, 0, alloc.add(val));
            A::set_field(&mut block, 1, hd);
            hd = block.build();
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
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        if self.is_empty() {
            return Value::int(0);
        }
        let len = self.len();
        let mut iter = self.iter();
        let (res, _) = btree_map_to_ocamlrep(&mut iter, alloc, len);
        res
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let mut map = BTreeMap::new();
        btree_map_from_ocamlrep(&mut map, value)?;
        Ok(map)
    }
}

fn btree_map_to_ocamlrep<'a, A: Allocator, K: OcamlRep, V: OcamlRep>(
    iter: &mut btree_map::Iter<K, V>,
    alloc: &'a A,
    size: usize,
) -> (Value<'a>, usize) {
    if size == 0 {
        return (Value::int(0), 0);
    }
    let (left, left_height) = btree_map_to_ocamlrep(iter, alloc, size / 2);
    let (key, val) = iter.next().unwrap();
    let (right, right_height) = btree_map_to_ocamlrep(iter, alloc, size - 1 - size / 2);
    let height = std::cmp::max(left_height, right_height) + 1;
    let mut block = alloc.block_with_size(5);
    A::set_field(&mut block, 0, left);
    A::set_field(&mut block, 1, alloc.add(key));
    A::set_field(&mut block, 2, alloc.add(val));
    A::set_field(&mut block, 3, right);
    A::set_field(&mut block, 4, alloc.add(&height));
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
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        if self.is_empty() {
            return Value::int(0);
        }
        let len = self.len();
        let mut iter = self.iter();
        let (res, _) = btree_set_to_ocamlrep(&mut iter, alloc, len);
        res
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let mut set = BTreeSet::new();
        btree_set_from_ocamlrep(&mut set, value)?;
        Ok(set)
    }
}

fn btree_set_to_ocamlrep<'a, A: Allocator, T: OcamlRep>(
    iter: &mut btree_set::Iter<T>,
    alloc: &'a A,
    size: usize,
) -> (Value<'a>, usize) {
    if size == 0 {
        return (Value::int(0), 0);
    }
    let (left, left_height) = btree_set_to_ocamlrep(iter, alloc, size / 2);
    let val = iter.next().unwrap();
    let (right, right_height) = btree_set_to_ocamlrep(iter, alloc, size - 1 - size / 2);
    let height = std::cmp::max(left_height, right_height) + 1;
    let mut block = alloc.block_with_size(4);
    A::set_field(&mut block, 0, left);
    A::set_field(&mut block, 1, alloc.add(val));
    A::set_field(&mut block, 2, right);
    A::set_field(&mut block, 3, alloc.add(&height));
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

impl OcamlRep for OsString {
    #[cfg(unix)]
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        use std::os::unix::ffi::OsStrExt;
        bytes_to_ocamlrep(self.as_bytes(), alloc)
    }

    #[cfg(unix)]
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        use std::os::unix::ffi::OsStrExt;
        Ok(OsString::from(std::ffi::OsStr::from_bytes(
            bytes_from_ocamlrep(value)?,
        )))
    }

    // TODO: A Windows implementation would be nice, but what does the OCaml
    // runtime do? If we need Windows support, we'll have to find out.
}

impl OcamlRep for PathBuf {
    #[cfg(unix)]
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        use std::os::unix::ffi::OsStrExt;
        bytes_to_ocamlrep(self.as_os_str().as_bytes(), alloc)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(PathBuf::from(OsString::from_ocamlrep(value)?))
    }
}

impl OcamlRep for String {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        str_to_ocamlrep(self.as_str(), alloc)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(String::from(str_from_ocamlrep(value)?))
    }
}

impl OcamlRep for Cow<'_, str> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        str_to_ocamlrep(self.borrow(), alloc)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(Cow::Owned(String::from(str_from_ocamlrep(value)?)))
    }
}

/// Allocate an OCaml string using the given allocator and copy the given string
/// slice into it.
pub fn str_to_ocamlrep<'a, A: Allocator>(s: &str, alloc: &'a A) -> Value<'a> {
    bytes_to_ocamlrep(s.as_bytes(), alloc)
}

/// Given an OCaml string, return a string slice pointing to its contents, if
/// they are valid UTF-8.
pub fn str_from_ocamlrep<'a>(value: Value<'a>) -> Result<&'a str, FromError> {
    Ok(std::str::from_utf8(bytes_from_ocamlrep(value)?)?)
}

impl OcamlRep for Vec<u8> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        bytes_to_ocamlrep(self, alloc)
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(Vec::from(bytes_from_ocamlrep(value)?))
    }
}

/// Allocate an OCaml string using the given allocator and copy the given byte
/// slice into it.
pub fn bytes_to_ocamlrep<'a, A: Allocator>(s: &[u8], alloc: &'a A) -> Value<'a> {
    let words = (s.len() + 1 /*null-ending*/ + (WORD_SIZE - 1)/*rounding*/) / WORD_SIZE;
    let length = words * WORD_SIZE;
    let mut block = alloc.block_with_size_and_tag(words, block::STRING_TAG);
    let block_contents_as_slice: &mut [u8] = unsafe {
        let block = block.as_mut_ptr();
        *block.add(words - 1) = Value::from_bits(0);
        let block_bytes = block as *mut u8;
        *block_bytes.add(length - 1) = (length - s.len() - 1) as u8;
        std::slice::from_raw_parts_mut(block_bytes, s.len())
    };
    block_contents_as_slice.copy_from_slice(s);
    block.build()
}

/// Given an OCaml string, return a byte slice pointing to its contents.
pub fn bytes_from_ocamlrep<'a>(value: Value<'a>) -> Result<&'a [u8], FromError> {
    let block = from::expect_block(value)?;
    from::expect_block_tag(block, block::STRING_TAG)?;
    let block_size_in_bytes = block.size() * std::mem::size_of::<Value>();
    let slice = unsafe {
        let ptr = block.0.as_ptr().add(1) as *const u8;
        let padding = *ptr.add(block_size_in_bytes - 1);
        let len = block_size_in_bytes - padding as usize - 1;
        std::slice::from_raw_parts(ptr, len)
    };
    Ok(slice)
}

impl<T0, T1> OcamlRep for (T0, T1)
where
    T0: OcamlRep,
    T1: OcamlRep,
{
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(2);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
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
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(3);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
        A::set_field(&mut block, 2, alloc.add(&self.2));
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
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(4);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
        A::set_field(&mut block, 2, alloc.add(&self.2));
        A::set_field(&mut block, 3, alloc.add(&self.3));
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
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(5);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
        A::set_field(&mut block, 2, alloc.add(&self.2));
        A::set_field(&mut block, 3, alloc.add(&self.3));
        A::set_field(&mut block, 4, alloc.add(&self.4));
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
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(6);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
        A::set_field(&mut block, 2, alloc.add(&self.2));
        A::set_field(&mut block, 3, alloc.add(&self.3));
        A::set_field(&mut block, 4, alloc.add(&self.4));
        A::set_field(&mut block, 5, alloc.add(&self.5));
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
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(7);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
        A::set_field(&mut block, 2, alloc.add(&self.2));
        A::set_field(&mut block, 3, alloc.add(&self.3));
        A::set_field(&mut block, 4, alloc.add(&self.4));
        A::set_field(&mut block, 5, alloc.add(&self.5));
        A::set_field(&mut block, 6, alloc.add(&self.6));
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
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(8);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
        A::set_field(&mut block, 2, alloc.add(&self.2));
        A::set_field(&mut block, 3, alloc.add(&self.3));
        A::set_field(&mut block, 4, alloc.add(&self.4));
        A::set_field(&mut block, 5, alloc.add(&self.5));
        A::set_field(&mut block, 6, alloc.add(&self.6));
        A::set_field(&mut block, 7, alloc.add(&self.7));
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
