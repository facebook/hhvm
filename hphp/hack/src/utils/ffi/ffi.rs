// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bstr::BStr;
use std::cmp::Ordering;
use std::hash::{Hash, Hasher};
use std::slice::from_raw_parts;

/// Maybe<T> is similar to C++ `std::option`. It is just like Rust `Option<T>`
/// but has repr(C) for use with with cbindgen.
#[derive(Copy, PartialEq, PartialOrd, Eq, Ord, Debug, Hash)]
#[repr(C)]
pub enum Maybe<T> {
    Just(T),
    Nothing,
}

pub use self::Maybe::*;
impl<T> Default for Maybe<T> {
    #[inline]
    fn default() -> Self {
        Nothing
    }
}

impl<T: Clone> Clone for Maybe<T> {
    #[inline]
    fn clone(&self) -> Self {
        match self {
            Just(x) => Just(x.clone()),
            Nothing => Nothing,
        }
    }
    #[inline]
    fn clone_from(&mut self, source: &Self) {
        match (self, source) {
            (Just(to), Just(from)) => to.clone_from(from),
            (to, from) => *to = from.clone(),
        }
    }
}

impl<U> Maybe<U> {
    #[inline]
    pub const fn as_ref(&self) -> Maybe<&U> {
        match self {
            Just(x) => Just(x),
            Nothing => Nothing,
        }
    }

    #[inline]
    pub fn as_mut(&mut self) -> Maybe<&mut U> {
        match self {
            Just(x) => Just(x),
            Nothing => Nothing,
        }
    }

    #[inline]
    pub const fn is_just(&self) -> bool {
        matches!(self, Just(_))
    }

    #[inline]
    pub const fn is_nothing(&self) -> bool {
        matches!(self, Nothing)
    }

    #[inline]
    pub fn map<T, F: FnOnce(U) -> T>(self, f: F) -> Maybe<T> {
        match self {
            Just(x) => Just(f(x)),
            Nothing => Nothing,
        }
    }

    #[inline]
    pub fn map_or<T, F: FnOnce(U) -> T>(self, default: T, f: F) -> T {
        match self {
            Just(t) => f(t),
            Nothing => default,
        }
    }

    pub fn unwrap_or(self, default: U) -> U {
        match self {
            Just(t) => t,
            Nothing => default,
        }
    }
}

impl<U: Default> Maybe<U> {
    pub fn unwrap_or_default(self) -> U {
        match self {
            Just(t) => t,
            Nothing => Default::default(),
        }
    }
}

impl<U> std::convert::From<Option<U>> for Maybe<U> {
    fn from(o: Option<U>) -> Self {
        match o {
            Some(x) => Just(x),
            None => Nothing,
        }
    }
}
impl<U> std::convert::From<Maybe<U>> for Option<U> {
    fn from(o: Maybe<U>) -> Self {
        match o {
            Just(x) => Some(x),
            Nothing => None,
        }
    }
}

#[derive(Clone, Copy, PartialEq, PartialOrd, Eq, Ord, Debug, Hash)]
#[repr(C)]
/// A tuple of two elements.
pub struct Pair<U, V>(pub U, pub V);
impl<U, V> std::convert::From<(U, V)> for Pair<U, V> {
    fn from((u, v): (U, V)) -> Self {
        Pair(u, v)
    }
}

#[derive(Clone, Copy, PartialEq, PartialOrd, Eq, Ord, Debug, Hash)]
#[repr(C)]
/// A tuple of three elements.
pub struct Triple<U, V, W>(pub U, pub V, pub W);
impl<U, V, W> std::convert::From<(U, V, W)> for Triple<U, V, W> {
    fn from((u, v, w): (U, V, W)) -> Self {
        Triple(u, v, w)
    }
}

