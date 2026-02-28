// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::Hash;
use std::hash::Hasher;
use std::ops::Deref;

use ffi::Vector;
use intern::string::BytesId;
use serde::Serialize;
use serde::Serializer;
use triomphe::Arc;
use triomphe::OffsetArc;

use crate::ClassName;

/// Raw IEEE floating point bits. We use this rather than f64 so that
/// hash/equality have bitwise interning behavior: -0.0 != 0.0, NaN == NaN.
/// If we ever implement Ord/PartialOrd, we'd need to base it on the raw bits
/// (u64), not floating point partial order.
#[derive(Copy, Clone, Debug, Serialize)]
#[repr(transparent)]
pub struct FloatBits(pub f64);

impl FloatBits {
    pub fn to_f64(self) -> f64 {
        self.0
    }

    pub fn to_bits(self) -> u64 {
        self.0.to_bits()
    }
}

impl Eq for FloatBits {}
impl PartialEq for FloatBits {
    fn eq(&self, other: &FloatBits) -> bool {
        self.to_bits() == other.to_bits()
    }
}

impl std::hash::Hash for FloatBits {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.to_bits().hash(state);
    }
}

impl From<f64> for FloatBits {
    fn from(x: f64) -> Self {
        Self(x)
    }
}

/// We introduce a type for Hack/PHP values, mimicking what happens at
/// runtime. Currently this is used for constant folding.
/// These convert to HHVM TypedValue - see runtime/base/typed-value.h
#[derive(Clone, Debug, Eq, Hash, PartialEq, Serialize)]
#[repr(C)]
pub enum TypedValue {
    /// Used for fields that are initialized in the 86pinit method
    Uninit,

    /// Hack/PHP integers are signed 64-bit
    Int(i64),

    /// Hack bool
    Bool(bool),

    /// Hack, C++, PHP, and Caml floats are IEEE754 64-bit
    Float(FloatBits),

    /// Hack strings are plain bytes with no utf-8 guarantee
    String(BytesId),

    /// Hack source code including identifiers must be valid utf-8
    LazyClass(ClassName),

    /// Hack null
    Null,

    /// Hack vec<T>
    Vec(ArcVec<TypedValue>),

    /// Hack keyset<T>
    Keyset(ArcVec<TypedValue>),

    /// Hack dict<T>
    Dict(ArcVec<Entry<TypedValue, TypedValue>>),

    /// Hack EnumClassLabel (#FieldName, EnumName#FieldName)
    EnumClassLabel(BytesId),
}

/// A reference counted Vector<T>
#[derive(Clone, Debug, Eq, PartialEq)]
#[repr(C)]
pub struct ArcVec<T>(OffsetArc<Vector<T>>);

impl<T: Hash> Hash for ArcVec<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.0.hash(state)
    }
}

impl<T: Serialize> Serialize for ArcVec<T> {
    fn serialize<S: Serializer>(&self, ser: S) -> Result<S::Ok, S::Error> {
        self.0.serialize(ser)
    }
}

impl<T> Deref for ArcVec<T> {
    type Target = Vector<T>;
    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl<T: Clone> ArcVec<T> {
    pub fn into_vec(self) -> Vec<T> {
        Arc::unwrap_or_clone(Arc::from_raw_offset(self.0)).into()
    }
}

// This is declared as a generic type to work around cbindgen's topo-sort,
// which outputs Entry before TypedValue. This violates C++ ordering rules:
// A struct field type must be declared before the field.
#[derive(Clone, Debug, Eq, Hash, PartialEq, Serialize)]
#[repr(C)]
pub struct Entry<K, V> {
    pub key: K,
    pub value: V,
}

pub type DictEntry = Entry<TypedValue, TypedValue>;

impl TypedValue {
    pub fn intern_string(x: impl AsRef<[u8]>) -> Self {
        Self::String(intern::string::intern_bytes(x.as_ref()))
    }

    pub fn intern_enum_class_label(x: impl AsRef<[u8]>) -> Self {
        Self::EnumClassLabel(intern::string::intern_bytes(x.as_ref()))
    }

    pub fn intern_lazy_class(x: impl AsRef<str>) -> Self {
        Self::LazyClass(ClassName::intern(x.as_ref()))
    }

    pub fn vec(x: Vec<TypedValue>) -> Self {
        Self::Vec(ArcVec(Arc::into_raw_offset(Arc::new(Vector::from(x)))))
    }

    pub fn keyset(x: Vec<TypedValue>) -> Self {
        Self::Keyset(ArcVec(Arc::into_raw_offset(Arc::new(Vector::from(x)))))
    }

    pub fn dict(x: Vec<DictEntry>) -> Self {
        Self::Dict(ArcVec(Arc::into_raw_offset(Arc::new(Vector::from(x)))))
    }

    pub fn float(f: f64) -> Self {
        Self::Float(f.into())
    }

    pub fn get_string(&self) -> Option<BytesId> {
        match self {
            TypedValue::String(str) => Some(*str),
            _ => None,
        }
    }

    pub fn get_int(&self) -> Option<i64> {
        match self {
            TypedValue::Int(num) => Some(*num),
            _ => None,
        }
    }

    pub fn get_bool(&self) -> Option<bool> {
        match self {
            TypedValue::Bool(b) => Some(*b),
            _ => None,
        }
    }

    pub fn get_dict(&self) -> Option<&[DictEntry]> {
        match self {
            TypedValue::Dict(dv) => Some(&dv.0),
            _ => None,
        }
    }
}

pub fn dict_get<'d>(d: &'d [DictEntry], q: &TypedValue) -> Option<&'d TypedValue> {
    for e in d {
        if &e.key == q {
            return Some(&e.value);
        }
    }
    None
}
