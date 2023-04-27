// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Slice;
use ffi::Str;
use hhvm_types_ffi::ffi::Attr;
use serde::Serialize;

use crate::Attribute;
use crate::PropName;
use crate::TypeInfo;
use crate::TypedValue;
use crate::Visibility;

#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Property<'arena> {
    pub name: PropName<'arena>,
    pub flags: Attr,
    pub attributes: Slice<'arena, Attribute<'arena>>,
    pub visibility: Visibility,
    pub initial_value: Maybe<TypedValue<'arena>>,
    pub type_info: TypeInfo<'arena>,
    pub doc_comment: Maybe<Str<'arena>>,
}

impl<'arena> Property<'arena> {
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
