// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::Attribute;
use crate::ModuleName;
use crate::SrcLoc;

#[derive(Debug)]
pub struct Module {
    pub attributes: Vec<Attribute>,
    pub name: ModuleName,
    pub src_loc: SrcLoc,
    pub doc_comment: Option<Vec<u8>>,
}
