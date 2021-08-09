// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Maybe, Str};
use hhbc_by_ref_runtime::TypedValue;

#[derive(Debug)]
#[repr(C)]
pub struct HhasTypeConstant<'arena> {
    pub name: Str<'arena>,
    pub initializer: Maybe<TypedValue<'arena>>,
    pub is_abstract: bool,
}

// For cbindgen
#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhas_type_const<'arena>(
    _: HhasTypeConstant<'arena>,
) {
    unimplemented!()
}
