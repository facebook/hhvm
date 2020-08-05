// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0e98d2d87f023b7de9cf7cd2dad28192>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
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
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ClassElt<'a> {
    pub visibility: Visibility<'a>,
    pub type_: lazy::Lazy<Ty<'a>>,
    /// identifies the class from which this elt originates
    pub origin: &'a str,
    pub deprecated: Option<&'a str>,
    pub pos: lazy::Lazy<&'a pos::Pos<'a>>,
    pub flags: isize,
}
impl<'a> TrivialDrop for ClassElt<'a> {}

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
pub struct FunElt<'a> {
    pub deprecated: Option<&'a str>,
    pub type_: Ty<'a>,
    pub decl_errors: Option<errors::Errors<'a>>,
    pub pos: &'a pos::Pos<'a>,
}
impl<'a> TrivialDrop for FunElt<'a> {}

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
pub struct ClassConst<'a> {
    pub synthesized: bool,
    pub abstract_: bool,
    pub pos: &'a pos::Pos<'a>,
    pub type_: Ty<'a>,
    pub expr: Option<nast::Expr<'a>>,
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
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Requirement<'a>(pub &'a pos::Pos<'a>, pub Ty<'a>);
impl<'a> TrivialDrop for Requirement<'a> {}

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
pub struct ClassType<'a> {
    pub need_init: bool,
    /// Whether the typechecker knows of all (non-interface) ancestors
    /// and thus known all accessible members of this class
    pub members_fully_known: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub const_: bool,
    /// True when the class is annotated with the __PPL attribute.
    pub ppl: bool,
    /// When a class is abstract (or in a trait) the initialization of
    /// a protected member can be delayed
    pub deferred_init_members: s_set::SSet<'a>,
    pub kind: oxidized::ast_defs::ClassKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub is_disposable: bool,
    pub name: &'a str,
    pub pos: &'a pos::Pos<'a>,
    pub tparams: &'a [Tparam<'a>],
    pub where_constraints: &'a [WhereConstraint<'a>],
    pub consts: s_map::SMap<'a, ClassConst<'a>>,
    pub typeconsts: s_map::SMap<'a, TypeconstType<'a>>,
    pub pu_enums: s_map::SMap<'a, PuEnumType<'a>>,
    pub props: s_map::SMap<'a, ClassElt<'a>>,
    pub sprops: s_map::SMap<'a, ClassElt<'a>>,
    pub methods: s_map::SMap<'a, ClassElt<'a>>,
    pub smethods: s_map::SMap<'a, ClassElt<'a>>,
    /// the consistent_kind represents final constructor or __ConsistentConstruct
    pub construct: (Option<ClassElt<'a>>, ConsistentKind),
    /// This includes all the classes, interfaces and traits this class is
    /// using.
    pub ancestors: s_map::SMap<'a, Ty<'a>>,
    pub req_ancestors: &'a [Requirement<'a>],
    /// the extends of req_ancestors
    pub req_ancestors_extends: s_set::SSet<'a>,
    pub extends: s_set::SSet<'a>,
    pub enum_type: Option<EnumType<'a>>,
    pub sealed_whitelist: Option<s_set::SSet<'a>>,
    pub decl_errors: Option<errors::Errors<'a>>,
}
impl<'a> TrivialDrop for ClassType<'a> {}

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
pub enum TypeconstAbstractKind<'a> {
    TCAbstract(Option<Ty<'a>>),
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
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct TypeconstType<'a> {
    pub abstract_: TypeconstAbstractKind<'a>,
    pub name: nast::Sid<'a>,
    pub constraint: Option<Ty<'a>>,
    pub type_: Option<Ty<'a>>,
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
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct PuEnumType<'a> {
    pub name: nast::Sid<'a>,
    pub is_final: bool,
    pub case_types: s_map::SMap<'a, (PuOrigin<'a>, Tparam<'a>)>,
    pub case_values: s_map::SMap<'a, (PuOrigin<'a>, nast::Sid<'a>, Ty<'a>)>,
    pub members: s_map::SMap<'a, PuMemberType<'a>>,
}
impl<'a> TrivialDrop for PuEnumType<'a> {}

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
pub struct PuMemberType<'a> {
    pub atom: nast::Sid<'a>,
    pub origin: PuOrigin<'a>,
    pub types: s_map::SMap<'a, (PuOrigin<'a>, nast::Sid<'a>, Ty<'a>)>,
    pub exprs: s_map::SMap<'a, (PuOrigin<'a>, nast::Sid<'a>)>,
}
impl<'a> TrivialDrop for PuMemberType<'a> {}

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
pub struct EnumType<'a> {
    pub base: Ty<'a>,
    pub constraint: Option<Ty<'a>>,
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
    pub errors: Option<errors::Errors<'a>>,
}
impl<'a> TrivialDrop for RecordDefType<'a> {}

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
pub struct TypedefType<'a> {
    pub pos: &'a pos::Pos<'a>,
    pub vis: oxidized::aast::TypedefVisibility,
    pub tparams: &'a [Tparam<'a>],
    pub constraint: Option<Ty<'a>>,
    pub type_: Ty<'a>,
    pub decl_errors: Option<errors::Errors<'a>>,
}
impl<'a> TrivialDrop for TypedefType<'a> {}

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
