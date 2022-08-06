// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;

use crate::TypedValue;

#[derive(Debug)]
pub struct TypeConstant<'a> {
    pub name: Str<'a>,
    pub initializer: Option<TypedValue<'a>>,
    pub is_abstract: bool,
}
