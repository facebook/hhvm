// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Slice;
use serde::Serialize;

use crate::Attribute;
use crate::ClassName;
use crate::Span;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Module<'arena> {
    pub attributes: Slice<'arena, Attribute<'arena>>,
    pub name: ClassName<'arena>,
    pub span: Span,
}
