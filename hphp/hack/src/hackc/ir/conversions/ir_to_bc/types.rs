// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use hhbc::TypeInfo;

pub(crate) fn convert(ty: &ir::TypeInfo) -> Maybe<TypeInfo> {
    if ty.is_empty() {
        Maybe::Nothing
    } else {
        Maybe::Just(ty.clone())
    }
}
