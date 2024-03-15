// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;

pub(crate) fn convert_maybe_type(ty: Maybe<&hhbc::TypeInfo>) -> ir::TypeInfo {
    match ty {
        Maybe::Just(ty) => ty.clone(),
        Maybe::Nothing => ir::TypeInfo::empty(),
    }
}
