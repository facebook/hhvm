// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d6ef01e9b2758851cec3fb309445af26>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub use typing_defs::ConstDecl;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowClassConst<'a> {
    pub abstract_: bool,
    pub name: typing_defs::PosId<'a>,
    pub type_: &'a Ty<'a>,
    pub refs: &'a [typing_defs::ClassConstRef<'a>],
}
impl<'a> TrivialDrop for ShallowClassConst<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowTypeconst<'a> {
    pub abstract_: TypeconstAbstractKind<'a>,
    pub as_constraint: Option<&'a Ty<'a>>,
    pub super_constraint: Option<&'a Ty<'a>>,
    pub name: typing_defs::PosId<'a>,
    pub type_: Option<&'a Ty<'a>>,
    pub enforceable: (&'a pos_or_decl::PosOrDecl<'a>, bool),
    pub reifiable: Option<&'a pos_or_decl::PosOrDecl<'a>>,
}
impl<'a> TrivialDrop for ShallowTypeconst<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowProp<'a> {
    pub name: typing_defs::PosId<'a>,
    pub xhp_attr: Option<XhpAttr>,
    pub type_: Option<&'a Ty<'a>>,
    pub visibility: oxidized::ast_defs::Visibility,
    pub flags: prop_flags::PropFlags,
}
impl<'a> TrivialDrop for ShallowProp<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShallowMethod<'a> {
    pub name: typing_defs::PosId<'a>,
    pub type_: &'a Ty<'a>,
    pub visibility: oxidized::ast_defs::Visibility,
    pub deprecated: Option<&'a str>,
    pub flags: method_flags::MethodFlags,
}
impl<'a> TrivialDrop for ShallowMethod<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
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
    pub name: typing_defs::PosId<'a>,
    pub tparams: &'a [&'a Tparam<'a>],
    pub where_constraints: &'a [&'a WhereConstraint<'a>],
    pub extends: &'a [&'a Ty<'a>],
    pub uses: &'a [&'a Ty<'a>],
    pub xhp_attr_uses: &'a [&'a Ty<'a>],
    pub req_extends: &'a [&'a Ty<'a>],
    pub req_implements: &'a [&'a Ty<'a>],
    pub implements: &'a [&'a Ty<'a>],
    pub implements_dynamic: bool,
    pub consts: &'a [&'a ShallowClassConst<'a>],
    pub typeconsts: &'a [&'a ShallowTypeconst<'a>],
    pub props: &'a [&'a ShallowProp<'a>],
    pub sprops: &'a [&'a ShallowProp<'a>],
    pub constructor: Option<&'a ShallowMethod<'a>>,
    pub static_methods: &'a [&'a ShallowMethod<'a>],
    pub methods: &'a [&'a ShallowMethod<'a>],
    pub user_attributes: &'a [&'a UserAttribute<'a>],
    pub enum_type: Option<&'a EnumType<'a>>,
}
impl<'a> TrivialDrop for ShallowClass<'a> {}

pub type FunDecl<'a> = FunElt<'a>;

pub type ClassDecl<'a> = ShallowClass<'a>;

pub type RecordDecl<'a> = RecordDefType<'a>;

pub type TypedefDecl<'a> = TypedefType<'a>;

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Decl<'a> {
    Class(&'a ClassDecl<'a>),
    Fun(&'a FunDecl<'a>),
    Record(&'a RecordDecl<'a>),
    Typedef(&'a TypedefDecl<'a>),
    Const(&'a ConstDecl<'a>),
}
impl<'a> TrivialDrop for Decl<'a> {}
