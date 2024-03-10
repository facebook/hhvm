// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use hash::IndexMap;
use hash::IndexSet;

use crate::BytesId;
use crate::ClassName;
use crate::Constant;
use crate::FloatBits;
use crate::TypeInfo;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum TypedValue {
    Bool(bool),
    Dict(DictValue),
    Float(FloatBits),
    Int(i64),
    Keyset(KeysetValue),
    LazyClass(ClassName),
    Null,
    String(BytesId),
    Uninit,
    Vec(Vec<TypedValue>),
}

impl TypedValue {
    pub fn type_info(&self) -> TypeInfo {
        use crate::types::BaseType;
        match self {
            TypedValue::Bool(_) => BaseType::Bool.into(),
            TypedValue::Dict(_) => BaseType::Dict.into(),
            TypedValue::Float(_) => BaseType::Float.into(),
            TypedValue::Int(_) => BaseType::Int.into(),
            TypedValue::Keyset(_) => BaseType::Keyset.into(),
            TypedValue::LazyClass(_) => BaseType::String.into(),
            TypedValue::Null => BaseType::Null.into(),
            TypedValue::String(_) => BaseType::String.into(),
            TypedValue::Uninit => TypeInfo::empty(),
            TypedValue::Vec(_) => BaseType::Vec.into(),
        }
    }

    pub fn get_dict(&self) -> Option<&DictValue> {
        match self {
            TypedValue::Dict(dv) => Some(dv),
            _ => None,
        }
    }

    pub fn get_int(&self) -> Option<i64> {
        match self {
            TypedValue::Int(num) => Some(*num),
            _ => None,
        }
    }

    pub fn get_string(&self) -> Option<BytesId> {
        match self {
            TypedValue::String(str) => Some(*str),
            _ => None,
        }
    }
}

