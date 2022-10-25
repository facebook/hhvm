// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::IndexMap;
use hash::IndexSet;
pub use hhbc::FloatBits;

use crate::UnitBytesId;

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum TypedValue {
    Uninit,
    Int(i64),
    Bool(bool),
    Float(FloatBits),
    String(UnitBytesId),
    LazyClass(UnitBytesId),
    Null,
    Vec(Vec<TypedValue>),
    Keyset(IndexSet<ArrayKey>),
    Dict(IndexMap<ArrayKey, TypedValue>),
}

#[allow(clippy::derive_hash_xor_eq)]
impl std::hash::Hash for TypedValue {
    fn hash<H: std::hash::Hasher>(&self, h: &mut H) {
        use std::hash::Hash;
        Hash::hash(&std::mem::discriminant(self), h);
        match self {
            TypedValue::Uninit => {}
            TypedValue::Int(i) => i.hash(h),
            TypedValue::Bool(b) => b.hash(h),
            TypedValue::Float(fb) => fb.hash(h),
            TypedValue::String(id) | TypedValue::LazyClass(id) => id.hash(h),
            TypedValue::Null => {}
            TypedValue::Vec(c) => c.hash(h),
            TypedValue::Keyset(c) => {
                // IndexSet doesn't implement Hash so we need to provide our
                // own. It needs to match IndexSet::eq() which uses insertion
                // ordering so we'll do the same.
                for v in c.iter() {
                    v.hash(h);
                }
            }
            TypedValue::Dict(c) => {
                // IndexMap doesn't implement Hash so we need to provide our
                // own. It needs to match IndexSet::eq() which uses insertion
                // ordering so we'll do the same.
                for e in c.iter() {
                    e.hash(h);
                }
            }
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
pub enum ArrayKey {
    Int(i64),
    String(UnitBytesId),
    LazyClass(UnitBytesId),
}
