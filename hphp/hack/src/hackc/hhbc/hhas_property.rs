// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    hhas_attribute::HhasAttribute, hhas_type::HhasTypeInfo, hhbc_ast::Visibility, hhbc_id,
    typed_value::TypedValue,
};
use ffi::{Maybe, Slice, Str};
use hhvm_types_ffi::ffi::Attr;

#[derive(Debug)]
#[repr(C)]
pub struct HhasProperty<'arena> {
    pub name: hhbc_id::prop::PropType<'arena>,
    pub flags: Attr,
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub visibility: Visibility,
    pub initial_value: Maybe<TypedValue<'arena>>,
    pub type_info: HhasTypeInfo<'arena>,
    pub doc_comment: Maybe<Str<'arena>>,
}

impl<'arena> HhasProperty<'arena> {
    pub fn is_private(&self) -> bool {
        self.visibility == Visibility::Private
    }
    pub fn is_protected(&self) -> bool {
        self.visibility == Visibility::Protected
    }
    pub fn is_public(&self) -> bool {
        self.visibility == Visibility::Public
    }
}
