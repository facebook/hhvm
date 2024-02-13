// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::func::SrcLoc;
use crate::Attribute;
use crate::ModuleId;

#[derive(Debug)]
pub struct Module {
    pub attributes: Vec<Attribute>,
    pub name: ModuleId,
    pub src_loc: SrcLoc,
    pub doc_comment: Option<Vec<u8>>,
}
