// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::Str;
use hhbc_by_ref_runtime::TypedValue;

pub const VARRAY_PREFIX: &str = "y";
pub const DARRAY_PREFIX: &str = "Y";
pub const VEC_PREFIX: &str = "v";
pub const DICT_PREFIX: &str = "D";
pub const KEYSET_PREFIX: &str = "k";

#[derive(Debug)]
#[repr(C)]
pub struct HhasAdata<'arena> {
    pub id: Str<'arena>,
    pub value: TypedValue<'arena>,
}

// For cbindgen
#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhas_adata<'arena>(_: HhasAdata<'arena>) {
    unimplemented!()
}
