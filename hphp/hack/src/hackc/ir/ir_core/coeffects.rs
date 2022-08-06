// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;
pub use naming_special_names_rust::coeffects::Ctx;

#[derive(Debug)]
pub struct CtxConstant<'a> {
    pub name: Str<'a>,
    pub recognized: Vec<Str<'a>>,
    pub unrecognized: Vec<Str<'a>>,
    pub is_abstract: bool,
}

#[derive(Debug)]
pub struct Coeffects<'a> {
    pub static_coeffects: Vec<Ctx>,
    pub unenforced_static_coeffects: Vec<Str<'a>>,
    pub fun_param: Vec<usize>,
    pub cc_param: Vec<(usize, Str<'a>)>,
    pub cc_this: Vec<Vec<Str<'a>>>,
    pub cc_reified: Vec<(bool, usize, Vec<Str<'a>>)>,
    pub closure_parent_scope: bool,
    pub generator_this: bool,
    pub caller: bool,
}
