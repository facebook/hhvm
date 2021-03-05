// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_type as hhas_type;
use hhbc_by_ref_hhbc_id as hhas_id;
use hhbc_by_ref_runtime::TypedValue;

#[derive(Debug)]
pub struct Typedef<'arena> {
    pub name: hhas_id::class::Type<'arena>,
    pub attributes: Vec<HhasAttribute<'arena>>,
    pub type_info: hhas_type::Info,
    pub type_structure: TypedValue<'arena>,
    pub span: Span,
}
