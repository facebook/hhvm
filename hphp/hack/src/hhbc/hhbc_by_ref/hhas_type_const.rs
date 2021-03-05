// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_runtime::TypedValue;

#[derive(Debug)]
pub struct HhasTypeConstant<'arena> {
    pub name: String,
    pub initializer: Option<TypedValue<'arena>>,
    pub is_abstract: bool,
}
