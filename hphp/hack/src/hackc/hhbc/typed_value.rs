// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;
use ffi::Vector;
use serde::Serialize;

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
/// runtime. Currently this is used for constant folding. By defining
/// a special type, we ensure independence from usage: for example, it
/// can be used for optimization on ASTs, or on bytecode, or (in
/// future) on a compiler intermediate language. HHVM takes a similar
/// approach: see runtime/base/typed-value.h
#[derive(Clone, Debug, Eq, Hash, PartialEq, Serialize)]
#[repr(C)]
pub enum TypedValue<'arena> {
    /// Used for fields that are initialized in the 86pinit method
    Uninit,
    /// Hack/PHP integers are signed 64-bit
    Int(i64),
    Bool(bool),
    /// Hack, C++, PHP, and Caml floats are IEEE754 64-bit
    Float(FloatBits),
    String(Str<'arena>),
    LazyClass(Str<'arena>),
    Null,
    // Hack arrays: vectors, keysets, and dictionaries
    Vec(Vector<TypedValue<'arena>>),
    Keyset(Vector<TypedValue<'arena>>),
    Dict(Vector<Entry<TypedValue<'arena>, TypedValue<'arena>>>),
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

pub type DictEntry<'a> = Entry<TypedValue<'a>, TypedValue<'a>>;

impl<'arena> TypedValue<'arena> {
    pub fn string(x: impl Into<Str<'arena>>) -> Self {
        Self::String(x.into())
    }

    pub fn vec(x: Vec<TypedValue<'arena>>) -> Self {
        Self::Vec(x.into())
    }

    pub fn keyset(x: Vec<TypedValue<'arena>>) -> Self {
        Self::Keyset(x.into())
    }

    pub fn dict(x: Vec<DictEntry<'arena>>) -> Self {
        Self::Dict(x.into())
    }

    pub fn alloc_string(s: impl AsRef<str>, alloc: &'arena bumpalo::Bump) -> Self {
        Self::String((alloc.alloc_str(s.as_ref()) as &str).into())
    }

    pub fn float(f: f64) -> Self {
        Self::Float(f.into())
    }
}
