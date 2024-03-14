// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use serde::Serialize;

use crate::typed_value::TypedValue;
use crate::StringId;

#[derive(Debug, Clone, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct TypeConstant {
    pub name: StringId,
    pub initializer: Maybe<TypedValue>,
    pub is_abstract: bool,
}
