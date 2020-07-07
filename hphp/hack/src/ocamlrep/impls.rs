// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::{Borrow, Cow};
use std::cell::RefCell;
use std::collections::{BTreeMap, BTreeSet};
use std::convert::TryInto;
use std::ffi::{OsStr, OsString};
use std::mem::size_of;
use std::path::{Path, PathBuf};
use std::rc::Rc;

use crate::{block, from};
use crate::{Allocator, FromError, FromOcamlRep, ToOcamlRep, Value};

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

impl ToOcamlRep for () {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int(0)
    }
}

impl FromOcamlRep for () {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        match from::expect_int(value)? {
            0 => Ok(()),
            x => Err(FromError::ExpectedUnit(x)),
        }
    }
}

impl ToOcamlRep for isize {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int(*self)
    }
}

impl FromOcamlRep for isize {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        from::expect_int(value)
    }
}

impl ToOcamlRep for usize {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }
}

impl FromOcamlRep for usize {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl ToOcamlRep for i64 {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }
}

impl FromOcamlRep for i64 {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl ToOcamlRep for u64 {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }
}

impl FromOcamlRep for u64 {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl ToOcamlRep for i32 {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }
}

impl FromOcamlRep for i32 {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl ToOcamlRep for u32 {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).try_into().unwrap())
    }
}

impl FromOcamlRep for u32 {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(from::expect_int(value)?.try_into()?)
    }
}

impl ToOcamlRep for bool {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        Value::int((*self).into())
    }
}

impl FromOcamlRep for bool {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        match from::expect_int(value)? {
            0 => Ok(false),
            1 => Ok(true),
            x => Err(FromError::ExpectedBool(x)),
        }
    }
}

impl ToOcamlRep for char {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        if *self as u32 > 255 {
            panic!("char out of range: {}", self.to_string())
        }
        Value::int(*self as isize)
    }
}

impl FromOcamlRep for char {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let c = from::expect_int(value)?;
        if 0 <= c && c <= 255 {
            Ok(c as u8 as char)
        } else {
            Err(FromError::ExpectedChar(c))
        }
    }
}

impl ToOcamlRep for f64 {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size_and_tag(1, block::DOUBLE_TAG);
        A::set_field(&mut block, 0, unsafe {
            Value::from_bits(self.to_bits() as usize)
        });
        block.build()
    }
}

impl FromOcamlRep for f64 {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = expect_block_with_size_and_tag(value, 1, block::DOUBLE_TAG)?;
        Ok(f64::from_bits(block[0].0 as u64))
    }
}

impl<T: ToOcamlRep> ToOcamlRep for Box<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(&**self)
    }
}

impl<T: FromOcamlRep> FromOcamlRep for Box<T> {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(Box::new(T::from_ocamlrep(value)?))
    }
}

impl<T: ToOcamlRep + Sized> ToOcamlRep for &'_ T {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.memoized(
            *self as *const T as *const usize as usize,
            size_of::<T>(),
            |alloc| (**self).to_ocamlrep(alloc),
        )
    }
}

impl<T: ToOcamlRep + Sized> ToOcamlRep for Rc<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.memoized(
            self.as_ref() as *const T as usize,
            size_of::<T>(),
            |alloc| alloc.add(self.as_ref()),
        )
    }
}

impl<T: FromOcamlRep> FromOcamlRep for Rc<T> {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        // NB: We don't get any sharing this way.
        Ok(Rc::new(T::from_ocamlrep(value)?))
    }
}

impl<T: ToOcamlRep> ToOcamlRep for RefCell<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(1);
        A::set_field(&mut block, 0, alloc.add(&*self.borrow()));
        block.build()
    }
}

impl<T: FromOcamlRep> FromOcamlRep for RefCell<T> {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 1)?;
        let value: T = from::field(block, 0)?;
        Ok(RefCell::new(value))
    }
}

impl<T: ToOcamlRep> ToOcamlRep for Option<T> {
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
}

impl<T: FromOcamlRep> FromOcamlRep for Option<T> {
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

impl<T: ToOcamlRep, E: ToOcamlRep> ToOcamlRep for Result<T, E> {
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
}

impl<T: FromOcamlRep, E: FromOcamlRep> FromOcamlRep for Result<T, E> {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_block(value)?;
        match block.tag() {
            0 => Ok(Ok(from::field(block, 0)?)),
            1 => Ok(Err(from::field(block, 0)?)),
            t => Err(FromError::BlockTagOutOfRange { max: 1, actual: t }),
        }
    }
}

