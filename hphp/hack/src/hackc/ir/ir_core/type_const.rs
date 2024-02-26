// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::StringId;
use crate::TypedValue;

#[derive(Debug)]
pub struct TypeConstant {
    pub name: StringId,
    pub initializer: Option<TypedValue>,
    pub is_abstract: bool,
}
