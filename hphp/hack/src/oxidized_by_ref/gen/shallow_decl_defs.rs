// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<92014d20b22d860e20a10736cc8fb897>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowClassConst<'a> {
    pub abstract_: bool,
    pub name: ast_defs::Id<'a>,
    pub type_: Ty<'a>,
}
impl<'a> TrivialDrop for ShallowClassConst<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowTypeconst<'a> {
    pub abstract_: TypeconstAbstractKind<'a>,
    pub constraint: Option<Ty<'a>>,
    pub name: ast_defs::Id<'a>,
    pub type_: Option<Ty<'a>>,
    pub enforceable: (&'a pos::Pos<'a>, bool),
    pub reifiable: Option<&'a pos::Pos<'a>>,
}
impl<'a> TrivialDrop for ShallowTypeconst<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowPuMember<'a> {
    pub atom: ast_defs::Id<'a>,
    pub types: &'a [(ast_defs::Id<'a>, Ty<'a>)],
    pub exprs: &'a [ast_defs::Id<'a>],
}
impl<'a> TrivialDrop for ShallowPuMember<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowPuEnum<'a> {
    pub name: ast_defs::Id<'a>,
    pub is_final: bool,
    pub case_types: &'a [&'a Tparam<'a>],
    pub case_values: &'a [(ast_defs::Id<'a>, Ty<'a>)],
    pub members: &'a [&'a ShallowPuMember<'a>],
}
impl<'a> TrivialDrop for ShallowPuEnum<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowProp<'a> {
    pub const_: bool,
    pub xhp_attr: Option<XhpAttr>,
    pub lateinit: bool,
    pub lsb: bool,
    pub name: ast_defs::Id<'a>,
    pub needs_init: bool,
    pub type_: Option<Ty<'a>>,
    pub abstract_: bool,
    pub visibility: oxidized::ast_defs::Visibility,
}
impl<'a> TrivialDrop for ShallowProp<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowMethod<'a> {
    pub abstract_: bool,
    pub final_: bool,
    pub memoizelsb: bool,
    pub name: ast_defs::Id<'a>,
    pub override_: bool,
    pub dynamicallycallable: bool,
    pub reactivity: Option<decl_defs::MethodReactivity<'a>>,
    pub type_: Ty<'a>,
    pub visibility: oxidized::ast_defs::Visibility,
    pub deprecated: Option<&'a str>,
}
impl<'a> TrivialDrop for ShallowMethod<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowClass<'a> {
    pub mode: oxidized::file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassKind,
    pub name: ast_defs::Id<'a>,
    pub tparams: &'a [&'a Tparam<'a>],
    pub where_constraints: &'a [&'a WhereConstraint<'a>],
    pub extends: &'a [Ty<'a>],
    pub uses: &'a [Ty<'a>],
    pub xhp_attr_uses: &'a [Ty<'a>],
    pub req_extends: &'a [Ty<'a>],
    pub req_implements: &'a [Ty<'a>],
    pub implements: &'a [Ty<'a>],
    pub consts: &'a [&'a ShallowClassConst<'a>],
    pub typeconsts: &'a [&'a ShallowTypeconst<'a>],
    pub pu_enums: &'a [&'a ShallowPuEnum<'a>],
    pub props: &'a [&'a ShallowProp<'a>],
    pub sprops: &'a [&'a ShallowProp<'a>],
    pub constructor: Option<&'a ShallowMethod<'a>>,
    pub static_methods: &'a [&'a ShallowMethod<'a>],
    pub methods: &'a [&'a ShallowMethod<'a>],
    pub user_attributes: &'a [&'a UserAttribute<'a>],
    pub enum_type: Option<&'a EnumType<'a>>,
}
impl<'a> TrivialDrop for ShallowClass<'a> {}
