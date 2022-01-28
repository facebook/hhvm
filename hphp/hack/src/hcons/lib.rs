// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Borrow;
use std::fmt;
use std::hash::{Hash, Hasher};
use std::ops::Deref;
use std::sync::{Arc, Weak};

use dashmap::{mapref::entry::Entry, DashMap};

/// A hash-consed pointer.
pub struct Hc<T: ?Sized>(Arc<T>);

impl<T: ?Sized> Clone for Hc<T> {
    #[inline]
    fn clone(&self) -> Self {
        Hc(Arc::clone(&self.0))
    }
}

impl<T: fmt::Debug + ?Sized> fmt::Debug for Hc<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        self.0.fmt(f)
    }
}

impl<T: fmt::Display + ?Sized> fmt::Display for Hc<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
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

impl<T: ?Sized> PartialEq<&Hc<T>> for Hc<T> {
    fn eq(&self, other: &&Hc<T>) -> bool {
        std::ptr::eq(self.0.as_ref(), other.0.as_ref())
    }
}

impl<T: ?Sized> PartialEq<Hc<T>> for &Hc<T> {
    fn eq(&self, other: &Hc<T>) -> bool {
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
impl_str_eq! { Hc<str>, &String }
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

fn hash<T: Hash + ?Sized>(value: &T) -> u64 {
    let mut hasher = fnv::FnvHasher::default();
    value.hash(&mut hasher);
    hasher.finish()
}

#[derive(Debug)]
pub struct Conser<T: ?Sized> {
    table: DashMap<u64, Weak<T>>,
}

impl<T: Eq + Hash + ?Sized> Conser<T> {
    pub fn new() -> Self {
        Conser {
            table: DashMap::new(),
        }
    }

    pub fn gc(&self) -> bool {
        let l = self.table.len();
        self.table.retain(|_, v| v.strong_count() != 0);
        l != self.table.len()
    }

    fn mk_helper<U>(
        &self,
        hash: u64,
        x: U,
        make_rc: impl FnOnce(U) -> Arc<T>,
        eq: impl FnOnce(&U, &T) -> bool,
    ) -> Hc<T> {
        let rc = match self.table.entry(hash) {
            Entry::Occupied(mut o) => match o.get().upgrade() {
                Some(rc) => {
                    // TODO: handle collisions
                    debug_assert!(eq(&x, &rc));
                    rc
                }
                None => {
                    let rc = make_rc(x);
                    o.insert(Arc::downgrade(&rc));
                    rc
                }
            },
            Entry::Vacant(v) => {
                let rc = make_rc(x);
                v.insert(Arc::downgrade(&rc));
                rc
            }
        };
        Hc(rc)
    }

    pub fn mk_from_ref<'a, Q>(&self, x: &'a Q) -> Hc<T>
    where
        T: Borrow<Q>,
        Q: 'a + Eq + Hash + PartialEq<T> + ?Sized,
        Arc<T>: From<&'a Q>,
    {
        self.mk_helper(hash(x), x, Arc::from, |x: &&Q, rc: &T| *x == rc.borrow())
    }
}

impl<T: Eq + Hash> Conser<T> {
    pub fn mk(&self, x: T) -> Hc<T> {
        self.mk_helper(hash(&x), x, Arc::new, |x: &T, rc: &T| x == rc)
    }
}

impl Conser<[u8]> {
    pub fn mk_str(&self, x: &str) -> Hc<str> {
        let bytes = self.mk_from_ref(x.as_bytes());
        let ptr: *const [u8] = Arc::into_raw(bytes.0);
        // SAFETY: `x` is known to be a valid `&str`, and `bytes` is just a
        // ref-counted copy of it.
        unsafe { Hc(Arc::from_raw(ptr as *const str)) }
    }

    pub fn mk_bstr<B: ?Sized + AsRef<[u8]>>(&self, x: &B) -> Hc<bstr::BStr> {
        let bytes = self.mk_from_ref(x.as_ref());
        let ptr: *const [u8] = Arc::into_raw(bytes.0);
        // SAFETY: `BStr` is a `repr(transparent)` wrapper for `[u8]`.
        unsafe { Hc(Arc::from_raw(ptr as *const bstr::BStr)) }
    }
}
