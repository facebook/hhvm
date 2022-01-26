// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::hash_map::Entry;
use std::fmt;
use std::hash::{Hash, Hasher};
use std::ops::Deref;
use std::rc::{Rc, Weak};
use std::sync::Mutex;

use nohash_hasher::IntMap;

/// A hash-consed pointer.
pub struct Hc<T: ?Sized>(Rc<T>);

impl<T: ?Sized> Clone for Hc<T> {
    #[inline]
    fn clone(&self) -> Self {
        Hc(Rc::clone(&self.0))
    }
}

impl<T: fmt::Debug + ?Sized> fmt::Debug for Hc<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        self.0.fmt(f)
    }
}

impl<T: ?Sized> Deref for Hc<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.0
    }
}

impl<T: ?Sized> Eq for Hc<T> {}

impl<T: Hash + ?Sized> Hash for Hc<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.0.hash(state);
    }
}

impl<T: ?Sized> PartialEq for Hc<T> {
    fn eq(&self, other: &Self) -> bool {
        std::ptr::eq(self.0.as_ref(), other.0.as_ref())
    }
}

macro_rules! impl_str_eq {
    ($lhs:ty, $rhs:ty) => {
        impl PartialEq<$rhs> for $lhs {
            #[inline]
            fn eq(&self, other: &$rhs) -> bool {
                PartialEq::eq(&self[..], &other[..])
            }
            #[inline]
            fn ne(&self, other: &$rhs) -> bool {
                PartialEq::ne(&self[..], &other[..])
            }
        }

        impl PartialEq<$lhs> for $rhs {
            #[inline]
            fn eq(&self, other: &$lhs) -> bool {
                PartialEq::eq(&self[..], &other[..])
            }
            #[inline]
            fn ne(&self, other: &$lhs) -> bool {
                PartialEq::ne(&self[..], &other[..])
            }
        }
    };
}

impl_str_eq! { Hc<str>, str }
impl_str_eq! { Hc<str>, &str }
impl_str_eq! { Hc<str>, String }
impl_str_eq! { Hc<str>, Box<str> }

impl AsRef<str> for Hc<str> {
    #[inline]
    fn as_ref(&self) -> &str {
        &self[..]
    }
}

impl AsRef<[u8]> for Hc<str> {
    #[inline]
    fn as_ref(&self) -> &[u8] {
        self.as_bytes()
    }
}

impl AsRef<std::path::Path> for Hc<str> {
    #[inline]
    fn as_ref(&self) -> &std::path::Path {
        std::path::Path::new(&self[..])
    }
}

impl AsRef<std::ffi::OsStr> for Hc<str> {
    #[inline]
    fn as_ref(&self) -> &std::ffi::OsStr {
        std::ffi::OsStr::new(&self[..])
    }
}

#[derive(Debug)]
pub struct Conser<T: ?Sized> {
    table: Mutex<IntMap<u64, Weak<T>>>,
}

impl<T: Eq + Hash + ?Sized> Conser<T> {
    pub fn new() -> Self {
        Conser {
            table: Mutex::new(IntMap::default()),
        }
    }

    fn hash(value: &T) -> u64 {
        let mut hasher = fnv::FnvHasher::default();
        value.hash(&mut hasher);
        hasher.finish()
    }

    pub fn gc(&self) -> bool {
        let mut table = self.table.lock().unwrap();
        let mut flag = false;
        loop {
            let l = table.len();
            table.retain(|_, v| v.strong_count() != 0);
            if l == table.len() {
                break;
            }
            flag = true;
        }
        flag
    }
}

impl<T: Eq + Hash> Conser<T> {
    pub fn mk(&self, x: T) -> Hc<T> {
        let hash = Self::hash(&x);
        let mut table = self.table.lock().unwrap();
        let rc = match table.entry(hash) {
            Entry::Occupied(mut o) => match o.get().upgrade() {
                Some(rc) => {
                    // TODO: handle collisions
                    debug_assert!(x == *rc);
                    rc
                }
                None => {
                    let rc = Rc::new(x);
                    o.insert(Rc::downgrade(&rc));
                    rc
                }
            },
            Entry::Vacant(v) => {
                let rc = Rc::new(x);
                v.insert(Rc::downgrade(&rc));
                rc
            }
        };
        Hc(rc)
    }
}

impl Conser<str> {
    pub fn mk_str(&self, x: &str) -> Hc<str> {
        let hash = Self::hash(&x);
        let mut table = self.table.lock().unwrap();
        let rc = match table.entry(hash) {
            Entry::Occupied(mut o) => match o.get().upgrade() {
                Some(rc) => {
                    // TODO: handle collisions
                    debug_assert_eq!(&x[..], &rc[..]);
                    rc
                }
                None => {
                    let rc = Rc::from(x);
                    o.insert(Rc::downgrade(&rc));
                    rc
                }
            },
            Entry::Vacant(v) => {
                let rc = Rc::from(x);
                v.insert(Rc::downgrade(&rc));
                rc
            }
        };
        Hc(rc)
    }
}
