// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<4d1d105e7d935ede08ec287483e5143c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;

use crate::aast;
use crate::ast_defs;
use crate::decl_defs;
use crate::errors;
use crate::file_info;
use crate::i_set;
use crate::nast;
use crate::pos;

use crate::typing_defs::*;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShallowClassConst {
    pub abstract_: bool,
    pub expr: Option<nast::Expr>,
    pub name: aast::Sid,
    pub type_: DeclTy,
    pub visibility: aast::Visibility,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShallowTypeconst {
    pub abstract_: TypeconstAbstractKind,
    pub constraint: Option<DeclTy>,
    pub name: aast::Sid,
    pub type_: Option<DeclTy>,
    pub enforceable: (pos::Pos, bool),
    pub visibility: aast::Visibility,
    pub reifiable: Option<pos::Pos>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShallowPuMember {
    pub atom: aast::Sid,
    pub types: Vec<(aast::Sid, DeclTy)>,
    pub exprs: Vec<aast::Sid>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShallowPuEnum {
    pub name: aast::Sid,
    pub is_final: bool,
    pub case_types: Vec<aast::Sid>,
    pub case_values: Vec<(aast::Sid, DeclTy)>,
    pub members: Vec<ShallowPuMember>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShallowProp {
    pub const_: bool,
    pub xhp_attr: Option<XhpAttr>,
    pub lateinit: bool,
    pub lsb: bool,
    pub name: aast::Sid,
    pub needs_init: bool,
    pub type_: Option<DeclTy>,
    pub abstract_: bool,
    pub visibility: aast::Visibility,
    pub fixme_codes: i_set::ISet,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShallowMethod {
    pub abstract_: bool,
    pub final_: bool,
    pub memoizelsb: bool,
    pub name: aast::Sid,
    pub override_: bool,
    pub reactivity: Option<decl_defs::MethodReactivity>,
    pub type_: DeclFunType,
    pub visibility: aast::Visibility,
    pub fixme_codes: i_set::ISet,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShallowMethodRedeclaration {
    pub abstract_: bool,
    pub final_: bool,
    pub static_: bool,
    pub name: aast::Sid,
    pub type_: DeclFunType,
    pub visibility: aast::Visibility,
    pub trait_: aast::Hint,
    pub method: aast::Pstring,
    pub fixme_codes: i_set::ISet,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShallowClass {
    pub mode: file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub kind: ast_defs::ClassKind,
    pub name: aast::Sid,
    pub tparams: Vec<DeclTparam>,
    pub where_constraints: Vec<DeclWhereConstraint>,
    pub extends: Vec<DeclTy>,
    pub uses: Vec<DeclTy>,
    pub method_redeclarations: Vec<ShallowMethodRedeclaration>,
    pub xhp_attr_uses: Vec<DeclTy>,
    pub req_extends: Vec<DeclTy>,
    pub req_implements: Vec<DeclTy>,
    pub implements: Vec<DeclTy>,
    pub consts: Vec<ShallowClassConst>,
    pub typeconsts: Vec<ShallowTypeconst>,
    pub pu_enums: Vec<ShallowPuEnum>,
    pub props: Vec<ShallowProp>,
    pub sprops: Vec<ShallowProp>,
    pub constructor: Option<ShallowMethod>,
    pub static_methods: Vec<ShallowMethod>,
    pub methods: Vec<ShallowMethod>,
    pub user_attributes: Vec<nast::UserAttribute>,
    pub enum_type: Option<EnumType>,
    pub decl_errors: errors::Errors,
}