impl From<TypedValue> for Constant {
    fn from(tv: TypedValue) -> Self {
        match tv {
            TypedValue::Bool(b) => Constant::Bool(b),
            TypedValue::Float(f) => Constant::Float(f),
            TypedValue::Int(i) => Constant::Int(i),
            TypedValue::LazyClass(id) => Constant::String(id.as_bytes_id()),
            TypedValue::Null => Constant::Null,
            TypedValue::String(id) => Constant::String(id),
            TypedValue::Uninit => Constant::Uninit,
            TypedValue::Dict(_) | TypedValue::Keyset(_) | TypedValue::Vec(_) => {
                Constant::Array(Arc::new(tv))
            }
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
pub enum ArrayKey {
    Int(i64),
    String(BytesId),
    LazyClass(ClassName),
}

/// A wrapper around IndexSet<ArrayKey> which includes key ordering for
/// comparisons and provides hash.
#[derive(Clone, Debug, Default)]
pub struct KeysetValue(pub IndexSet<ArrayKey>);

impl std::ops::Deref for KeysetValue {
    type Target = IndexSet<ArrayKey>;

    fn deref(&self) -> &IndexSet<ArrayKey> {
        &self.0
    }
}

impl std::cmp::PartialEq for KeysetValue {
    fn eq(&self, other: &KeysetValue) -> bool {
        // IndexSet implements PartialEq but it ignores insertion order,
        // which we care about.
        self.len() == other.len() && self.iter().zip(other.iter()).all(|(a, b)| a == b)
    }
}

impl std::cmp::Eq for KeysetValue {}

impl std::hash::Hash for KeysetValue {
    fn hash<H: std::hash::Hasher>(&self, h: &mut H) {
        // IndexSet doesn't implement Hash so we need to provide our
        // own. It needs to match TypedValue::eq() which uses insertion
        // ordering so we'll do the same.
        for v in self.iter() {
            v.hash(h);
        }
    }
}

impl FromIterator<ArrayKey> for KeysetValue {
    fn from_iter<T>(iterator: T) -> Self
    where
        T: IntoIterator<Item = ArrayKey>,
    {
        Self(IndexSet::from_iter(iterator))
    }
}

/// A wrapper around IndexMap<ArrayKey, TypedValue> which includes key ordering
/// for comparisons and provides hash.
#[derive(Clone, Debug, Default)]
pub struct DictValue(pub IndexMap<ArrayKey, TypedValue>);

impl std::ops::Deref for DictValue {
    type Target = IndexMap<ArrayKey, TypedValue>;

    fn deref(&self) -> &IndexMap<ArrayKey, TypedValue> {
        &self.0
    }
}

impl std::cmp::PartialEq for DictValue {
    fn eq(&self, other: &DictValue) -> bool {
        // IndexMap implements PartialEq but it ignores insertion order,
        // which we care about.
        self.len() == other.len() && self.iter().zip(other.iter()).all(|(a, b)| a == b)
    }
}

impl std::cmp::Eq for DictValue {}

impl std::hash::Hash for DictValue {
    fn hash<H: std::hash::Hasher>(&self, h: &mut H) {
        // IndexMap doesn't implement Hash so we need to provide our
        // own. It needs to match TypedValue::eq() which uses insertion
        // ordering so we'll do the same.
        for e in self.iter() {
            e.hash(h);
        }
    }
}

impl FromIterator<(ArrayKey, TypedValue)> for DictValue {
    fn from_iter<T>(iterator: T) -> Self
    where
        T: IntoIterator<Item = (ArrayKey, TypedValue)>,
    {
        Self(IndexMap::from_iter(iterator))
    }
}

#[cfg(test)]
mod test {
    use std::hash::BuildHasher;

    use super::*;

    #[test]
    fn test1() {
        let a = TypedValue::Keyset(
            [ArrayKey::Int(1), ArrayKey::Int(2), ArrayKey::Int(3)]
                .into_iter()
                .collect(),
        );
        let b = TypedValue::Keyset(
            [ArrayKey::Int(3), ArrayKey::Int(2), ArrayKey::Int(1)]
                .into_iter()
                .collect(),
        );
        assert_eq!(a, a);
        assert_ne!(a, b);
        let h = hash::BuildHasher::default();
        assert_ne!(h.hash_one(a), h.hash_one(b));
    }

    #[test]
    fn test2() {
        let a = TypedValue::Dict(
            [
                (ArrayKey::Int(1), TypedValue::Int(10)),
                (ArrayKey::Int(2), TypedValue::Int(11)),
                (ArrayKey::Int(3), TypedValue::Int(12)),
            ]
            .into_iter()
            .collect(),
        );
        let b = TypedValue::Dict(
            [
                (ArrayKey::Int(3), TypedValue::Int(12)),
                (ArrayKey::Int(2), TypedValue::Int(11)),
                (ArrayKey::Int(1), TypedValue::Int(10)),
            ]
            .into_iter()
            .collect(),
        );
        assert_eq!(a, a);
        assert_ne!(a, b);
        let h = hash::BuildHasher::default();
        assert_ne!(h.hash_one(a), h.hash_one(b));
    }

    #[test]
    fn test3() {
        use intern::bytes_id;
        let a = TypedValue::Dict(
            [
                (ArrayKey::String(bytes_id!(b"E")), TypedValue::Int(1)),
                (ArrayKey::String(bytes_id!(b"F")), TypedValue::Int(1)),
                (ArrayKey::String(bytes_id!(b"G")), TypedValue::Int(1)),
                (ArrayKey::String(bytes_id!(b"H")), TypedValue::Int(1)),
                (ArrayKey::String(bytes_id!(b"I")), TypedValue::Int(1)),
            ]
            .into_iter()
            .collect(),
        );
        let b = TypedValue::Dict(Default::default());
        assert_eq!(a, a);
        assert_ne!(a, b);
        let h = hash::BuildHasher::default();
        assert_ne!(h.hash_one(a), h.hash_one(b));
    }
}
