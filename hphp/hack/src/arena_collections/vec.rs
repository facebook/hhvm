// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bumpalo::collections::{vec::IntoIter, Vec as BVec};
use bumpalo::Bump;
use std::ops::Deref;
use std::slice;

#[derive(Debug, Eq, Ord, PartialEq, PartialOrd)]
pub struct Vec<'a, T: 'a>(BVec<'a, T>);

#[macro_export]
macro_rules! pvec {
    (in $arena: expr) => {
        Vec::new_in($arena);
    };
    (in $arena: expr; $($x:expr),*) => {{
        let mut v = bumpalo::collections::Vec::new_in($arena);
        $(v.push($x);)*
        Vec::from(v)
    }};
}

impl<'a, T: 'a> Deref for Vec<'a, T> {
    type Target = [T];

    fn deref(&self) -> &<Self as Deref>::Target {
        self.0.deref()
    }
}

impl<'a, T: 'a> IntoIterator for Vec<'a, T> {
    type Item = T;
    type IntoIter = IntoIter<T>;

    fn into_iter(self) -> IntoIter<T> {
        self.0.into_iter()
    }
}

impl<'a, 'bump, T> IntoIterator for &'a Vec<'bump, T> {
    type Item = &'a T;
    type IntoIter = slice::Iter<'a, T>;

    fn into_iter(self) -> slice::Iter<'a, T> {
        self.iter()
    }
}

impl<'a, T> From<BVec<'a, T>> for Vec<'a, T> {
    fn from(v: BVec<'a, T>) -> Self {
        Vec(v)
    }
}

impl<'a, T: 'a> Vec<'a, T> {
    pub fn new_in(arena: &'a Bump) -> Self {
        Vec(BVec::new_in(arena))
    }
}

impl<'a, T: 'a + Copy> Vec<'a, T> {
    pub fn append(&self, arena: &'a Bump, other: &Self) -> Self {
        Vec(BVec::from_iter_in(
            self.0.iter().chain(other.0.iter()).copied(),
            arena,
        ))
    }
}