impl<T: ToOcamlRep> ToOcamlRep for [T] {
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
}

impl<T: ToOcamlRep> ToOcamlRep for &'_ [T] {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.memoized(
            self.as_ptr() as usize,
            self.len() * size_of::<T>(),
            |alloc| (**self).to_ocamlrep(alloc),
        )
    }
}

impl<T: ToOcamlRep> ToOcamlRep for Vec<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(self.as_slice())
    }
}

impl<T: FromOcamlRep> FromOcamlRep for Vec<T> {
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

impl<K: ToOcamlRep + Ord, V: ToOcamlRep> ToOcamlRep for BTreeMap<K, V> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        if self.is_empty() {
            return Value::int(0);
        }
        let len = self.len();
        let mut iter = self.iter();
        let (res, _) = sorted_iter_to_ocaml_map(&mut iter, alloc, len);
        res
    }
}

impl<K: FromOcamlRep + Ord, V: FromOcamlRep> FromOcamlRep for BTreeMap<K, V> {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let mut map = BTreeMap::new();
        btree_map_from_ocamlrep(&mut map, value)?;
        Ok(map)
    }
}

/// Given an iterator which emits key-value pairs, build an OCaml Map containing
/// those bindings. The iterator must emit each key only once. The key-value
/// pairs must be emitted in ascending order, sorted by key. The iterator must
/// emit exactly `size` pairs.
pub fn sorted_iter_to_ocaml_map<'a, 'i, A: Allocator, K: ToOcamlRep + 'i, V: ToOcamlRep + 'i>(
    iter: &mut impl Iterator<Item = (&'i K, &'i V)>,
    alloc: &'a A,
    size: usize,
) -> (Value<'a>, usize) {
    if size == 0 {
        return (Value::int(0), 0);
    }
    let (left, left_height) = sorted_iter_to_ocaml_map(iter, alloc, size / 2);
    let (key, val) = iter.next().unwrap();
    let (right, right_height) = sorted_iter_to_ocaml_map(iter, alloc, size - 1 - size / 2);
    let height = std::cmp::max(left_height, right_height) + 1;
    let mut block = alloc.block_with_size(5);
    A::set_field(&mut block, 0, left);
    A::set_field(&mut block, 1, alloc.add(key));
    A::set_field(&mut block, 2, alloc.add(val));
    A::set_field(&mut block, 3, right);
    A::set_field(&mut block, 4, alloc.add(&height));
    (block.build(), height)
}

fn btree_map_from_ocamlrep<K: FromOcamlRep + Ord, V: FromOcamlRep>(
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

impl<T: ToOcamlRep + Ord> ToOcamlRep for BTreeSet<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        if self.is_empty() {
            return Value::int(0);
        }
        let len = self.len();
        let mut iter = self.iter();
        let (res, _) = sorted_iter_to_ocaml_set(&mut iter, alloc, len);
        res
    }
}

impl<T: FromOcamlRep + Ord> FromOcamlRep for BTreeSet<T> {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let mut set = BTreeSet::new();
        btree_set_from_ocamlrep(&mut set, value)?;
        Ok(set)
    }
}

/// Build an OCaml Set containing all items emitted by the given iterator. The
/// iterator must emit each item only once. The items must be emitted in
/// ascending order. The iterator must emit exactly `size` items.
pub fn sorted_iter_to_ocaml_set<'a, 'i, A: Allocator, T: ToOcamlRep + 'i>(
    iter: &mut impl Iterator<Item = &'i T>,
    alloc: &'a A,
    size: usize,
) -> (Value<'a>, usize) {
    if size == 0 {
        return (Value::int(0), 0);
    }
    let (left, left_height) = sorted_iter_to_ocaml_set(iter, alloc, size / 2);
    let val = iter.next().unwrap();
    let (right, right_height) = sorted_iter_to_ocaml_set(iter, alloc, size - 1 - size / 2);
    let height = std::cmp::max(left_height, right_height) + 1;
    let mut block = alloc.block_with_size(4);
    A::set_field(&mut block, 0, left);
    A::set_field(&mut block, 1, alloc.add(val));
    A::set_field(&mut block, 2, right);
    A::set_field(&mut block, 3, alloc.add(&height));
    (block.build(), height)
}

