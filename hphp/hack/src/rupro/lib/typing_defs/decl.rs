// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::decl_defs::DeclTy;
use crate::reason::Reason;
use pos::Symbol;

#[allow(dead_code)]
#[derive(Debug)]
pub struct ClassElt<R: Reason> {
    ce_type: DeclTy<R>,
    ce_origin: Symbol,
    ce_pos: R::Pos,
}
