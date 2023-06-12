// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;
use std::sync::Arc;

pub use eq_modulo_pos_derive::EqModuloPos;
use ocamlrep_caml_builtins::Int64;

/// An implementation of `Eq` which is insensitive to positions
/// (e.g., `pos::BPos`) and reasons (e.g., `ty::reason::BReason`).
///
/// If `PartialOrd` or `Ord` are also implemented for `Self`, their methods must
/// be consistent with `EqModuloPos`. For any two values for which
/// `eq_modulo_pos` or `eq_modulo_pos_and_reason` returns `false`, it must be
/// the case that their ordering cannot be changed by modifying positions or
/// reasons inside them.
pub trait EqModuloPos {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool;
    fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool;
}

impl<T: EqModuloPos> EqModuloPos for Option<T> {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        match (self, rhs) {
            (Some(lhs), Some(rhs)) => lhs.eq_modulo_pos(rhs),
            (None, None) => true,
            _ => false,
        }
    }
    fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
        match (self, rhs) {
            (Some(lhs), Some(rhs)) => lhs.eq_modulo_pos_and_reason(rhs),
            (None, None) => true,
            _ => false,
        }
    }
}

impl<T: EqModuloPos> EqModuloPos for [T] {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        if self.len() != rhs.len() {
            false
        } else {
            for (lhs, rhs) in self.iter().zip(rhs.iter()) {
                if !lhs.eq_modulo_pos(rhs) {
                    return false;
                }
            }
            true
        }
    }
    fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
        if self.len() != rhs.len() {
            false
        } else {
            for (lhs, rhs) in self.iter().zip(rhs.iter()) {
                if !lhs.eq_modulo_pos_and_reason(rhs) {
                    return false;
                }
            }
            true
        }
    }
}

impl<T: EqModuloPos> EqModuloPos for hcons::Hc<T> {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        (**self).eq_modulo_pos(&**rhs)
    }
    fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
        (**self).eq_modulo_pos_and_reason(&**rhs)
    }
}

macro_rules! impl_with_equal {
    ($($ty:ty,)*) => {$(
        impl EqModuloPos for $ty {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
                self == rhs
            }
            fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
                self == rhs
            }
        }
    )*}
}

impl_with_equal! {
    u8,
    u16,
    u32,
    u64,
    usize,
    i8,
    i16,
    i32,
    i64,
    isize,
    u128,
    i128,
    bool,
    char,
    str,
    String,
    std::path::Path,
    std::path::PathBuf,
    bstr::BStr,
    bstr::BString,
    Int64,
}

macro_rules! impl_deref {
    ($ty:ty) => {
        impl<T: EqModuloPos + ?Sized> EqModuloPos for $ty {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
                (**self).eq_modulo_pos(&**rhs)
            }
            fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
                (**self).eq_modulo_pos_and_reason(&**rhs)
            }
        }
    };
}

impl_deref! { &T }
impl_deref! { &mut T }
impl_deref! { Box<T> }
impl_deref! { Rc<T> }
impl_deref! { Arc<T> }

macro_rules! impl_tuple {
    () => (
        impl EqModuloPos for () {
            fn eq_modulo_pos(&self, _rhs: &Self) -> bool { true }
            fn eq_modulo_pos_and_reason(&self, _rhs: &Self) -> bool { true }
        }
    );

    ( $(($name:ident, $lhs:ident, $rhs:ident))+) => (
        impl< $($name: EqModuloPos),+ > EqModuloPos for ($($name,)+) {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
                let ($(ref $lhs,)+) = self;
                let ($(ref $rhs,)+) = rhs;
                true
                $(&& $lhs.eq_modulo_pos($rhs))+
            }
            fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
                let ($(ref $lhs,)+) = self;
                let ($(ref $rhs,)+) = rhs;
                true
                $(&& $lhs.eq_modulo_pos_and_reason($rhs))+
            }
        }
    );
}

impl_tuple! { (A, a1, a2) }
impl_tuple! { (A, a1, a2) (B, b1, b2) }
impl_tuple! { (A, a1, a2) (B, b1, b2)  (C, c1, c2) }
impl_tuple! { (A, a1, a2) (B, b1, b2)  (C, c1, c2)  (D, d1, d2) }

