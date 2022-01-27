// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::decl_defs::DeclTy;
use crate::pos::PosId;
use crate::reason::Reason;

#[derive(Debug)]
pub struct ShallowFun<R: Reason> {
    pub pos: R::Pos,
    pub ty: DeclTy<R>,
}

#[derive(Debug)]
pub struct ShallowMethod<R: Reason> {
    pub name: PosId<R::Pos>,
    pub ty: DeclTy<R>,
    pub visibility: oxidized::ast_defs::Visibility,
}

#[derive(Debug)]
pub struct ShallowClass<R: Reason> {
    pub name: PosId<R::Pos>,
    pub extends: Vec<DeclTy<R>>,
    pub methods: Vec<ShallowMethod<R>>,
    pub module: Option<PosId<R::Pos>>,
}
