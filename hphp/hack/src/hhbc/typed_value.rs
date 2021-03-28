// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
extern crate lazy_static;

use std::{
    convert::{Into, TryFrom, TryInto},
    num::{FpCategory, Wrapping},
};

mod float {
    use std::num::FpCategory;

    #[derive(Clone, Copy, Debug, Hash, PartialOrd, Ord, PartialEq, Eq)]
    pub struct F64([u8; 8]);

    impl F64 {
        pub fn to_f64(&self) -> f64 {
            (*self).into()
        }

        pub fn classify(&self) -> FpCategory {
            self.to_f64().classify()
        }
    }

    impl From<f64> for F64 {
        fn from(f: f64) -> Self {
            F64(f.to_be_bytes())
        }
    }

    impl Into<f64> for F64 {
        fn into(self) -> f64 {
            f64::from_be_bytes(self.0)
        }
    }
}

/// We introduce a type for Hack/PHP values, mimicking what happens at runtime.
/// Currently this is used for constant folding. By defining a special type, we
/// ensure independence from usage: for example, it can be used for optimization
/// on ASTs, or on bytecode, or (in future) on a compiler intermediate language.
/// HHVM takes a similar approach: see runtime/base/typed-value.h
#[derive(Clone, Debug, Eq, Hash, PartialEq, PartialOrd, Ord)]
pub enum TypedValue {
    /// Used for fields that are initialized in the 86pinit method
    Uninit,
    /// Hack/PHP integers are 64-bit
    Int(i64),
    Bool(bool),
    /// Both Hack/PHP and Caml floats are IEEE754 64-bit
    Float(float::F64),
    String(String),
    LazyClass(String),
    Null,
    // Classic PHP arrays with explicit (key,value) entries
    HhasAdata(String),
    // Hack arrays: vectors, keysets, and dictionaries
    Vec(Vec<TypedValue>),
    Keyset(Vec<TypedValue>),
    Dict(Vec<(TypedValue, TypedValue)>),
}

mod string_ops {
    pub fn bitwise_not(s: &str) -> String {
        let s = s.as_bytes();
        let len = s.len();
        let mut res = vec![0; len];
        for i in 0..len {
            // keep only last byte
            res[i] = (!s[i]) & 0xFF;
        }
        // The "~" operator in Hack will create invalid utf-8 strings
        unsafe { String::from_utf8_unchecked(res) }
    }
}

