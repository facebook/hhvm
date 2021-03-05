// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_type::Info;
use hhbc_by_ref_hhbc_id::record;
use hhbc_by_ref_runtime::TypedValue;

#[derive(Debug)]
pub struct Field<'a, 'arena>(pub &'a str, pub Info, pub Option<TypedValue<'arena>>);

#[derive(Debug)]
pub struct HhasRecord<'a, 'arena> {
    pub name: record::Type<'arena>,
    pub is_abstract: bool,
    pub base: Option<record::Type<'arena>>,
    pub fields: Vec<Field<'a, 'arena>>,
    pub span: Span,
}
