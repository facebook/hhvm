// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Slice;
use hhvm_types_ffi::ffi::Attr;
use serde::Serialize;

use crate::Attribute;
use crate::ClassName;
use crate::Span;
use crate::TypeInfo;
use crate::TypedValue;

#[derive(Clone, Debug, Serialize)]
#[repr(C)]
pub struct Typedef<'arena> {
    pub name: ClassName<'arena>,
    pub attributes: Slice<'arena, Attribute<'arena>>,
    pub type_info: TypeInfo<'arena>,
    pub type_structure: TypedValue<'arena>,
    pub span: Span,
    pub attrs: Attr,
}
