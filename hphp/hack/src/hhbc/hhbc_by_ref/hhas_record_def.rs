// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::{Maybe, Slice, Str};
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_type::Info;
use hhbc_by_ref_hhbc_id::record::RecordType;
use hhbc_by_ref_runtime::TypedValue;

#[derive(Debug)]
#[repr(C)]
pub struct Field<'arena>(
    pub Str<'arena>,
    pub Info<'arena>,
    pub Maybe<TypedValue<'arena>>,
);

#[derive(Debug)]
#[repr(C)]
pub struct HhasRecord<'arena> {
    pub name: RecordType<'arena>,
    pub is_abstract: bool,
    pub base: Maybe<RecordType<'arena>>,
    pub fields: Slice<'arena, Field<'arena>>,
    pub span: Span,
}

// For cbindgen
#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhas_record_def<'arena>(
    _: HhasRecord<'arena>,
) {
    unimplemented!()
}
