// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<4fff3bc54e4589868cc1dea891714aa7>>
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
    /// This field is used for two different meanings in two different places...
    /// enum class A:arraykey {int X="a";} -- here X.scc_type=\HH\MemberOf<A,int>
    /// enum B:int as arraykey {X="a"; Y=1; Z=B::X;} -- here X.scc_type=string, Y.scc_type=int, Z.scc_type=TAny
    /// In the later case, the scc_type is just a simple syntactic attempt to retrieve the type from the initializer.
    pub type_: &'a Ty<'a>,
    /// This is a list of all scope-resolution operators "A::B" that are mentioned in the const initializer,
    /// for members of regular-enums and enum-class-enums to detect circularity of initializers.
    /// We don't yet have a similar mechanism for top-level const initializers.
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
    pub name: typing_defs::PosId<'a>,
    pub kind: typing_defs::Typeconst<'a>,
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
    pub xhp_enum_values: s_map::SMap<'a, &'a [ast_defs::XhpEnumValue<'a>]>,
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
