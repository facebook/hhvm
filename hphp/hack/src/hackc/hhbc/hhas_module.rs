// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Slice;
use hhas_attribute::HhasAttribute;
use hhas_pos::HhasSpan;
use hhbc_id::class::ClassType;

#[derive(Debug)]
#[repr(C)]
pub struct HhasModule<'arena> {
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub name: ClassType<'arena>,
    pub span: HhasSpan,
}