#[derive(Clone, Copy, PartialEq, PartialOrd, Eq, Ord, Debug, Hash)]
#[repr(C)]
/// A tuple of four elements.
pub struct Quadruple<U, V, W, X>(pub U, pub V, pub W, pub X);
impl<U, V, W, X> std::convert::From<(U, V, W, X)> for Quadruple<U, V, W, X> {
    fn from((u, v, w, x): (U, V, W, X)) -> Self {
        Quadruple(u, v, w, x)
    }
}
// [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>` safety]
// -------------------------------------------------------
// If we assume construction via the factory functions
// `BumpSliceMut<'a, T>::new()` and `Slice<'a, T>::new()` then we know
// that the contained members are safe to use with
// `from_raw_parts_mut`/`from_raw_parts`. We rely on this in the
// implementation of traits such as `Eq` and friends.

#[derive(Clone, Copy, Debug)]
#[repr(C)]
/// A type to substitute for `&'a[T]`.
// Safety: Must be initialized from an `&[T]`. Use `Slice<'a,
// T>::new()`.
pub struct Slice<'a, T> {
    data: *const T,
    len: usize,
    marker: std::marker::PhantomData<&'a ()>,
}
impl<'a, T: 'a> Default for Slice<'a, T> {
    fn default() -> Self {
        Slice::empty()
    }
}
impl<'a, T> AsRef<[T]> for Slice<'a, T> {
    fn as_ref(&self) -> &[T] {
        // Safety: Assumes `self` has been constructed via `Slice<'a,
        // T>::new()` from some `&'a [T]` and so the call to
        // `from_raw_parts` is a valid.
        unsafe { std::slice::from_raw_parts(self.data, self.len) }
    }
}

impl<'a, T> std::ops::Deref for Slice<'a, T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        self.as_ref()
    }
}

impl<'a, T: 'a> Slice<'a, T> {
    pub fn new(t: &'a [T]) -> Self {
        Slice {
            data: t.as_ptr(),
            len: t.len(),
            marker: std::marker::PhantomData,
        }
    }

    pub fn fill_iter<I>(alloc: &'a bumpalo::Bump, iter: I) -> Slice<'a, T>
    where
        I: IntoIterator<Item = T>,
        I::IntoIter: ExactSizeIterator,
    {
        Slice::new(alloc.alloc_slice_fill_iter(iter))
    }

    pub fn empty() -> Self {
        Slice {
            data: std::ptr::NonNull::dangling().as_ptr(),
            len: 0,
            marker: std::marker::PhantomData,
        }
    }

    pub fn is_empty(&self) -> bool {
        self.len == 0
    }

    pub fn len(&self) -> usize {
        self.len
    }
}

impl<'a, T: 'a> Slice<'a, T> {
    pub fn from_vec(alloc: &'a bumpalo::Bump, xs: Vec<T>) -> Self {
        alloc.alloc_slice_fill_iter(xs.into_iter()).into()
    }
}

impl<'a, T: 'a> IntoIterator for &'a Slice<'a, T> {
    type Item = &'a T;
    type IntoIter = std::slice::Iter<'a, T>;

    fn into_iter(self) -> std::slice::Iter<'a, T> {
        self.iter()
    }
}

impl<'a, T> std::convert::From<&'a [T]> for Slice<'a, T> {
    fn from(x: &'a [T]) -> Self {
        Self::new(x)
    }
}

impl<'a, T> std::convert::From<&'a mut [T]> for Slice<'a, T> {
    fn from(x: &'a mut [T]) -> Self {
        Self::new(x)
    }
}

impl<'a, T: PartialEq> PartialEq for Slice<'a, T> {
    fn eq(&self, other: &Self) -> bool {
        // Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
        // safety].
        unsafe {
            let left = from_raw_parts(self.data, self.len);
            let right = from_raw_parts(other.data, other.len);
            left.eq(right)
        }
    }
}
impl<'a, T: Eq> Eq for Slice<'a, T> {}
impl<'a, T: Hash> Hash for Slice<'a, T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        // Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
        // safety].
        unsafe {
            let me = from_raw_parts(self.data, self.len);
            me.hash(state);
        }
    }
}
impl<'a, T: Ord> Ord for Slice<'a, T> {
    fn cmp(&self, other: &Self) -> Ordering {
        // Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
        // safety].
        unsafe {
            let left = from_raw_parts(self.data, self.len);
            let right = from_raw_parts(other.data, other.len);
            left.cmp(right)
        }
    }
}
impl<'a, T: PartialOrd> PartialOrd for Slice<'a, T> {
    // Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
    // safety].
    fn partial_cmp(&self, other: &Self) -> std::option::Option<Ordering> {
        unsafe {
            let left = from_raw_parts(self.data, self.len);
            let right = from_raw_parts(other.data, other.len);
            left.partial_cmp(right)
        }
    }
}

