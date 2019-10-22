// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[macro_use]
extern crate lazy_static;

use std::convert::{Into, TryInto};

/// We introduce a type for Hack/PHP values, mimicking what happens at runtime.
/// Currently this is used for constant folding. By defining a special type, we
/// ensure independence from usage: for example, it can be used for optimization
/// on ASTs, or on bytecode, or (in future) on a compiler intermediate language.
/// HHVM takes a similar approach: see runtime/base/typed-value.h
#[derive(Clone, Debug)]
pub enum TypedValue {
    /// Used for fields that are initialized in the 86pinit method
    Uninit,
    /// Hack/PHP integers are 64-bit
    Int(i64),
    Bool(bool),
    /// Both Hack/PHP and Caml floats are IEEE754 64-bit
    Float(f64),
    String(String),
    Null,
    // Classic PHP arrays with explicit (key,value) entries
    HhasAdata(String),
    Array(Vec<(TypedValue, TypedValue)>),
    VArray(Vec<Self>),
    DArray(Vec<(TypedValue, TypedValue)>),
    // Hack arrays: vectors, keysets, and dictionaries
    // TODO(hrust) add after adding deps to oxidized & OCaml runtime
}

lazy_static! {
    static ref ZERO: TypedValue = TypedValue::Int(0);
    pub static ref NULL: TypedValue = TypedValue::Null;
}

/// Cast to a boolean: the (bool) operator in PHP
impl Into<bool> for TypedValue {
    fn into(self) -> bool {
        match self {
            TypedValue::Uninit => false, // Should not happen
            TypedValue::Bool(b) => b,
            TypedValue::Null => false,
            TypedValue::String(s) => s != "" && s != "0",
            TypedValue::Int(i) => i != 0,
            TypedValue::Float(f) => f != 0.0,
            // Empty collections cast to false if empty, otherwise true
            TypedValue::Array(v) => !v.is_empty(),
            TypedValue::VArray(v) => !v.is_empty(),
            TypedValue::DArray(v) => !v.is_empty(),
            // Non-empty collections cast to true
            TypedValue::HhasAdata(_) => true,
        }
    }
}

pub type CastError = String;

/// Cast to an integer: the (int) operator in PHP. Return Err if we can't
/// or won't produce the correct value
impl TryInto<isize> for TypedValue {
    type Error = CastError;
    fn try_into(self) -> Result<isize, Self::Error> {
        Err("not implemented".into()) // TODO(hrust)
    }
}

/// Cast to a float: the (float) operator in PHP. Return Err if we can't
/// or won't produce the correct value
impl TryInto<f64> for TypedValue {
    type Error = CastError;
    fn try_into(self) -> Result<f64, Self::Error> {
        Err("not implemented".into()) // TODO(hrust)
    }
}

/// Cast to a string: the (string) operator in PHP. Return Err if we can't
/// or won't produce the correct value *)
impl TryInto<String> for TypedValue {
    type Error = CastError;
    fn try_into(self) -> Result<String, Self::Error> {
        Err("not implemented".into()) // TODO(hrust) blocked by string_utils
    }
}

// TODO(hrust) group remaining funcs into modules: int_ops, string_ops, etc.
#[allow(dead_code)]
impl TypedValue {
    fn as_string(&self) -> Result<TypedValue, CastError> {
        Err("not implemented".into())
    }

    fn as_int(&self) -> Result<TypedValue, CastError> {
        Err("not implemented".into())
    }
}
