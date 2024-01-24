// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::fmt;
use std::hash::Hash;
use std::hash::Hasher;
use std::slice::from_raw_parts;

use bstr::BStr;
use serde::Serialize;
use serde::Serializer;

/// Maybe<T> is similar to C++ `std::option`. It is just like Rust `Option<T>`
/// but has repr(C) for use with with cbindgen.
#[derive(Copy, PartialEq, PartialOrd, Eq, Ord, Debug, Hash, Serialize)]
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

    pub fn into_option(self) -> Option<U> {
        match self {
            Just(t) => Some(t),
            Nothing => None,
        }
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

    pub fn map_or_else<T, D, F>(self, default: D, f: F) -> T
    where
        F: FnOnce(U) -> T,
        D: FnOnce() -> T,
    {
        match self {
            Just(t) => f(t),
            Nothing => default(),
        }
    }

    pub fn unwrap(self) -> U {
        match self {
            Just(t) => t,
            Nothing => panic!("Expected Just(_)"),
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

#[repr(C)]
/// A type to substitute for `&'a[T]`.
// Safety: Must be initialized from an `&[T]`. Use `Slice<'a,
// T>::new()`.
pub struct Slice<'a, T> {
    data: *const T,
    len: usize,
    marker: std::marker::PhantomData<&'a ()>,
}

// A Slice can be cloned even if the underlying data is non-clonable.
impl<'a, T> Clone for Slice<'a, T> {
    fn clone(&self) -> Slice<'a, T> {
        Slice {
            data: self.data,
            len: self.len,
            marker: self.marker,
        }
    }
}

impl<'a, T> Copy for Slice<'a, T> {}

impl<'a, T: serde::Serialize> Serialize for Slice<'a, T> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        self.as_arena_ref().serialize(serializer)
    }
}
// Send+Sync Safety: Slice is no more mutable than T
unsafe impl<'a, T: Sync> Sync for Slice<'a, T> {}
unsafe impl<'a, T: Send> Send for Slice<'a, T> {}

impl<'a, T: fmt::Debug> Slice<'a, T> {
    fn generic_debug_fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str("Slice")?;
        f.debug_list().entries(self.as_ref().iter()).finish()
    }
}

impl<'a, T: fmt::Debug> fmt::Debug for Slice<'a, T> {
    #[cfg(UNSTABLE_DEBUG_SLICE)]
    default fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        self.generic_debug_fmt(f)
    }

    #[cfg(not(UNSTABLE_DEBUG_SLICE))]
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        self.generic_debug_fmt(f)
    }
}

#[cfg(UNSTABLE_DEBUG_SLICE)]
impl<'a> fmt::Debug for Slice<'a, u8> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let mut s = String::new();
        for &ch in self.as_ref() {
            match ch {
                b'\"' => {
                    s.push('\\');
                    s.push('\"');
                }
                b'\\' => {
                    s.push('\\');
                    s.push('\\');
                }
                ch => {
                    if !ch.is_ascii_graphic() {
                        s.push_str(&format!("\\{ch:03o}"));
                    } else {
                        s.push(ch as char);
                    }
                }
            }
        }

        f.write_str("Str(b\"")?;
        f.write_str(&s)?;
        f.write_str("\")")
    }
}

impl<'a, T: 'a> Default for Slice<'a, T> {
    fn default() -> Self {
        Slice::empty()
    }
}
impl<'a, T> AsRef<[T]> for Slice<'a, T> {
    fn as_ref(&self) -> &[T] {
        self.as_arena_ref()
    }
}

impl<'a, T> std::ops::Deref for Slice<'a, T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        self.as_ref()
    }
}

impl<'a, T: 'a> Slice<'a, T> {
    pub const fn new(t: &'a [T]) -> Self {
        Slice {
            data: t.as_ptr(),
            len: t.len(),
            marker: std::marker::PhantomData,
        }
    }

    /// Like `as_ref()` but reflects the fact that the underlying ref has a
    /// lifetime of `arena and not the same lifetime as `self`.
    pub fn as_arena_ref(&self) -> &'a [T] {
        // Safety: Assumes `self` has been constructed via `Slice<'a,
        // T>::new()` from some `&'a [T]` and so the call to
        // `from_raw_parts` is a valid.
        unsafe { std::slice::from_raw_parts(self.data, self.len) }
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
        // Safety: See [Note: `Slice<'a, T>` safety].
        let left = unsafe { from_raw_parts(self.data, self.len) };
        let right = unsafe { from_raw_parts(other.data, other.len) };
        left.eq(right)
    }
}
impl<'a, T: Eq> Eq for Slice<'a, T> {}
impl<'a, T: Hash> Hash for Slice<'a, T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        // Safety: See [Note: `Slice<'a, T>` safety].
        let me = unsafe { from_raw_parts(self.data, self.len) };
        me.hash(state);
    }
}
impl<'a, T: Ord> Ord for Slice<'a, T> {
    fn cmp(&self, other: &Self) -> Ordering {
        // Safety: See [Note: `Slice<'a, T>` safety].
        let left = unsafe { from_raw_parts(self.data, self.len) };
        let right = unsafe { from_raw_parts(other.data, other.len) };
        left.cmp(right)
    }
}
impl<'a, T: PartialOrd> PartialOrd for Slice<'a, T> {
    // Safety: See [Note: Slice<'a, T>` safety].
    fn partial_cmp(&self, other: &Self) -> std::option::Option<Ordering> {
        let left = unsafe { from_raw_parts(self.data, self.len) };
        let right = unsafe { from_raw_parts(other.data, other.len) };
        left.partial_cmp(right)
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

    /// Make a copy of a slice of bytes in an `'a Bump' and return it as a `Str<'a>`.
    pub fn new_slice(alloc: &'a bumpalo::Bump, src: &[u8]) -> Str<'a> {
        Slice::new(alloc.alloc_slice_copy(src))
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

impl std::borrow::Borrow<[u8]> for Str<'_> {
    fn borrow(&self) -> &[u8] {
        self.as_ref()
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
    fn test_03() {
        let alloc: bumpalo::Bump = bumpalo::Bump::new();
        let data = bumpalo::vec![in &alloc; 1, 2, 3].into_bump_slice();
        let s = Slice::new(data);
        let t = Slice::new(data);
        assert_eq!(s, t)
    }
}
