// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;

use crate::func::SrcLoc;
use crate::Attribute;
use crate::ClassId;

pub struct Module<'a> {
    pub attributes: Vec<Attribute<'a>>,
    pub name: ClassId,
    pub src_loc: SrcLoc,
    pub doc_comment: Option<Str<'a>>,
}
