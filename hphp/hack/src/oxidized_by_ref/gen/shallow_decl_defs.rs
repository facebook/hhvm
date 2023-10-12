// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<92dca76e3b1b47a538eef9cae24c30b5>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::ConstDecl;
pub use typing_defs::*;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "scc_")]
#[repr(C)]
pub struct ShallowClassConst<'a> {
    pub abstract_: oxidized::typing_defs::ClassConstKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: typing_defs::PosId<'a>,
    /// This field is used for two different meanings in two different places...
    /// enum class A:arraykey {int X="a";} -- here X.scc_type=\HH\MemberOf<A,int>
    /// enum B:int as arraykey {X="a"; Y=1; Z=B::X;} -- here X.scc_type=string, Y.scc_type=int, Z.scc_type=TAny
    /// In the later case, the scc_type is just a simple syntactic attempt to retrieve the type from the initializer.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
    /// This is a list of all scope-resolution operators "A::B" that are mentioned in the const initializer,
    /// for members of regular-enums and enum-class-enums to detect circularity of initializers.
    /// We don't yet have a similar mechanism for top-level const initializers.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub refs: &'a [typing_defs::ClassConstRef<'a>],
}
impl<'a> TrivialDrop for ShallowClassConst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShallowClassConst<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "stc_")]
#[repr(C)]
pub struct ShallowTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: typing_defs::PosId<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: typing_defs::Typeconst<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub enforceable: (&'a pos_or_decl::PosOrDecl<'a>, bool),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub reifiable: Option<&'a pos_or_decl::PosOrDecl<'a>>,
    pub is_ctx: bool,
}
impl<'a> TrivialDrop for ShallowTypeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShallowTypeconst<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "sp_")]
#[repr(C)]
pub struct ShallowProp<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: typing_defs::PosId<'a>,
    pub xhp_attr: Option<XhpAttr>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
    pub visibility: oxidized::ast_defs::Visibility,
    pub flags: prop_flags::PropFlags,
}
impl<'a> TrivialDrop for ShallowProp<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShallowProp<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "sm_")]
#[repr(C)]
pub struct ShallowMethod<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: typing_defs::PosId<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
    pub visibility: oxidized::ast_defs::Visibility,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub deprecated: Option<&'a str>,
    pub flags: method_flags::MethodFlags,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub attributes: &'a [&'a UserAttribute<'a>],
}
impl<'a> TrivialDrop for ShallowMethod<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShallowMethod<'arena>);

#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type XhpEnumValues<'a> = s_map::SMap<'a, &'a [ast_defs::XhpEnumValue<'a>]>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "sc_")]
#[repr(C)]
pub struct ShallowClass<'a> {
    pub mode: oxidized::file_info::Mode,
    pub final_: bool,
    pub abstract_: bool,
    pub is_xhp: bool,
    pub internal: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassishKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<ast_defs::Id<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: typing_defs::PosId<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub extends: &'a [&'a Ty<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub uses: &'a [&'a Ty<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attr_uses: &'a [&'a Ty<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_enum_values: s_map::SMap<'a, &'a [ast_defs::XhpEnumValue<'a>]>,
    pub xhp_marked_empty: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub req_extends: &'a [&'a Ty<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub req_implements: &'a [&'a Ty<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub req_class: &'a [&'a Ty<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub implements: &'a [&'a Ty<'a>],
    pub support_dynamic_type: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub consts: &'a [&'a ShallowClassConst<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub typeconsts: &'a [&'a ShallowTypeconst<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub props: &'a [&'a ShallowProp<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub sprops: &'a [&'a ShallowProp<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constructor: Option<&'a ShallowMethod<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub static_methods: &'a [&'a ShallowMethod<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub methods: &'a [&'a ShallowMethod<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub enum_type: Option<&'a EnumType<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub docs_url: Option<&'a str>,
}
impl<'a> TrivialDrop for ShallowClass<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShallowClass<'arena>);

#[rust_to_ocaml(attr = "deriving show")]
pub type FunDecl<'a> = FunElt<'a>;

#[rust_to_ocaml(attr = "deriving show")]
pub type ClassDecl<'a> = ShallowClass<'a>;

#[rust_to_ocaml(attr = "deriving show")]
pub type TypedefDecl<'a> = TypedefType<'a>;

#[rust_to_ocaml(attr = "deriving show")]
pub type ModuleDecl<'a> = typing_defs::ModuleDefType<'a>;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum Decl<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Class(&'a ClassDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Fun(&'a FunDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Typedef(&'a TypedefDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Const(&'a ConstDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Module(&'a ModuleDecl<'a>),
}
impl<'a> TrivialDrop for Decl<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Decl<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum NamedDecl<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    NClass(&'a (&'a str, &'a ClassDecl<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    NFun(&'a (&'a str, &'a FunDecl<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    NTypedef(&'a (&'a str, &'a TypedefDecl<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    NConst(&'a (&'a str, &'a ConstDecl<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    NModule(&'a (&'a str, &'a ModuleDecl<'a>)),
}
impl<'a> TrivialDrop for NamedDecl<'a> {}
arena_deserializer::impl_deserialize_in_arena!(NamedDecl<'arena>);
