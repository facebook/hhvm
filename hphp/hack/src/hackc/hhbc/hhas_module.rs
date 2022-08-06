// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Slice;
use serde::Serialize;

use crate::hhas_attribute::HhasAttribute;
use crate::hhas_pos::HhasSpan;
use crate::ClassName;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct HhasModule<'arena> {
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub name: ClassName<'arena>,
    pub span: HhasSpan,
}