macro_rules! impl_with_iter {
    (<$($gen:ident),* $(,)?> <$($unbounded:ident),*> $ty:ty , $size:ident) => {
        impl<$($gen: EqModuloPos,)* $($unbounded,)*> EqModuloPos for $ty {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
                if self.$size() != rhs.$size() {
                    false
                } else {
                    let mut res = true;
                    for (lhs, rhs) in self.iter().zip(rhs.iter()) {
                        res = res && lhs.eq_modulo_pos(&rhs);
                    }
                    res
                }
            }
            fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
                if self.$size() != rhs.$size() {
                    false
                } else {
                    let mut res = true;
                    for (lhs, rhs) in self.iter().zip(rhs.iter()) {
                        res = res && lhs.eq_modulo_pos_and_reason(&rhs);
                    }
                    res
                }
            }
        }
    };
    (<$($gen:ident),* $(,)?> $ty:ty , $size:ident) => {
        impl_with_iter! { <$($gen,)*> <> $ty , $size }
    }
}

impl_with_iter! { <T> Vec<T>, len }
impl_with_iter! {
    <K, V> arena_collections::SortedAssocList<'_, K, V>, len
}

// The arena_collections Set and Map are ordered collections, and rely on the
// invariant that the impl of `Ord` is consistent with the impl of
// `EqModuloPos`.
impl_with_iter! {
    <T> arena_collections::set::Set<'_, T>, count
}
impl_with_iter! {
    <K, V> arena_collections::map::Map<'_, K, V>, count
}

// `BTreeSet` and `BTreeMap` also rely on the invariant that the impl of `Ord`
// is consistent with the impl of `EqModuloPos`. We can iterate over both
// collections and expect their keys to be in the same order, even if they
// differ in positions.
impl_with_iter! {
    <T> std::collections::BTreeSet<T>, len
}
impl_with_iter! {
    <K, V> std::collections::BTreeMap<K, V>, len
}

impl<K, V, S> EqModuloPos for std::collections::HashMap<K, V, S>
where
    K: Eq + std::hash::Hash,
    V: EqModuloPos,
    S: std::hash::BuildHasher,
    // This impl (and the impls for IndexMap, {Hash,Index}Set below) is
    // restricted to collections whose keys implement AsRef<str>. The intent is
    // to exclude maps and sets whose key types contain positions or reasons,
    // since this implementation does not compare keys modulo pos. In practice,
    // we only use maps and sets with string keys in types which need to
    // implement EqModuloPos.
    K: AsRef<str>,
{
    fn eq_modulo_pos(&self, other: &Self) -> bool {
        if self.len() != other.len() {
            return false;
        }
        // Since we've checked that both collections have the same number of
        // elements, and all keys are distinct, iterating over one set is
        // sufficient. That is to say, if all members of `self` are in `other`,
        // we can assume that all members of `other` are in `self`.
        // c.f. the impl of PartialEq: https://github.com/rust-lang/rust/blob/1.63.0/library/std/src/collections/hash/map.rs#L1275
        self.iter()
            .all(|(key, value)| other.get(key).map_or(false, |v| value.eq_modulo_pos(v)))
    }
    fn eq_modulo_pos_and_reason(&self, other: &Self) -> bool {
        if self.len() != other.len() {
            return false;
        }
        self.iter().all(|(key, value)| {
            other
                .get(key)
                .map_or(false, |v| value.eq_modulo_pos_and_reason(v))
        })
    }
}

impl<K, V, S> EqModuloPos for indexmap::IndexMap<K, V, S>
where
    K: Eq + std::hash::Hash + AsRef<str>,
    V: EqModuloPos,
    S: std::hash::BuildHasher,
{
    fn eq_modulo_pos(&self, other: &Self) -> bool {
        if self.len() != other.len() {
            return false;
        }
        self.iter()
            .all(|(key, value)| other.get(key).map_or(false, |v| value.eq_modulo_pos(v)))
    }
    fn eq_modulo_pos_and_reason(&self, other: &Self) -> bool {
        if self.len() != other.len() {
            return false;
        }
        self.iter().all(|(key, value)| {
            other
                .get(key)
                .map_or(false, |v| value.eq_modulo_pos_and_reason(v))
        })
    }
}

impl<T, S> EqModuloPos for std::collections::HashSet<T, S>
where
    T: Eq + std::hash::Hash + AsRef<str>,
    S: std::hash::BuildHasher,
{
    fn eq_modulo_pos(&self, other: &Self) -> bool {
        if self.len() != other.len() {
            return false;
        }
        self.iter().all(|key| other.contains(key))
    }
    fn eq_modulo_pos_and_reason(&self, other: &Self) -> bool {
        self.eq_modulo_pos(other)
    }
}

impl<T, S> EqModuloPos for indexmap::IndexSet<T, S>
where
    T: Eq + std::hash::Hash + AsRef<str>,
    S: std::hash::BuildHasher,
{
    fn eq_modulo_pos(&self, other: &Self) -> bool {
        if self.len() != other.len() {
            return false;
        }
        self.iter().all(|key| other.contains(key))
    }
    fn eq_modulo_pos_and_reason(&self, other: &Self) -> bool {
        self.eq_modulo_pos(other)
    }
}
