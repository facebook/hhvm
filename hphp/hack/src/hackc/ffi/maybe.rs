// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::Serialize;

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
}
