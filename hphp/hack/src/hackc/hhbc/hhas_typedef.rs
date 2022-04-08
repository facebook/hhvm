// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    hhas_attribute::HhasAttribute, hhas_pos::HhasSpan, hhas_type::HhasTypeInfo,
    typed_value::TypedValue, ClassName,
};
use ffi::Slice;
use hhvm_types_ffi::ffi::Attr;

#[derive(Debug)]
#[repr(C)]
pub struct HhasTypedef<'arena> {
    pub name: ClassName<'arena>,
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub type_info: HhasTypeInfo<'arena>,
    pub type_structure: TypedValue<'arena>,
    pub span: HhasSpan,
    pub attrs: Attr,
}