/// An alias for a type that substitutes for `&'str`.
pub type Str<'a> = Slice<'a, u8>;
// C++:
// std::string slice_to_string(Str s) {
//    return std::string{s.data, s.data + s.len};
// }
impl<'a> Str<'a> {
    /// Make a copy of a `&str` in an `'a Bump` and return it as a `Str<'a>`.
    //  Don't use this if you have an `&'a str` already, prefer
    //  `Str::from` in that case and avoid a copy.
    pub fn new_str(alloc: &'a bumpalo::Bump, src: &str) -> Str<'a> {
        Slice::new(alloc.alloc_str(src.as_ref()).as_bytes())
    }

    /// Cast a `Str<'a>` back into a `&'a str`.
    pub fn unsafe_as_str(&self) -> &'a str {
        // Safety: Assumes `self` has been constructed via `Slice<'a,
        // T>::new()` from some `&'a str` and so the calls to
        // `from_raw_parts` and `from_utf8_unchecked` are valid.
        unsafe { std::str::from_utf8_unchecked(std::slice::from_raw_parts(self.data, self.len)) }
    }

    /// Cast a `Str<'a>` back into a `&'a BStr`.
    pub fn as_bstr(&self) -> &'a BStr {
        // Safety: Assumes `self` has been constructed via `Slice<'a,
        // T>::new()` from some `&'a BStr` and so the call to
        // `from_raw_parts` is valid.
        unsafe { std::slice::from_raw_parts(self.data, self.len).into() }
    }
}

impl<'a> write_bytes::DisplayBytes for Str<'a> {
    fn fmt(&self, f: &mut write_bytes::BytesFormatter<'_>) -> std::io::Result<()> {
        use std::io::Write;
        f.write_all(self.as_ref())
    }
}

impl<'a> std::convert::From<&'a String> for Slice<'a, u8> {
    fn from(s: &'a String) -> Self {
        Self::new(s.as_bytes())
    }
}

impl<'a> std::convert::From<&'a str> for Slice<'a, u8> {
    fn from(s: &'a str) -> Self {
        Self::new(s.as_bytes())
    }
}

impl<'a> std::convert::From<&'a mut str> for Slice<'a, u8> {
    fn from(s: &'a mut str) -> Self {
        Self::new(s.as_bytes())
    }
}

#[derive(Debug)]
#[repr(C)]
/// A type for an arena backed `&'a mut[T]`. Similar to `Slice<'a, T>`
/// but with mutable contents and an allocator reference (enabling
/// `Clone` support).
// Safety: Initialize from an `&'arena [T]` where the memory is owned
// by `alloc`. Use `BumpSliceMut<'a, T>::new()`.
pub struct BumpSliceMut<'a, T> {
    data: *mut T,
    len: usize,
    alloc: usize, // *const bumpalo::Bump,
    marker: std::marker::PhantomData<&'a ()>,
}
impl<'a, T> BumpSliceMut<'a, T> {
    // Safety: `t` must be owned by `alloc`.
    pub fn new(alloc: &'a bumpalo::Bump, t: &'a mut [T]) -> Self {
        BumpSliceMut {
            data: t.as_mut_ptr(),
            len: t.len(),
            alloc: alloc as *const bumpalo::Bump as usize,
            marker: std::marker::PhantomData,
        }
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.as_ref().is_empty()
    }

    #[inline]
    pub fn len(&self) -> usize {
        self.as_ref().len()
    }

