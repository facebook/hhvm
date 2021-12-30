// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::{Maybe, Slice, Str};
use hhas_pos::HhasSpan;
use hhas_type::HhasTypeInfo;
use hhbc_id::record::RecordType;
use runtime::TypedValue;

#[derive(Debug)]
#[repr(C)]
pub struct Field<'arena>(
    pub Str<'arena>,
    pub HhasTypeInfo<'arena>,
    pub Maybe<TypedValue<'arena>>,
);

#[derive(Debug)]
#[repr(C)]
pub struct HhasRecord<'arena> {
    pub name: RecordType<'arena>,
    pub is_abstract: bool,
    pub base: Maybe<RecordType<'arena>>,
    pub fields: Slice<'arena, Field<'arena>>,
    pub span: HhasSpan,
}
