// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::{Str, Maybe};
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_type::Info;
use hhbc_by_ref_hhbc_id::record;
use hhbc_by_ref_runtime::TypedValue;

#[derive(Debug)]
pub struct Field<'arena>(pub Str<'arena>, pub Info<'arena>, pub Maybe<TypedValue<'arena>>);

#[derive(Debug)]
pub struct HhasRecord<'arena> {
    pub name: record::RecordType<'arena>,
    pub is_abstract: bool,
    pub base: Option<record::RecordType<'arena>>,
    pub fields: Vec<Field<'arena>>,
    pub span: Span,
}
