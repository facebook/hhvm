// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use runtime::TypedValue;

#[derive(Debug)]
pub struct HhasTypeConstant {
    pub name: String,
    pub initializer: Option<TypedValue>,
}
