// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::hash::{Hash, Hasher};
use std::slice::from_raw_parts;
use std::slice::from_raw_parts_mut;

#[derive(Copy, PartialEq, PartialOrd, Eq, Ord, Debug, Hash)]
#[repr(C)]
/// Like `std::option`.
pub enum Maybe<T> {
    Just(T),
    Nothing,
}
pub use self::Maybe::*;
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

#[derive(Clone, Copy, PartialEq, PartialOrd, Eq, Ord, Debug, Hash)]
#[repr(C)]
/// A tuple of two elements.
pub struct Pair<U, V>(pub U, pub V);
impl<U, V> std::convert::From<(U, V)> for Pair<U, V> {
    fn from((u, v): (U, V)) -> Self {
        Pair(u, v)
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
    pub data: *const T,
    pub len: usize,
    pub marker: std::marker::PhantomData<&'a ()>,
}
impl<'a, T> AsRef<[T]> for Slice<'a, T> {
    fn as_ref(&self) -> &[T] {
        //Safety: Assumes `self` has been constructed via `Slice<'a,
        // T>::new()` from some `&'a[T]` and so the call to
        // `from_raw_parts` is a valid.
        unsafe { std::slice::from_raw_parts(self.data, self.len) }
    }
}
impl<'a, T> Slice<'a, T> {
    pub fn new(t: &'a [T]) -> Self {
        Slice {
            data: t.as_ptr(),
            len: t.len(),
            marker: std::marker::PhantomData,
        }
    }
}
impl<'a, T: PartialEq> PartialEq for Slice<'a, T> {
    fn eq(&self, other: &Self) -> bool {
        //Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
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
        //Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
        // safety].
        unsafe {
            let me = from_raw_parts(self.data, self.len);
            me.hash(state);
        }
    }
}
impl<'a, T: Ord> Ord for Slice<'a, T> {
    fn cmp(&self, other: &Self) -> Ordering {
        //Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
        // safety].
        unsafe {
            let left = from_raw_parts(self.data, self.len);
            let right = from_raw_parts(other.data, other.len);
            left.cmp(right)
        }
    }
}
impl<'a, T: PartialOrd> PartialOrd for Slice<'a, T> {
    //Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
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
    /// Cast a `Str<'a>` back into a `&'a str`.
    pub fn as_str(&self) -> &'a str {
        //Safety: Assumes `self` has been constructed via `Slice<'a,
        // T>::new()` from some `&'a str` and so the calls to
        // `from_raw_parts` and `from_utf8_unchecked` are valid.
        unsafe { std::str::from_utf8_unchecked(std::slice::from_raw_parts(self.data, self.len)) }
    }
}
impl<'a> AsRef<str> for Str<'a> {
    fn as_ref(&self) -> &str {
        self.as_str()
    }
}

#[derive(Debug)]
#[repr(C)]
/// A type for an arena backed `&'a mut[T]`. Similar to `Slice<'a, T>`
/// but with mutable contents and an allocator reference (enabling
/// `Clone` support).
// Safety: Initialize from an `&'arena [T]` where the memory is owned
// by `alloc`. Use `BumpSliceMut<'a, T>::new()`.
pub struct BumpSliceMut<'arena, T> {
    pub data: *mut T,
    pub len: usize,
    pub alloc: &'arena bumpalo::Bump,
}
impl<'a, T> BumpSliceMut<'a, T> {
    // Safety: `t` must be owned by `alloc`.
    pub fn new(alloc: &'a bumpalo::Bump, t: &'a mut [T]) -> Self {
        BumpSliceMut {
            data: t.as_mut_ptr(),
            len: t.len(),
            alloc,
        }
    }
}
impl<'arena, T: Clone> Clone for BumpSliceMut<'arena, T> {
    fn clone(&self) -> Self {
        //Safety: See [Note: `BumpSliceMut<'a, T>` and `Slice<'a, T>`
        // safety].
        unsafe {
            let alloc: &'arena bumpalo::Bump = self.alloc;
            let mut vec = bumpalo::collections::Vec::from_iter_in(
                from_raw_parts_mut(self.data, self.len).iter().cloned(),
                alloc,
            );
            let slice = vec.as_mut_slice();
            BumpSliceMut {
                data: slice.as_mut_ptr(),
                len: slice.len(),
                alloc: self.alloc,
            }
        }
    }
}

#[repr(C)]
pub struct Bytes {
    pub data: *mut u8,
    pub len: usize,
    pub cap: usize,
}

impl std::convert::From<Vec<u8>> for Bytes {
    fn from(bytes: Vec<u8>) -> Self {
        let mut leaked_bytes = std::mem::ManuallyDrop::new(bytes);
        Self {
            data: leaked_bytes.as_mut_ptr(),
            len: leaked_bytes.len(),
            cap: leaked_bytes.capacity(),
        }
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
}

// For cbindgen
#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn ffi_07<'arena>(_: Str<'arena>) {
    unimplemented!()
}
