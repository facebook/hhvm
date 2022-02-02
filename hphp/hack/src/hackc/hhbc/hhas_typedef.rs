// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Slice;
use hhas_attribute::HhasAttribute;
use hhas_pos::HhasSpan;
use hhas_type::HhasTypeInfo;
use hhbc_id::class::ClassType;
use hhvm_types_ffi::ffi::Attr;
use runtime::TypedValue;

#[derive(Debug)]
#[repr(C)]
pub struct HhasTypedef<'arena> {
    pub name: ClassType<'arena>,
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub type_info: HhasTypeInfo<'arena>,
    pub type_structure: TypedValue<'arena>,
    pub span: HhasSpan,
    pub attrs: Attr,
}