    #[inline]
    pub fn iter(&self) -> std::slice::Iter<'_, T> {
        self.as_ref().iter()
    }

    #[inline]
    pub fn iter_mut(&mut self) -> std::slice::IterMut<'_, T> {
        self.as_mut().iter_mut()
    }

    #[inline]
    pub fn get(&self, index: usize) -> Option<&T> {
        self.as_ref().get(index)
    }
}
impl<'a, T> std::ops::Index<usize> for BumpSliceMut<'a, T> {
    type Output = T;

    #[inline]
    fn index(&self, i: usize) -> &T {
        &self.as_ref()[i]
    }
}
impl<'a, T> std::ops::IndexMut<usize> for BumpSliceMut<'a, T> {
    #[inline]
    fn index_mut(&mut self, i: usize) -> &mut T {
        &mut self.as_mut()[i]
    }
}
impl<'a, T> AsRef<[T]> for BumpSliceMut<'a, T> {
    fn as_ref<'r>(&'r self) -> &'r [T] {
        // Safety:
        // - We assume 'a: 'r
        // - Assumes `self` has been constructed via
        //   `BumpSliceMut<'a, T>::new()` from some `&'a[T]` and so the
        //   call to `from_raw_parts` is a valid.
        unsafe { std::slice::from_raw_parts(self.data, self.len) }
    }
}
impl<'a, T> AsMut<[T]> for BumpSliceMut<'a, T> {
    fn as_mut<'r>(&'r mut self) -> &'r mut [T] {
        // Safety:
        // - We assume 'a: 'r
        // - Assumes `self` has been constructed via
        //   `BumpSliceMut<'a, T>::new()` from some `&'a[T]` and so the
        //   call to `from_raw_parts_mut` is a valid.
        unsafe { std::slice::from_raw_parts_mut(self.data, self.len) }
    }
}
impl<'arena, T: 'arena + Clone> Clone for BumpSliceMut<'arena, T> {
    fn clone(&self) -> Self {
        // Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
        // safety].
        let alloc: &'arena bumpalo::Bump =
            unsafe { (self.alloc as *const bumpalo::Bump).as_ref().unwrap() };
        BumpSliceMut::new(alloc, alloc.alloc_slice_clone(self.as_ref()))
    }
}

/// A ReprC view of Vec<u8>. The underlying Vec is owned by this object.
#[repr(C)]
pub struct Bytes {
    pub data: *mut u8,
    pub len: usize,
    pub cap: usize,
}

impl From<Vec<u8>> for Bytes {
    fn from(bytes: Vec<u8>) -> Self {
        let mut leaked_bytes = std::mem::ManuallyDrop::new(bytes);
        Self {
            data: leaked_bytes.as_mut_ptr(),
            len: leaked_bytes.len(),
            cap: leaked_bytes.capacity(),
        }
    }
}

impl Bytes {
    pub unsafe fn as_slice(&self) -> &[u8] {
        std::slice::from_raw_parts(self.data, self.len)
    }
}

impl std::ops::Drop for Bytes {
    fn drop(&mut self) {
        let _ = unsafe { Vec::from_raw_parts(self.data, self.len, self.cap) };
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_00() {
        let foo = Just(2);
        match foo {
            Just(i) => assert_eq!(i, 2),
            Nothing => {}
        }
    }

    #[test]
    fn test_01() {
        let Pair(u, v) = Pair::from((2, "foo"));
        assert_eq!(u, 2);
        assert_eq!(v, "foo")
    }

    #[test]
    fn test_02() {
        let alloc: bumpalo::Bump = bumpalo::Bump::new();
        let mut buf = bumpalo::vec![in &alloc; 1, 2, 3];
        let _s = BumpSliceMut::new(&alloc, buf.as_mut_slice());
    }

    #[test]
    fn test_03() {
        let alloc: bumpalo::Bump = bumpalo::Bump::new();
        let data = bumpalo::vec![in &alloc; 1, 2, 3].into_bump_slice();
        let s = Slice::new(data);
        let t = Slice::new(data);
        assert_eq!(s, t)
    }

    #[test]
    fn test_04() {
        let Triple(u, v, w) = Triple::from((2, "foo", 1.0e-2));
        assert_eq!(u, 2);
        assert_eq!(v, "foo");
        assert_eq!(w, 1.0e-2);
    }
}
