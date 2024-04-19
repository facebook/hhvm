// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::Hash;
use std::hash::Hasher;
use std::ptr::NonNull;

use serde::Serialize;
use serde::Serializer;

/// A ReprC view of Vec<T>. The underlying Vec is owned by this object.
#[repr(C)]
pub struct Vector<T> {
    data: NonNull<T>,
    len: usize,
    cap: usize,
}

// Sync Safety: Vector<T> is safe to access from other threads if T is
unsafe impl<T: Sync> Sync for Vector<T> {}

// Sync Safety: Vector<T> is safe to move to other threads if T is
unsafe impl<T: Send> Send for Vector<T> {}

impl<T> From<Vec<T>> for Vector<T> {
    fn from(v: Vec<T>) -> Self {
        let mut leaked = std::mem::ManuallyDrop::new(v);
        Self {
            data: NonNull::new(leaked.as_mut_ptr()).unwrap(),
            len: leaked.len(),
            cap: leaked.capacity(),
        }
    }
}

impl<T> From<Vector<T>> for Vec<T> {
    fn from(v: Vector<T>) -> Self {
        // Safety: data, len, and cap haven't been modified since constructing self
        let v = std::mem::ManuallyDrop::new(v);
        unsafe { Vec::from_raw_parts(v.data.as_ptr(), v.len, v.cap) }
    }
}

impl<T> Vector<T> {
    pub fn as_slice(&self) -> &[T] {
        // Safety: data and len haven't been modified since constructing self
        unsafe { std::slice::from_raw_parts(self.data.as_ptr(), self.len) }
    }

    pub fn as_mut_slice(&mut self) -> &mut [T] {
        // Safety: data and len haven't been modified since constructing self
        unsafe { std::slice::from_raw_parts_mut(self.data.as_ptr(), self.len) }
    }

    pub fn push(&mut self, x: T) {
        let mut v = Vec::from(std::mem::take(self));
        v.push(x);
        *self = Self::from(v);
    }

    pub fn insert(&mut self, index: usize, x: T) {
        let mut v = Vec::from(std::mem::take(self));
        v.insert(index, x);
        *self = Self::from(v);
    }
}

impl<T> std::ops::Drop for Vector<T> {
    fn drop(&mut self) {
        // Safety: data, len, and cap haven't been modified since constructing self
        let _ = unsafe { Vec::from_raw_parts(self.data.as_ptr(), self.len, self.cap) };
    }
}

impl<T: std::fmt::Debug> std::fmt::Debug for Vector<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        std::fmt::Debug::fmt(self.as_slice(), f)
    }
}

impl<T: Serialize> Serialize for Vector<T> {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        self.as_slice().serialize(serializer)
    }
}

impl<T> Default for Vector<T> {
    fn default() -> Self {
        Self::from(Vec::default())
    }
}

impl<T> AsRef<[T]> for Vector<T> {
    fn as_ref(&self) -> &[T] {
        self.as_slice()
    }
}

impl<T: Clone> Clone for Vector<T> {
    fn clone(&self) -> Self {
        self.as_slice().to_vec().into()
    }
}

impl<T: Eq> Eq for Vector<T> {}
impl<T: PartialEq> PartialEq for Vector<T> {
    fn eq(&self, other: &Self) -> bool {
        self.as_slice().eq(other.as_slice())
    }
}

impl<T: Hash> Hash for Vector<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.as_slice().hash(state);
    }
}

impl<T> std::ops::Deref for Vector<T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        self.as_slice()
    }
}

impl<T> std::ops::DerefMut for Vector<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.as_mut_slice()
    }
}

impl<'a, T> IntoIterator for &'a Vector<T> {
    type Item = &'a T;
    type IntoIter = std::slice::Iter<'a, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.as_slice().iter()
    }
}

impl<T> IntoIterator for Vector<T> {
    type Item = T;
    type IntoIter = std::vec::IntoIter<T>;

    fn into_iter(self) -> Self::IntoIter {
        Vec::from(self).into_iter()
    }
}

impl<T> std::iter::FromIterator<T> for Vector<T> {
    fn from_iter<I>(iter: I) -> Self
    where
        I: IntoIterator<Item = T>,
    {
        Self::from(Vec::from_iter(iter))
    }
}
