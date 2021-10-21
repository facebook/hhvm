// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Pair, Slice, Str};

mod float {
    #[derive(Clone, Copy, Debug, Hash, PartialOrd, Ord, PartialEq, Eq)]
    #[repr(C)]
    pub struct F64([u8; 8]);

    impl F64 {
        pub fn to_f64(&self) -> f64 {
            (*self).into()
        }

        pub fn classify(&self) -> std::num::FpCategory {
            self.to_f64().classify()
        }
    }

    impl std::convert::From<f64> for F64 {
        fn from(f: f64) -> F64 {
            F64(f.to_be_bytes())
        }
    }

    impl std::convert::Into<f64> for F64 {
        fn into(self) -> f64 {
            f64::from_be_bytes(self.0)
        }
    }
}

/// We introduce a type for Hack/PHP values, mimicking what happens at
/// runtime. Currently this is used for constant folding. By defining
/// a special type, we ensure independence from usage: for example, it
/// can be used for optimization on ASTs, or on bytecode, or (in
/// future) on a compiler intermediate language. HHVM takes a similar
/// approach: see runtime/base/typed-value.h
#[derive(Clone, Debug, Eq, Hash, PartialEq, PartialOrd, Ord)]
#[repr(C)]
pub enum TypedValue<'arena> {
    /// Used for fields that are initialized in the 86pinit method
    Uninit,
    /// Hack/PHP integers are 64-bit
    Int(i64),
    Bool(bool),
    /// Both Hack/PHP and Caml floats are IEEE754 64-bit
    Float(float::F64),
    String(Str<'arena>),
    LazyClass(Str<'arena>),
    Null,
    // Classic PHP arrays with explicit (key,value) entries
    HhasAdata(Str<'arena>),
    // Hack arrays: vectors, keysets, and dictionaries
    Vec(Slice<'arena, TypedValue<'arena>>),
    Keyset(Slice<'arena, TypedValue<'arena>>),
    Dict(Slice<'arena, Pair<TypedValue<'arena>, TypedValue<'arena>>>),
}

/// Cast to a boolean: the (bool) operator in PHP
impl<'arena> std::convert::From<TypedValue<'arena>> for bool {
    fn from(x: TypedValue<'arena>) -> bool {
        match x {
            TypedValue::Uninit => false, // Should not happen
            TypedValue::Bool(b) => b,
            TypedValue::Null => false,
            TypedValue::String(s) => !s.is_empty() && s.as_str() != "0",
            TypedValue::LazyClass(_) => true,
            TypedValue::Int(i) => i != 0,
            TypedValue::Float(f) => f.to_f64() != 0.0,
            // Empty collections cast to false if empty, otherwise true
            TypedValue::Vec(v) => !v.is_empty(),
            TypedValue::Keyset(v) => !v.is_empty(),
            TypedValue::Dict(v) => !v.is_empty(),
            // Non-empty collections cast to true
            TypedValue::HhasAdata(_) => true,
        }
    }
}

pub type CastError = ();

/// Cast to an integer: the (int) operator in PHP. Return Err if we can't
/// or won't produce the correct value
impl<'arena> std::convert::TryFrom<TypedValue<'arena>> for i64 {
    type Error = CastError;
    fn try_from(x: TypedValue<'arena>) -> std::result::Result<i64, Self::Error> {
        match x {
            TypedValue::Uninit => Err(()), // Should not happen
            // Unreachable - the only calliste of to_int is cast_to_arraykey, which never
            // calls it with String
            TypedValue::String(_) => Err(()),    // not worth it
            TypedValue::LazyClass(_) => Err(()), // not worth it
            TypedValue::Int(i) => Ok(i),
            TypedValue::Float(f) => match f.classify() {
                std::num::FpCategory::Nan | std::num::FpCategory::Infinite => {
                    if f.to_f64() == std::f64::INFINITY {
                        Ok(0)
                    } else {
                        Ok(std::i64::MIN)
                    }
                }
                _ => panic!("TODO"),
            },
            v => Ok(if v.into() { 1 } else { 0 }),
        }
    }
}

/// Cast to a float: the (float) operator in PHP. Return Err if we can't
/// or won't produce the correct value
impl<'arena> std::convert::TryFrom<TypedValue<'arena>> for f64 {
    type Error = CastError;
    fn try_from(v: TypedValue<'arena>) -> std::result::Result<f64, Self::Error> {
        match v {
            TypedValue::Uninit => Err(()),       // Should not happen
            TypedValue::String(_) => Err(()),    // not worth it
            TypedValue::LazyClass(_) => Err(()), // not worth it
            TypedValue::Int(i) => Ok(i as f64),
            TypedValue::Float(f) => Ok(f.into()),
            _ => Ok(if v.into() { 1.0 } else { 0.0 }),
        }
    }
}

/// Cast to a string: the (string) operator in PHP. Return Err if we can't
/// or won't produce the correct value *)
impl<'arena> std::convert::TryFrom<TypedValue<'arena>> for std::string::String {
    type Error = CastError;
    fn try_from(x: TypedValue<'arena>) -> std::result::Result<std::string::String, Self::Error> {
        match x {
            TypedValue::Uninit => Err(()), // Should not happen
            TypedValue::Bool(false) => Ok("".into()),
            TypedValue::Bool(true) => Ok("1".into()),
            TypedValue::Null => Ok("".into()),
            TypedValue::Int(i) => Ok(i.to_string()),
            TypedValue::String(s) => Ok(s.as_str().into()),
            TypedValue::LazyClass(s) => Ok(s.as_str().into()),
            _ => Err(()),
        }
    }
}

struct WithBump<'arena, T>(&'arena bumpalo::Bump, T);

impl<'arena> std::convert::TryFrom<WithBump<'arena, TypedValue<'arena>>> for Str<'arena> {
    type Error = CastError;
    fn try_from(
        x: WithBump<'arena, TypedValue<'arena>>,
    ) -> std::result::Result<Str<'arena>, Self::Error> {
        let alloc = x.0;
        match x.1 {
            TypedValue::Uninit => Err(()), // Should not happen
            TypedValue::Bool(false) => Ok("".into()),
            TypedValue::Bool(true) => Ok("1".into()),
            TypedValue::Null => Ok("".into()),
            TypedValue::Int(i) => Ok(alloc.alloc_str(i.to_string().as_str()).into()),
            TypedValue::String(s) => Ok(s),
            TypedValue::LazyClass(s) => Ok(s),
            _ => Err(()),
        }
    }
}

impl<'arena> TypedValue<'arena> {
    pub fn mk_string(x: impl Into<Str<'arena>>) -> Self {
        Self::String(x.into())
    }

    pub fn mk_hhas_adata(x: impl Into<Str<'arena>>) -> Self {
        Self::HhasAdata(x.into())
    }

    pub fn mk_vec(x: impl Into<Slice<'arena, TypedValue<'arena>>>) -> Self {
        Self::Vec(x.into())
    }

    pub fn mk_keyset(x: impl Into<Slice<'arena, TypedValue<'arena>>>) -> Self {
        Self::Keyset(x.into())
    }

    pub fn mk_dict(
        x: impl Into<Slice<'arena, Pair<TypedValue<'arena>, TypedValue<'arena>>>>,
    ) -> Self {
        Self::Dict(x.into())
    }

    // Integer operations. For now, we don't attempt to implement the
    // overflow-to-float semantics
    fn add_int(i1: i64, i2: i64) -> Option<Self> {
        Some(Self::Int(
            (std::num::Wrapping(i1) + std::num::Wrapping(i2)).0,
        ))
    }

    pub fn neg(&self) -> Option<Self> {
        match self {
            Self::Int(i) => Some(Self::Int((-std::num::Wrapping(*i)).0)),
            Self::Float(i) => Some(Self::float(0.0 - i.to_f64())),
            _ => None,
        }
    }

    fn sub_int(i1: i64, i2: i64) -> Option<Self> {
        Some(Self::Int(
            (std::num::Wrapping(i1) - std::num::Wrapping(i2)).0,
        ))
    }

    // Arithmetic. For now, only on pure integer or float operands
    pub fn sub(&self, v2: &Self) -> Option<Self> {
        match (self, v2) {
            (Self::Int(i1), Self::Int(i2)) => Self::sub_int(*i1, *i2),
            (Self::Float(f1), Self::Float(f2)) => Some(Self::float(f1.to_f64() - f2.to_f64())),
            _ => None,
        }
    }

    fn mul_int(i1: i64, i2: i64) -> Option<Self> {
        Some(Self::Int(
            (std::num::Wrapping(i1) * std::num::Wrapping(i2)).0,
        ))
    }

    // Arithmetic. For now, only on pure integer or float operands
    pub fn mul(&self, other: &Self) -> Option<Self> {
        match (self, other) {
            (Self::Int(i1), Self::Int(i2)) => Self::mul_int(*i1, *i2),
            (Self::Float(i1), Self::Float(i2)) => Some(Self::float(i1.to_f64() * i2.to_f64())),
            (Self::Int(i1), Self::Float(i2)) => Some(Self::float(*i1 as f64 * i2.to_f64())),
            (Self::Float(i1), Self::Int(i2)) => Some(Self::float(i1.to_f64() * *i2 as f64)),
            _ => None,
        }
    }

    // Arithmetic. For now, only on pure integer or float operands
    pub fn div(&self, v2: &Self) -> Option<Self> {
        match (self, v2) {
            (Self::Int(i1), Self::Int(i2)) if *i2 != 0 => {
                if i1 % i2 == 0 {
                    Some(Self::Int(i1 / i2))
                } else {
                    Some(Self::float(*i1 as f64 / *i2 as f64))
                }
            }
            (Self::Float(f1), Self::Float(f2)) if f2.to_f64() != 0.0 => {
                Some(Self::float(f1.to_f64() / f2.to_f64()))
            }
            (Self::Int(i1), Self::Float(f2)) if f2.to_f64() != 0.0 => {
                Some(Self::float(*i1 as f64 / f2.to_f64()))
            }
            (Self::Float(f1), Self::Int(i2)) if *i2 != 0 => {
                Some(Self::float(f1.to_f64() / *i2 as f64))
            }
            _ => None,
        }
    }

    // Arithmetic. For now, only on pure integer or float operands
    pub fn add(&self, other: &Self) -> Option<Self> {
        match (self, other) {
            (Self::Float(i1), Self::Float(i2)) => Some(Self::float(i1.to_f64() + i2.to_f64())),
            (Self::Int(i1), Self::Int(i2)) => Self::add_int(*i1, *i2),
            (Self::Int(i1), Self::Float(i2)) => Some(Self::float(*i1 as f64 + i2.to_f64())),
            (Self::Float(i1), Self::Int(i2)) => Some(Self::float(i1.to_f64() + *i2 as f64)),
            _ => None,
        }
    }

    pub fn shift_left(&self, v2: &Self) -> Option<Self> {
        use std::convert::TryFrom;
        match (self, v2) {
            (Self::Int(i1), Self::Int(i2)) => {
                if *i2 < 0 {
                    None
                } else {
                    i32::try_from(*i2)
                        .ok()
                        .map(|i2| Self::Int(i1 << (i2 % 64) as u32))
                }
            }
            _ => None,
        }
    }

    // Arithmetic. For now, only on pure integer operands
    pub fn bitwise_or(&self, other: &Self) -> Option<Self> {
        match (self, other) {
            (Self::Int(i1), Self::Int(i2)) => Some(Self::Int(i1 | i2)),
            _ => None,
        }
    }

    // String concatenation
    pub fn concat(self, alloc: &'arena bumpalo::Bump, v2: Self) -> Option<Self> {
        fn safe_to_cast(t: &TypedValue<'_>) -> bool {
            matches!(
                t,
                TypedValue::Int(_) | TypedValue::String(_) | TypedValue::LazyClass(_)
            )
        }
        if !safe_to_cast(&self) || !safe_to_cast(&v2) {
            return None;
        }

        use std::convert::TryInto;
        let s1: Option<std::string::String> = self.try_into().ok();
        let s2: Option<std::string::String> = v2.try_into().ok();
        match (s1, s2) {
            (Some(l), Some(r)) => Some(Self::String((alloc.alloc_str(&(l + &r)) as &str).into())),
            _ => None,
        }
    }

    // Bitwise operations.
    pub fn bitwise_not(&self, _alloc: &'arena bumpalo::Bump) -> Option<Self> {
        match self {
            Self::Int(i) => Some(Self::Int(!i)),
            _ => None,
        }
    }

    #[allow(clippy::should_implement_trait)]
    pub fn not(self) -> Option<Self> {
        let b: bool = self.into();
        Some(Self::Bool(!b))
    }

    pub fn cast_to_string(self, alloc: &'arena bumpalo::Bump) -> Option<Self> {
        use std::convert::TryInto;
        WithBump(alloc, self).try_into().ok().map(Self::String)
    }

    pub fn cast_to_int(self) -> Option<Self> {
        use std::convert::TryInto;
        self.try_into().ok().map(Self::Int)
    }

    pub fn cast_to_bool(self) -> Option<Self> {
        Some(Self::Bool(self.into()))
    }

    pub fn cast_to_float(self) -> Option<Self> {
        use std::convert::TryInto;
        self.try_into().ok().map(Self::float)
    }

    pub fn string(s: impl AsRef<str>, alloc: &'arena bumpalo::Bump) -> Self {
        Self::String((alloc.alloc_str(s.as_ref()) as &str).into())
    }

    pub fn float(f: f64) -> Self {
        Self::Float(f.into())
    }
}

#[cfg(test)]
mod typed_value_tests {
    use super::TypedValue;
    use std::convert::TryInto;

    #[test]
    fn non_numeric_string_to_int() {
        let alloc = bumpalo::Bump::new();
        let res: Option<i64> = TypedValue::mk_string(alloc.alloc_str("foo"))
            .try_into()
            .ok();
        assert!(res.is_none());
    }

    #[test]
    fn nan_to_int() {
        let res: i64 = TypedValue::float(std::f64::NAN).try_into().unwrap();
        assert_eq!(res, std::i64::MIN);
    }
}

#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhbc_runtime<'arena>(
    _: TypedValue<'arena>,
) {
}
