// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

pub use eq_modulo_pos_derive::EqModuloPos;

pub trait EqModuloPos {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool;
}

impl<T: EqModuloPos> EqModuloPos for Option<T> {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        match (self, rhs) {
            (Some(lhs), Some(rhs)) => lhs.eq_modulo_pos(rhs),
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
}

macro_rules! impl_with_equal {
    ($($ty:ty,)*) => {$(
        impl EqModuloPos for $ty {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
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
}

macro_rules! impl_deref_str {
    ($ty:ty) => {
        impl EqModuloPos for $ty {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
                (**self).eq_modulo_pos(*rhs)
            }
        }
    };
}

impl_deref_str! { &str }
impl_deref_str! { &bstr::BStr }
impl_deref_str! { &std::path::Path }

macro_rules! impl_deref {
    ($ty:ty) => {
        impl<T: EqModuloPos> EqModuloPos for $ty {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
                (**self).eq_modulo_pos(*rhs)
            }
        }
    };
}

impl_deref! { &T }
impl_deref! { &mut T }

macro_rules! impl_deref2 {
    ($ty:ty) => {
        impl<T: EqModuloPos> EqModuloPos for $ty {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
                self.as_ref().eq_modulo_pos(rhs.as_ref())
            }
        }
    };
}
impl_deref2! { Box<T> }
impl_deref2! { Rc<T> }

macro_rules! impl_tuple {
    () => (
        impl EqModuloPos for () {
            fn eq_modulo_pos(&self, _rhs: &Self) -> bool { true }
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
        }
    );
}

impl_tuple! { (A, a1, a2) }
impl_tuple! { (A, a1, a2) (B, b1, b2) }
impl_tuple! { (A, a1, a2) (B, b1, b2)  (C, c1, c2) }
impl_tuple! { (A, a1, a2) (B, b1, b2)  (C, c1, c2)  (D, d1, d2) }

macro_rules! impl_with_iter {
    (<$($gen:ident),*> $ty:ty , $size:ident) => {
        impl<$($gen: EqModuloPos,)*> EqModuloPos for $ty {
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
        }
    };
}

impl_with_iter! { <T> Vec<T>, len }
impl_with_iter! {
    <K, V> arena_collections::SortedAssocList<'_, K, V>, len
}
impl_with_iter! {
    <T> arena_collections::set::Set<'_, T>, count
}
impl_with_iter! {
    <K, V> arena_collections::map::Map<'_, K, V>, count
}
impl_with_iter! {
    <T> std::collections::BTreeSet<T>, len
}
impl_with_iter! {
    <K, V> std::collections::BTreeMap<K, V>, len
}
impl_with_iter! {
    <T> &[T], len
}
