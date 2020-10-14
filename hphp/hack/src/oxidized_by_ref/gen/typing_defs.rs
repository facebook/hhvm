// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<64785250bfd8e7e1772943966e746c35>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs_flags::*;

pub use typing_defs_core::*;

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
pub struct PuOrigin<'a> {
    pub class: &'a str,
    pub enum_: &'a str,
}
impl<'a> TrivialDrop for PuOrigin<'a> {}

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
pub struct ConstDecl<'a> {
    pub pos: &'a pos::Pos<'a>,
    pub type_: &'a Ty<'a>,
}
impl<'a> TrivialDrop for ConstDecl<'a> {}

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
pub struct ClassElt<'a> {
    pub visibility: CeVisibility<'a>,
    pub type_: &'a lazy::Lazy<&'a Ty<'a>>,
    /// identifies the class from which this elt originates
    pub origin: &'a str,
    pub deprecated: Option<&'a str>,
    pub pos: &'a lazy::Lazy<&'a pos::Pos<'a>>,
    pub flags: isize,
}
impl<'a> TrivialDrop for ClassElt<'a> {}

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
pub struct FunElt<'a> {
    pub deprecated: Option<&'a str>,
    pub type_: &'a Ty<'a>,
    pub pos: &'a pos::Pos<'a>,
    pub php_std_lib: bool,
}
impl<'a> TrivialDrop for FunElt<'a> {}

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
pub struct ClassConst<'a> {
    pub synthesized: bool,
    pub abstract_: bool,
    pub pos: &'a pos::Pos<'a>,
    pub type_: &'a Ty<'a>,
    /// identifies the class from which this const originates
    pub origin: &'a str,
}
impl<'a> TrivialDrop for ClassConst<'a> {}