/// Cast to a boolean: the (bool) operator in PHP
impl From<TypedValue> for bool {
    fn from(x: TypedValue) -> bool {
        match x {
            TypedValue::Uninit => false, // Should not happen
            TypedValue::Bool(b) => b,
            TypedValue::Null => false,
            TypedValue::String(s) => s != "" && s != "0",
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
impl TryFrom<TypedValue> for i64 {
    type Error = CastError;
    fn try_from(x: TypedValue) -> Result<i64, Self::Error> {
        match x {
            TypedValue::Uninit => Err(()), // Should not happen
            // Unreachable - the only calliste of to_int is cast_to_arraykey, which never
            // calls it with String
            TypedValue::String(_) => Err(()),    // not worth it
            TypedValue::LazyClass(_) => Err(()), // not worth it
            TypedValue::Int(i) => Ok(i),
            TypedValue::Float(f) => match f.classify() {
                FpCategory::Nan | FpCategory::Infinite => {
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
impl TryFrom<TypedValue> for f64 {
    type Error = CastError;
    fn try_from(v: TypedValue) -> Result<f64, Self::Error> {
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
impl TryFrom<TypedValue> for String {
    type Error = CastError;
    fn try_from(x: TypedValue) -> Result<String, Self::Error> {
        match x {
            TypedValue::Uninit => Err(()), // Should not happen
            TypedValue::Bool(false) => Ok("".into()),
            TypedValue::Bool(true) => Ok("1".into()),
            TypedValue::Null => Ok("".into()),
            TypedValue::Int(i) => Ok(i.to_string()),
            TypedValue::String(s) => Ok(s),
            TypedValue::LazyClass(s) => Ok(s),
            _ => Err(()),
        }
    }
}

impl TypedValue {
    // Integer operations. For now, we don't attempt to implement the
    // overflow-to-float semantics
    fn add_int(i1: i64, i2: i64) -> Option<TypedValue> {
        Some(Self::Int((Wrapping(i1) + Wrapping(i2)).0))
    }

    pub fn neg(&self) -> Option<TypedValue> {
        match self {
            Self::Int(i) => Some(Self::Int((-Wrapping(*i)).0)),
            Self::Float(i) => Some(Self::float(0.0 - i.to_f64())),

            _ => None,
        }
    }

    fn sub_int(i1: i64, i2: i64) -> Option<TypedValue> {
        Some(Self::Int((Wrapping(i1) - Wrapping(i2)).0))
    }

    // Arithmetic. For now, only on pure integer or float operands
    pub fn sub(&self, v2: &TypedValue) -> Option<TypedValue> {
        match (self, v2) {
            (Self::Int(i1), Self::Int(i2)) => Self::sub_int(*i1, *i2),
            (Self::Float(f1), Self::Float(f2)) => Some(Self::float(f1.to_f64() - f2.to_f64())),
            _ => None,
        }
    }

    fn mul_int(i1: i64, i2: i64) -> Option<TypedValue> {
        Some(Self::Int((Wrapping(i1) * Wrapping(i2)).0))
    }

    // Arithmetic. For now, only on pure integer or float operands
    pub fn mul(&self, other: &TypedValue) -> Option<TypedValue> {
        match (self, other) {
            (Self::Int(i1), Self::Int(i2)) => Self::mul_int(*i1, *i2),
            (Self::Float(i1), Self::Float(i2)) => Some(Self::float(i1.to_f64() * i2.to_f64())),
            (Self::Int(i1), Self::Float(i2)) => Some(Self::float(*i1 as f64 * i2.to_f64())),
            (Self::Float(i1), Self::Int(i2)) => Some(Self::float(i1.to_f64() * *i2 as f64)),
            _ => None,
        }
    }

    // Arithmetic. For now, only on pure integer or float operands
    pub fn div(&self, v2: &TypedValue) -> Option<TypedValue> {
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
    pub fn add(&self, other: &TypedValue) -> Option<TypedValue> {
        match (self, other) {
            (Self::Float(i1), Self::Float(i2)) => Some(Self::float(i1.to_f64() + i2.to_f64())),
            (Self::Int(i1), Self::Int(i2)) => Self::add_int(*i1, *i2),
            (Self::Int(i1), Self::Float(i2)) => Some(Self::float(*i1 as f64 + i2.to_f64())),
            (Self::Float(i1), Self::Int(i2)) => Some(Self::float(i1.to_f64() + *i2 as f64)),
            _ => None,
        }
    }

    pub fn shift_left(&self, v2: &TypedValue) -> Option<TypedValue> {
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
    pub fn bitwise_or(&self, other: &TypedValue) -> Option<TypedValue> {
        match (self, other) {
            (Self::Int(i1), Self::Int(i2)) => Some(Self::Int(i1 | i2)),
            _ => None,
        }
    }

    // String concatenation
    pub fn concat(self, v2: TypedValue) -> Option<TypedValue> {
        fn safe_to_cast(t: &TypedValue) -> bool {
            matches!(
                t,
                TypedValue::Int(_) | TypedValue::String(_) | TypedValue::LazyClass(_)
            )
        }
        if !safe_to_cast(&self) || !safe_to_cast(&v2) {
            return None;
        }

        let s1: Option<String> = self.try_into().ok();
        let s2: Option<String> = v2.try_into().ok();
        match (s1, s2) {
            (Some(l), Some(r)) => Some(Self::String(l + &r)),
            _ => None,
        }
    }

    // Bitwise operations.
    pub fn bitwise_not(&self) -> Option<TypedValue> {
        match self {
            Self::Int(i) => Some(Self::Int(!i)),
            Self::String(s) => Some(Self::String(string_ops::bitwise_not(s))),
            _ => None,
        }
    }

    pub fn not(self) -> Option<TypedValue> {
        let b: bool = self.into();
        Some(Self::Bool(!b))
    }

    pub fn cast_to_string(self) -> Option<TypedValue> {
        self.try_into().ok().map(|x| Self::String(x))
    }

    pub fn cast_to_int(self) -> Option<TypedValue> {
        self.try_into().ok().map(|x| Self::Int(x))
    }
    pub fn cast_to_bool(self) -> Option<TypedValue> {
        Some(Self::Bool(self.into()))
    }

    pub fn cast_to_float(self) -> Option<TypedValue> {
        self.try_into().ok().map(|x| Self::float(x))
    }

    pub fn cast_to_arraykey(self) -> Option<TypedValue> {
        match self {
            TypedValue::String(s) => Some(Self::String(s)),
            TypedValue::Null => Some(Self::String("".into())),
            TypedValue::Uninit
            | TypedValue::Vec(_)
            | TypedValue::Keyset(_)
            | TypedValue::Dict(_) => None,
            _ => Self::cast_to_int(self),
        }
    }

    pub fn string(s: impl Into<String>) -> TypedValue {
        Self::String(s.into())
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
        let res: Option<i64> = TypedValue::String("foo".to_string()).try_into().ok();
        assert!(res.is_none());
    }

    #[test]
    fn nan_to_int() {
        let res: i64 = TypedValue::float(std::f64::NAN).try_into().unwrap();
        assert_eq!(res, std::i64::MIN);
    }

    #[test]
    fn inf_to_int() {
        let res: i64 = TypedValue::float(std::f64::INFINITY).try_into().unwrap();
        assert_eq!(res, 0);
    }

    #[test]
    fn neg_inf_to_int() {
        let res: i64 = TypedValue::float(std::f64::NEG_INFINITY)
            .try_into()
            .unwrap();
        assert_eq!(res, std::i64::MIN);
    }

    #[test]
    fn false_to_int() {
        let res: i64 = TypedValue::Bool(false).try_into().unwrap();
        assert_eq!(res, 0);
    }

    #[test]
    fn true_to_int() {
        let res: i64 = TypedValue::Bool(true).try_into().unwrap();
        assert_eq!(res, 1);
    }

    #[test]
    fn non_numeric_string_to_float() {
        let res: Option<f64> = TypedValue::String("foo".to_string()).try_into().ok();
        assert!(res.is_none());
    }

    #[test]
    fn int_wrapping_add() {
        let res = TypedValue::Int(0x7FFFFFFFFFFFFFFF)
            .add(&TypedValue::Int(1))
            .unwrap();
        assert_eq!(res, TypedValue::Int(-9223372036854775808));
    }

    #[test]
    fn int_wrapping_neg() {
        let res = TypedValue::Int(1 << 63).neg().unwrap();
        assert_eq!(res, TypedValue::Int(-9223372036854775808));
    }

    #[test]
    fn int_wrapping_sub() {
        let res = TypedValue::Int(1 << 63).sub(&TypedValue::Int(1)).unwrap();
        assert_eq!(res, TypedValue::Int(9223372036854775807));
    }

    #[test]
    fn int_wrapping_mul() {
        let res = TypedValue::Int(0x80000001)
            .mul(&TypedValue::Int(-0xffffffff))
            .unwrap();
        assert_eq!(res, TypedValue::Int(9223372034707292161));
    }

    #[test]
    fn negative_shift_left() {
        let res = TypedValue::Int(3).shift_left(&TypedValue::Int(-1));
        assert_eq!(res, None);
    }

    #[test]
    fn big_shift_left() {
        let res = TypedValue::Int(1).shift_left(&TypedValue::Int(70)).unwrap();
        assert_eq!(res, TypedValue::Int(64));
    }

    #[test]
    fn eq() {
        assert_eq!(TypedValue::Bool(true), TypedValue::Bool(true));
        assert_ne!(TypedValue::Int(2), TypedValue::Int(3));
        assert_eq!(
            TypedValue::String("foo".to_string()),
            TypedValue::String("foo".to_string())
        );
        assert_eq!(
            TypedValue::float(std::f64::NAN),
            TypedValue::float(std::f64::NAN)
        );
    }

    #[test]
    fn overflowing_shift_left() {
        let res = TypedValue::Int(1).shift_left(&TypedValue::Int(0xffffffffffff));
        assert_eq!(res, None);
    }

    /*     #[test]
    fn eq() {
        assert_eq!(TypedValue::Bool(true), TypedValue::Bool(true));
    } */
}