fn btree_set_from_ocamlrep<T: FromOcamlRep + Ord>(
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

impl ToOcamlRep for OsStr {
    // TODO: A Windows implementation would be nice, but what does the OCaml
    // runtime do? If we need Windows support, we'll have to find out.
    #[cfg(unix)]
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        use std::os::unix::ffi::OsStrExt;
        alloc.add(self.as_bytes())
    }
}

impl ToOcamlRep for &'_ OsStr {
    #[cfg(unix)]
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        use std::os::unix::ffi::OsStrExt;
        alloc.add(self.as_bytes())
    }
}

impl ToOcamlRep for OsString {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(self.as_os_str())
    }
}

impl FromOcamlRep for OsString {
    #[cfg(unix)]
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        use std::os::unix::ffi::OsStrExt;
        Ok(OsString::from(std::ffi::OsStr::from_bytes(
            bytes_from_ocamlrep(value)?,
        )))
    }
}

impl ToOcamlRep for Path {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(self.as_os_str())
    }
}

impl ToOcamlRep for &'_ Path {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(self.as_os_str())
    }
}

impl ToOcamlRep for PathBuf {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(self.as_os_str())
    }
}

impl FromOcamlRep for PathBuf {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(PathBuf::from(OsString::from_ocamlrep(value)?))
    }
}

impl ToOcamlRep for String {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(self.as_str())
    }
}

impl FromOcamlRep for String {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(String::from(str_from_ocamlrep(value)?))
    }
}

impl ToOcamlRep for Cow<'_, str> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let s: &str = self.borrow();
        alloc.add(s)
    }
}

impl FromOcamlRep for Cow<'_, str> {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(Cow::Owned(String::from(str_from_ocamlrep(value)?)))
    }
}

impl ToOcamlRep for str {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        str_to_ocamlrep(self, alloc)
    }
}

impl ToOcamlRep for &'_ str {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.memoized(self.as_bytes().as_ptr() as usize, self.len(), |alloc| {
            (**self).to_ocamlrep(alloc)
        })
    }
}

/// Allocate an OCaml string using the given allocator and copy the given string
/// slice into it.
pub fn str_to_ocamlrep<'a, A: Allocator>(s: &str, alloc: &'a A) -> Value<'a> {
    alloc.add(s.as_bytes())
}

/// Given an OCaml string, return a string slice pointing to its contents, if
/// they are valid UTF-8.
pub fn str_from_ocamlrep<'a>(value: Value<'a>) -> Result<&'a str, FromError> {
    Ok(std::str::from_utf8(bytes_from_ocamlrep(value)?)?)
}

impl ToOcamlRep for Vec<u8> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.add(self.as_slice())
    }
}

impl FromOcamlRep for Vec<u8> {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(Vec::from(bytes_from_ocamlrep(value)?))
    }
}

impl ToOcamlRep for [u8] {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        bytes_to_ocamlrep(self, alloc)
    }
}

impl ToOcamlRep for &'_ [u8] {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        alloc.memoized(self.as_ptr() as usize, self.len(), |alloc| {
            (**self).to_ocamlrep(alloc)
        })
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

impl<T0, T1> ToOcamlRep for (T0, T1)
where
    T0: ToOcamlRep,
    T1: ToOcamlRep,
{
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(2);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
        block.build()
    }
}

impl<T0, T1> FromOcamlRep for (T0, T1)
where
    T0: FromOcamlRep,
    T1: FromOcamlRep,
{
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 2)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        Ok((f0, f1))
    }
}

impl<T0, T1, T2> ToOcamlRep for (T0, T1, T2)
where
    T0: ToOcamlRep,
    T1: ToOcamlRep,
    T2: ToOcamlRep,
{
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(3);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
        A::set_field(&mut block, 2, alloc.add(&self.2));
        block.build()
    }
}

impl<T0, T1, T2> FromOcamlRep for (T0, T1, T2)
where
    T0: FromOcamlRep,
    T1: FromOcamlRep,
    T2: FromOcamlRep,
{
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 3)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        let f2: T2 = from::field(block, 2)?;
        Ok((f0, f1, f2))
    }
}

