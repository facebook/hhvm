// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Str;
use serde::Serialize;

use crate::typed_value::TypedValue;

#[derive(Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct HhasTypeConstant<'arena> {
    pub name: Str<'arena>,
    pub initializer: Maybe<TypedValue<'arena>>,
    pub is_abstract: bool,
}
