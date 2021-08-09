// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::{Maybe, Str};
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_type::Info;
use hhbc_by_ref_hhbc_id::record;
use hhbc_by_ref_runtime::TypedValue;

#[derive(Debug)]
#[repr(C)]
pub struct Field<'arena>(
    pub Str<'arena>,
    pub Info<'arena>,
    pub Maybe<TypedValue<'arena>>,
);

#[derive(Debug)]
pub struct HhasRecord<'arena> {
    pub name: record::RecordType<'arena>,
    pub is_abstract: bool,
    pub base: Option<record::RecordType<'arena>>,
    pub fields: Vec<Field<'arena>>,
    pub span: Span,
}

// For cbindgen
#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhas_record_def<'arena>(_: Field<'arena>) {
    unimplemented!()
}