impl<T0, T1, T2, T3> ToOcamlRep for (T0, T1, T2, T3)
where
    T0: ToOcamlRep,
    T1: ToOcamlRep,
    T2: ToOcamlRep,
    T3: ToOcamlRep,
{
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        let mut block = alloc.block_with_size(4);
        A::set_field(&mut block, 0, alloc.add(&self.0));
        A::set_field(&mut block, 1, alloc.add(&self.1));
        A::set_field(&mut block, 2, alloc.add(&self.2));
        A::set_field(&mut block, 3, alloc.add(&self.3));
        block.build()
    }
}

impl<T0, T1, T2, T3> FromOcamlRep for (T0, T1, T2, T3)
where
    T0: FromOcamlRep,
    T1: FromOcamlRep,
    T2: FromOcamlRep,
    T3: FromOcamlRep,
{
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_tuple(value, 4)?;
        let f0: T0 = from::field(block, 0)?;
        let f1: T1 = from::field(block, 1)?;
        let f2: T2 = from::field(block, 2)?;
        let f3: T3 = from::field(block, 3)?;
        Ok((f0, f1, f2, f3))
    }
}

impl<T0, T1, T2, T3, T4> ToOcamlRep for (T0, T1, T2, T3, T4)
where
    T0: ToOcamlRep,
    T1: ToOcamlRep,
    T2: ToOcamlRep,
    T3: ToOcamlRep,
    T4: ToOcamlRep,
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
}

impl<T0, T1, T2, T3, T4> FromOcamlRep for (T0, T1, T2, T3, T4)
where
    T0: FromOcamlRep,
    T1: FromOcamlRep,
    T2: FromOcamlRep,
    T3: FromOcamlRep,
    T4: FromOcamlRep,
{
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

impl<T0, T1, T2, T3, T4, T5> ToOcamlRep for (T0, T1, T2, T3, T4, T5)
where
    T0: ToOcamlRep,
    T1: ToOcamlRep,
    T2: ToOcamlRep,
    T3: ToOcamlRep,
    T4: ToOcamlRep,
    T5: ToOcamlRep,
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
}

impl<T0, T1, T2, T3, T4, T5> FromOcamlRep for (T0, T1, T2, T3, T4, T5)
where
    T0: FromOcamlRep,
    T1: FromOcamlRep,
    T2: FromOcamlRep,
    T3: FromOcamlRep,
    T4: FromOcamlRep,
    T5: FromOcamlRep,
{
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

impl<T0, T1, T2, T3, T4, T5, T6> ToOcamlRep for (T0, T1, T2, T3, T4, T5, T6)
where
    T0: ToOcamlRep,
    T1: ToOcamlRep,
    T2: ToOcamlRep,
    T3: ToOcamlRep,
    T4: ToOcamlRep,
    T5: ToOcamlRep,
    T6: ToOcamlRep,
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
}

impl<T0, T1, T2, T3, T4, T5, T6> FromOcamlRep for (T0, T1, T2, T3, T4, T5, T6)
where
    T0: FromOcamlRep,
    T1: FromOcamlRep,
    T2: FromOcamlRep,
    T3: FromOcamlRep,
    T4: FromOcamlRep,
    T5: FromOcamlRep,
    T6: FromOcamlRep,
{
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

impl<T0, T1, T2, T3, T4, T5, T6, T7> ToOcamlRep for (T0, T1, T2, T3, T4, T5, T6, T7)
where
    T0: ToOcamlRep,
    T1: ToOcamlRep,
    T2: ToOcamlRep,
    T3: ToOcamlRep,
    T4: ToOcamlRep,
    T5: ToOcamlRep,
    T6: ToOcamlRep,
    T7: ToOcamlRep,
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
}

impl<T0, T1, T2, T3, T4, T5, T6, T7> FromOcamlRep for (T0, T1, T2, T3, T4, T5, T6, T7)
where
    T0: FromOcamlRep,
    T1: FromOcamlRep,
    T2: FromOcamlRep,
    T3: FromOcamlRep,
    T4: FromOcamlRep,
    T5: FromOcamlRep,
    T6: FromOcamlRep,
    T7: FromOcamlRep,
{
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
