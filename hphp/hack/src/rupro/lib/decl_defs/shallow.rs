// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::decl_defs::{ClassishKind, DeclTy, Tparam, UserAttribute};
use crate::reason::Reason;
use hcons::Hc;
use pos::{Positioned, Symbol};

#[derive(Debug)]
pub struct ShallowFun<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Typing_defs.fun_elt`
    pub pos: R::Pos,
    pub ty: DeclTy<R>,
}

#[derive(Debug)]
pub struct ShallowMethod<R: Reason> {
    // note(sf, 2022-01-27):
    //   - c.f.
    //     - `Shallow_decl_defs.shallow_method`
    //     - `oxidized_by_ref::shallow_decl_defs::ShallowMethod<'_>`
    pub name: Positioned<Symbol, R::Pos>,
    pub ty: DeclTy<R>,
    pub visibility: oxidized::ast_defs::Visibility,
    pub deprecated: Option<Hc<str>>, // e.g. "The method foo is deprecated: ..."
    pub flags: oxidized_by_ref::method_flags::MethodFlags,
    pub attributes: Vec<UserAttribute<R>>,
}

#[derive(Debug)]
pub struct ShallowClass<R: Reason> {
    // note(sf, 2022-01-27):
    //  - c.f.
    //    - `Shallow_decl_defs.shallow_class`
    //    - `oxidized_by_ref::shallow_decl_defs::ShallowClass<'_>`
    pub mode: oxidized::file_info::Mode,
    pub is_final: bool,
    pub is_abstract: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: ClassishKind,
    pub module: Option<Positioned<Symbol, R::Pos>>,
    pub name: Positioned<Symbol, R::Pos>,
    pub tparams: Vec<Tparam<R>>,
    pub extends: Vec<DeclTy<R>>,
    pub methods: Vec<ShallowMethod<R>>,
    pub static_methods: Vec<ShallowMethod<R>>,
}
