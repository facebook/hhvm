// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0891cc6db5e42414b49868bafc6b1ef9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ShallowClassConst {
    pub abstract_: bool,
    pub expr: Option<nast::Expr>,
    pub name: aast::Sid,
    pub type_: Ty,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ShallowTypeconst {
    pub abstract_: TypeconstAbstractKind,
    pub constraint: Option<Ty>,
    pub name: aast::Sid,
    pub type_: Option<Ty>,
    pub enforceable: (std::rc::Rc<pos::Pos>, bool),
    pub reifiable: Option<std::rc::Rc<pos::Pos>>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ShallowPuMember {
    pub atom: aast::Sid,
    pub types: Vec<(aast::Sid, Ty)>,
    pub exprs: Vec<aast::Sid>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ShallowPuEnum {
    pub name: aast::Sid,
    pub is_final: bool,
    pub case_types: Vec<Tparam>,
    pub case_values: Vec<(aast::Sid, Ty)>,
    pub members: Vec<ShallowPuMember>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ShallowProp {
    pub const_: bool,
    pub xhp_attr: Option<oxidized_by_ref::shallow_decl_defs::XhpAttr>,
    pub lateinit: bool,
    pub lsb: bool,
    pub name: aast::Sid,
    pub needs_init: bool,
    pub type_: Option<Ty>,
    pub abstract_: bool,
    pub visibility: oxidized::aast::Visibility,
    pub fixme_codes: i_set::ISet,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ShallowMethod {
    pub abstract_: bool,
    pub final_: bool,
    pub memoizelsb: bool,
    pub name: aast::Sid,
    pub override_: bool,
    pub dynamicallycallable: bool,
    pub reactivity: Option<decl_defs::MethodReactivity>,
    pub type_: Ty,
    pub visibility: oxidized::aast::Visibility,
    pub fixme_codes: i_set::ISet,
    pub deprecated: Option<std::rc::Rc<String>>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ShallowClass {
    pub mode: oxidized::file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassKind,
    pub name: aast::Sid,
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    pub extends: Vec<Ty>,
    pub uses: Vec<Ty>,
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
