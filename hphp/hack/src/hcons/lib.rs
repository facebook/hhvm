// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;
use std::hash::Hash;
use std::hash::Hasher;
use std::ops::Deref;
use std::sync::Arc;
use std::sync::Weak;

use dashmap::mapref::entry::Entry;
use dashmap::DashMap;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
pub use once_cell::sync::Lazy;
use serde::Deserialize;
use serde::Deserializer;
use serde::Serialize;
use serde::Serializer;

/// A hash-consed pointer.
pub struct Hc<T>(Arc<T>);

impl<T: Consable> Hc<T> {
    #[inline]
    pub fn new(value: T) -> Self {
        T::conser().mk(value)
    }
}

pub trait Consable: Eq + Hash + Sized + 'static {
    fn conser() -> &'static Conser<Self>;
}

#[macro_export]
macro_rules! consable {
    ( $ty:ty ) => {
        impl crate::Consable for $ty {
            #[inline]
            fn conser() -> &'static crate::Conser<$ty> {
                static CONSER: crate::Lazy<crate::Conser<$ty>> =
                    crate::Lazy::new(crate::Conser::new);
                &CONSER
            }
        }
    };
}

impl<T> Clone for Hc<T> {
    #[inline]
    fn clone(&self) -> Self {
        Hc(Arc::clone(&self.0))
    }
}

impl<T: fmt::Debug> fmt::Debug for Hc<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        self.0.fmt(f)
    }
}

impl<T: fmt::Display> fmt::Display for Hc<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        self.0.fmt(f)
    }
}

impl<T> Deref for Hc<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.0
    }
}

impl<T: Eq> Eq for Hc<T> {}

impl<T: Hash> Hash for Hc<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        // hashbrown-based hash tables use the upper byte of the hash code as a
        // tag which drives SIMD parallelism. If `state` is a Hasher which
        // doesn't distribute pointer hashes well (e.g., nohash_hasher), then
        // all upper bytes of Hc hash codes will be the same, and perf of
        // hashbrown tables containing Hc keys will suffer. If we carefully
        // avoid such hashers, we could probably just hash the (fat) pointer
        // here. But as a precaution for now, run it through FNV first.
        state.write_u64(fnv_hash(&Arc::as_ptr(&self.0)));
    }
}

impl<T: PartialEq> PartialEq for Hc<T> {
    fn eq(&self, other: &Self) -> bool {
        std::ptr::eq(self.0.as_ref(), other.0.as_ref())
    }
}

impl<T: PartialEq> PartialEq<&Hc<T>> for Hc<T> {
    fn eq(&self, other: &&Hc<T>) -> bool {
        std::ptr::eq(self.0.as_ref(), other.0.as_ref())
    }
}

impl<T: PartialEq> PartialEq<Hc<T>> for &Hc<T> {
    fn eq(&self, other: &Hc<T>) -> bool {
        std::ptr::eq(self.0.as_ref(), other.0.as_ref())
    }
}

impl<T: PartialOrd> PartialOrd for Hc<T> {
    #[inline]
    fn partial_cmp(&self, other: &Hc<T>) -> Option<std::cmp::Ordering> {
        (**self).partial_cmp(&**other)
    }
}

impl<T: Ord> Ord for Hc<T> {
    #[inline]
    fn cmp(&self, other: &Hc<T>) -> std::cmp::Ordering {
        (**self).cmp(&**other)
    }
}

impl<T: Serialize> Serialize for Hc<T> {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        // TODO: The `intern` crate has a way of preserving sharing of interned
        // values in serde output; we may want to do the same here.
        (**self).serialize(serializer)
    }
}

impl<'de, T: Deserialize<'de> + Consable> Deserialize<'de> for Hc<T> {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        Deserialize::deserialize(deserializer).map(Hc::new)
    }
}

impl<T: ToOcamlRep + Consable> ToOcamlRep for Hc<T> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(
        &'a self,
        alloc: &'a A,
    ) -> ocamlrep::OpaqueValue<'a> {
        (**self).to_ocamlrep(alloc)
    }
}

impl<T: FromOcamlRep + Consable> FromOcamlRep for Hc<T> {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        Ok(Hc::new(T::from_ocamlrep(value)?))
    }
}

fn fnv_hash<T: Hash>(value: &T) -> u64 {
    let mut hasher = fnv::FnvHasher::default();
    value.hash(&mut hasher);
    hasher.finish()
}

#[derive(Debug)]
pub struct Conser<T> {
    table: DashMap<u64, Weak<T>>,
}

impl<T: Consable> Conser<T> {
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

    pub fn clear(&self) {
        self.table.clear()
    }

    fn mk(&self, x: T) -> Hc<T> {
        let hash = fnv_hash(&x);
        let rc = match self.table.entry(hash) {
            Entry::Occupied(mut o) => match o.get().upgrade() {
                Some(rc) => {
                    // TODO: handle collisions
                    debug_assert!(x == *rc);
                    rc
                }
                None => {
                    let rc = Arc::new(x);
                    o.insert(Arc::downgrade(&rc));
                    rc
                }
            },
            Entry::Vacant(v) => {
                let rc = Arc::new(x);
                v.insert(Arc::downgrade(&rc));
                rc
            }
        };
        Hc(rc)
    }
}
