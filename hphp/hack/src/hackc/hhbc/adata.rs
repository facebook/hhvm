// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;
use serde::Serialize;

use crate::typed_value::TypedValue;

#[derive(Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Adata<'arena> {
    pub id: Str<'arena>,
    pub value: TypedValue<'arena>,
}

impl Adata<'_> {
    pub const VEC_PREFIX: &'static str = "v";
    pub const DICT_PREFIX: &'static str = "D";
    pub const KEYSET_PREFIX: &'static str = "k";
}
