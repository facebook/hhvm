// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::decl_defs::DeclTy;
use crate::pos::PosId;
use crate::reason::Reason;

#[derive(Debug)]
pub struct ShallowFun<R: Reason> {
    pub fe_pos: R::Pos,
    pub fe_type: DeclTy<R>,
}

#[derive(Debug)]
pub struct ShallowMethod<R: Reason> {
    pub sm_name: PosId<R::Pos>,
    pub sm_type: DeclTy<R>,
    pub sm_visibility: oxidized::ast_defs::Visibility,
}

#[derive(Debug)]
pub struct ShallowClass<R: Reason> {
    pub sc_name: PosId<R::Pos>,
    pub sc_extends: Vec<DeclTy<R>>,
    pub sc_methods: Vec<ShallowMethod<R>>,
}
