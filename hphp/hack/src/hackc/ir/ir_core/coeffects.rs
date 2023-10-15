// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;

use crate::CcParam;
use crate::Ctx;

#[derive(Debug)]
pub struct CtxConstant<'a> {
    pub name: Str<'a>,
    pub recognized: Vec<Str<'a>>,
    pub unrecognized: Vec<Str<'a>>,
    pub is_abstract: bool,
}

#[derive(Clone, Default, Debug)]
pub struct Coeffects<'a> {
    pub static_coeffects: Vec<Ctx>,
    pub unenforced_static_coeffects: Vec<Str<'a>>,
    pub fun_param: Vec<u32>,
    pub cc_param: Vec<CcParam<'a>>,
    pub cc_this: Vec<CcThis<'a>>,
    pub cc_reified: Vec<CcReified<'a>>,
    pub closure_parent_scope: bool,
    pub generator_this: bool,
    pub caller: bool,
}

#[derive(Clone, Debug)]
pub struct CcThis<'arena> {
    pub types: Vec<Str<'arena>>,
}

#[derive(Clone, Debug)]
pub struct CcReified<'arena> {
    pub is_class: bool,
    pub index: u32,
    pub types: Vec<Str<'arena>>,
}