/// The position is that of the hint in the `use` / `implements` AST node
/// that causes a class to have this requirement applied to it. E.g.
///
/// ```
/// class Foo {}
///
/// interface Bar {
///   require extends Foo; <- position of the decl_phase ty
/// }
///
/// class Baz extends Foo implements Bar { <- position of the `implements`
/// }
/// ```
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
pub struct Requirement<'a>(pub &'a pos::Pos<'a>, pub &'a Ty<'a>);
impl<'a> TrivialDrop for Requirement<'a> {}

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
pub struct ClassType<'a> {
    pub need_init: bool,
    /// Whether the typechecker knows of all (non-interface) ancestors
    /// and thus known all accessible members of this class
    pub members_fully_known: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub const_: bool,
    /// When a class is abstract (or in a trait) the initialization of
    /// a protected member can be delayed
    pub deferred_init_members: s_set::SSet<'a>,
    pub kind: oxidized::ast_defs::ClassKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub is_disposable: bool,
    pub name: &'a str,
    pub pos: &'a pos::Pos<'a>,
    pub tparams: &'a [&'a Tparam<'a>],
    pub where_constraints: &'a [&'a WhereConstraint<'a>],
    pub consts: s_map::SMap<'a, &'a ClassConst<'a>>,
    pub typeconsts: s_map::SMap<'a, &'a TypeconstType<'a>>,
    pub pu_enums: s_map::SMap<'a, &'a PuEnumType<'a>>,
    pub props: s_map::SMap<'a, &'a ClassElt<'a>>,
    pub sprops: s_map::SMap<'a, &'a ClassElt<'a>>,
    pub methods: s_map::SMap<'a, &'a ClassElt<'a>>,
    pub smethods: s_map::SMap<'a, &'a ClassElt<'a>>,
    /// the consistent_kind represents final constructor or __ConsistentConstruct
    pub construct: (Option<&'a ClassElt<'a>>, ConsistentKind),
    /// This includes all the classes, interfaces and traits this class is
    /// using.
    pub ancestors: s_map::SMap<'a, &'a Ty<'a>>,
    pub req_ancestors: &'a [&'a Requirement<'a>],
    /// the extends of req_ancestors
    pub req_ancestors_extends: s_set::SSet<'a>,
    pub extends: s_set::SSet<'a>,
    pub enum_type: Option<&'a EnumType<'a>>,
    pub sealed_whitelist: Option<s_set::SSet<'a>>,
    pub decl_errors: Option<&'a errors::Errors<'a>>,
}
impl<'a> TrivialDrop for ClassType<'a> {}

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
pub enum TypeconstAbstractKind<'a> {
    TCAbstract(Option<&'a Ty<'a>>),
    TCPartiallyAbstract,
    TCConcrete,
}
impl<'a> TrivialDrop for TypeconstAbstractKind<'a> {}

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
pub struct TypeconstType<'a> {
    pub abstract_: TypeconstAbstractKind<'a>,
    pub name: nast::Sid<'a>,
    pub constraint: Option<&'a Ty<'a>>,
    pub type_: Option<&'a Ty<'a>>,
    pub origin: &'a str,
    pub enforceable: (&'a pos::Pos<'a>, bool),
    pub reifiable: Option<&'a pos::Pos<'a>>,
}
impl<'a> TrivialDrop for TypeconstType<'a> {}

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
pub struct PuEnumType<'a> {
    pub name: nast::Sid<'a>,
    pub is_final: bool,
    pub case_types: s_map::SMap<'a, (&'a PuOrigin<'a>, &'a Tparam<'a>)>,
    pub case_values: s_map::SMap<'a, (&'a PuOrigin<'a>, nast::Sid<'a>, &'a Ty<'a>)>,
    pub members: s_map::SMap<'a, &'a PuMemberType<'a>>,
}
impl<'a> TrivialDrop for PuEnumType<'a> {}

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
pub struct PuMemberType<'a> {
    pub atom: nast::Sid<'a>,
    pub origin: &'a PuOrigin<'a>,
    pub types: s_map::SMap<'a, (&'a PuOrigin<'a>, nast::Sid<'a>, &'a Ty<'a>)>,
    pub exprs: s_map::SMap<'a, (&'a PuOrigin<'a>, nast::Sid<'a>)>,
}
impl<'a> TrivialDrop for PuMemberType<'a> {}

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
pub struct EnumType<'a> {
    pub base: &'a Ty<'a>,
    pub constraint: Option<&'a Ty<'a>>,
    pub includes: &'a [&'a Ty<'a>],
    pub enum_class: bool,
}
impl<'a> TrivialDrop for EnumType<'a> {}

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum RecordFieldReq {
    ValueRequired,
    HasDefaultValue,
}
impl TrivialDrop for RecordFieldReq {}

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
pub struct RecordDefType<'a> {
    pub name: nast::Sid<'a>,
    pub extends: Option<nast::Sid<'a>>,
    pub fields: &'a [(nast::Sid<'a>, RecordFieldReq)],
    pub abstract_: bool,
    pub pos: &'a pos::Pos<'a>,
}
impl<'a> TrivialDrop for RecordDefType<'a> {}

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
pub struct TypedefType<'a> {
    pub pos: &'a pos::Pos<'a>,
    pub vis: oxidized::aast::TypedefVisibility,
    pub tparams: &'a [&'a Tparam<'a>],
    pub constraint: Option<&'a Ty<'a>>,
    pub type_: &'a Ty<'a>,
}
impl<'a> TrivialDrop for TypedefType<'a> {}

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
pub enum DeserializationError<'a> {
    /// The type was valid, but some component thereof was a decl_ty when we
    /// expected a locl_phase ty, or vice versa.
    WrongPhase(&'a str),
    /// The specific type or some component thereof is not one that we support
    /// deserializing, usually because not enough information was serialized to be
    /// able to deserialize it again.
    NotSupported(&'a str),
    /// The input JSON was invalid for some reason.
    DeserializationError(&'a str),
}
impl<'a> TrivialDrop for DeserializationError<'a> {}
