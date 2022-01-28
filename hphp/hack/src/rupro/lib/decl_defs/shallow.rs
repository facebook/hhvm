// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::decl_defs::DeclTy;
use crate::pos::PosId;
use crate::reason::Reason;

#[derive(Debug)]
pub struct ShallowFun<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Typing_defs.fun_elt`
    pub pos: R::Pos,
    pub ty: DeclTy<R>,
}

#[derive(Debug)]
pub struct ShallowMethod<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Shallow_decl_defs.shallow_method`
    pub name: PosId<R::Pos>,
    pub ty: DeclTy<R>,
    pub visibility: oxidized::ast_defs::Visibility,
}

#[derive(Debug)]
pub struct ShallowClass<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Shallow_decl_defs.shallow_class`
    pub name: PosId<R::Pos>,
    pub extends: Vec<DeclTy<R>>,
    pub methods: Vec<ShallowMethod<R>>,
    pub static_methods: Vec<ShallowMethod<R>>,
    pub module: Option<PosId<R::Pos>>,
}
