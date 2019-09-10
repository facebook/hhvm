// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c05285ae47a26ae9a5907a55e102030a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;

use crate::aast;
use crate::ast_defs;
use crate::decl_defs;
use crate::errors;
use crate::file_info;
use crate::i_set;
use crate::nast;
use crate::pos;

use crate::typing_defs::*;

#[derive(Clone, Debug, OcamlRep)]
pub struct ShallowClassConst {
    pub abstract_: bool,
    pub expr: Option<nast::Expr>,
    pub name: aast::Sid,
    pub type_: Ty,
    pub visibility: aast::Visibility,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct ShallowTypeconst {
    pub abstract_: TypeconstAbstractKind,
    pub constraint: Option<Ty>,
    pub name: aast::Sid,
    pub type_: Option<Ty>,
    pub enforceable: (pos::Pos, bool),
    pub visibility: aast::Visibility,
    pub reifiable: Option<pos::Pos>,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct ShallowPuMember {
    pub atom: aast::Sid,
    pub types: Vec<(aast::Sid, Ty)>,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct ShallowPuEnum {
    pub name: aast::Sid,
    pub is_final: bool,
    pub case_types: Vec<aast::Sid>,
    pub case_values: Vec<(aast::Sid, Ty)>,
    pub members: Vec<ShallowPuMember>,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct ShallowProp {
    pub const_: bool,
    pub xhp_attr: Option<XhpAttr>,
    pub lateinit: bool,
    pub lsb: bool,
    pub name: aast::Sid,
    pub needs_init: bool,
    pub type_: Option<Ty>,
    pub abstract_: bool,
    pub visibility: aast::Visibility,
    pub fixme_codes: i_set::ISet,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct ShallowMethod {
    pub abstract_: bool,
    pub final_: bool,
    pub memoizelsb: bool,
    pub name: aast::Sid,
    pub override_: bool,
    pub reactivity: Option<decl_defs::MethodReactivity>,
    pub type_: FunType,
    pub visibility: aast::Visibility,
    pub fixme_codes: i_set::ISet,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct ShallowMethodRedeclaration {
    pub abstract_: bool,
    pub final_: bool,
    pub static_: bool,
    pub name: aast::Sid,
    pub type_: FunType,
    pub visibility: aast::Visibility,
    pub trait_: aast::Hint,
    pub method: aast::Pstring,
    pub fixme_codes: i_set::ISet,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct ShallowClass {
    pub mode: file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub kind: ast_defs::ClassKind,
    pub name: aast::Sid,
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    pub extends: Vec<Ty>,
    pub uses: Vec<Ty>,
    pub method_redeclarations: Vec<ShallowMethodRedeclaration>,
    pub xhp_attr_uses: Vec<Ty>,
    pub req_extends: Vec<Ty>,
    pub req_implements: Vec<Ty>,
    pub implements: Vec<Ty>,
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
