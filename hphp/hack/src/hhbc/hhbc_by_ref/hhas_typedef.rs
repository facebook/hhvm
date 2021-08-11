// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Slice;
use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_type::HhasTypeInfo;
use hhbc_by_ref_hhbc_id::class::ClassType;
use hhbc_by_ref_runtime::TypedValue;

#[derive(Debug)]
pub struct Typedef<'arena> {
    pub name: ClassType<'arena>,
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub type_info: HhasTypeInfo<'arena>,
    pub type_structure: TypedValue<'arena>,
    pub span: Span,
}
