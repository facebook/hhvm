// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused)]
use crate::decl_defs::DeclTy;
use crate::pos::PosId;
use crate::reason::Reason;

#[derive(Debug)]
pub struct ShallowFun<REASON: Reason> {
    pub fe_pos: REASON::P,
    pub fe_type: DeclTy<REASON>,
}

#[derive(Debug)]
pub struct ShallowMethod<REASON: Reason> {
    pub sm_name: PosId<REASON::P>,
    pub sm_type: DeclTy<REASON>,
}

#[derive(Debug)]
pub struct ShallowClass<REASON: Reason> {
    pub sc_name: PosId<REASON::P>,
    pub sc_extends: Vec<DeclTy<REASON>>,
    pub sc_methods: Vec<ShallowMethod<REASON>>,
}
