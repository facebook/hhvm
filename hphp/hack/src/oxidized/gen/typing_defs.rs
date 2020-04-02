// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<83d21fdf67ee988f0b4c9f38b05fd37e>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs_flags::*;

pub use typing_defs_core::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct ClassElt {
    pub xhp_attr: Option<XhpAttr>,
    pub dynamicallycallable: bool,
    pub visibility: Visibility,
    pub type_: lazy::Lazy<Ty>,
    /// identifies the class from which this elt originates
    pub origin: String,
    pub deprecated: Option<String>,
    pub pos: lazy::Lazy<pos::Pos>,
    pub flags: isize,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct FunElt {
    pub deprecated: Option<String>,
    pub type_: Ty,
    pub decl_errors: Option<errors::Errors>,
    pub pos: pos::Pos,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct ClassConst {
    pub synthesized: bool,
    pub abstract_: bool,
    pub pos: pos::Pos,
    pub type_: Ty,
    pub expr: Option<nast::Expr>,
    /// identifies the class from which this const originates
    pub origin: String,
}

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
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Requirement(pub pos::Pos, pub Ty);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct ClassType {
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
    pub deferred_init_members: s_set::SSet,
    pub kind: ast_defs::ClassKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub is_disposable: bool,
    pub name: String,
    pub pos: pos::Pos,
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    pub consts: s_map::SMap<ClassConst>,
    pub typeconsts: s_map::SMap<TypeconstType>,
    pub pu_enums: s_map::SMap<PuEnumType>,
    pub props: s_map::SMap<ClassElt>,
    pub sprops: s_map::SMap<ClassElt>,
    pub methods: s_map::SMap<ClassElt>,
    pub smethods: s_map::SMap<ClassElt>,
    /// the consistent_kind represents final constructor or __ConsistentConstruct
    pub construct: (Option<ClassElt>, ConsistentKind),
    /// This includes all the classes, interfaces and traits this class is
    /// using.
    pub ancestors: s_map::SMap<Ty>,
    pub req_ancestors: Vec<Requirement>,
    /// the extends of req_ancestors
    pub req_ancestors_extends: s_set::SSet,
    pub extends: s_set::SSet,
    pub enum_type: Option<EnumType>,
    pub sealed_whitelist: Option<s_set::SSet>,
    pub decl_errors: Option<errors::Errors>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum TypeconstAbstractKind {
    TCAbstract(Option<Ty>),
    TCPartiallyAbstract,
    TCConcrete,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct TypeconstType {
    pub abstract_: TypeconstAbstractKind,
    pub name: nast::Sid,
    pub constraint: Option<Ty>,
    pub type_: Option<Ty>,
    pub origin: String,
    pub enforceable: (pos::Pos, bool),
    pub reifiable: Option<pos::Pos>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct PuEnumType {
    pub name: nast::Sid,
    pub is_final: bool,
    pub case_types: s_map::SMap<(nast::Sid, aast::ReifyKind)>,
    pub case_values: s_map::SMap<(nast::Sid, Ty)>,
    pub members: s_map::SMap<PuMemberType>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct PuMemberType {
    pub atom: nast::Sid,
    pub types: s_map::SMap<(nast::Sid, Ty)>,
    pub exprs: s_map::SMap<nast::Sid>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct EnumType {
    pub base: Ty,
    pub constraint: Option<Ty>,
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum RecordFieldReq {
    ValueRequired,
    HasDefaultValue,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct RecordDefType {
    pub name: nast::Sid,
    pub extends: Option<nast::Sid>,
    pub fields: Vec<(nast::Sid, RecordFieldReq)>,
    pub abstract_: bool,
    pub pos: pos::Pos,
    pub errors: Option<errors::Errors>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct TypedefType {
    pub pos: pos::Pos,
    pub vis: aast::TypedefVisibility,
    pub tparams: Vec<Tparam>,
    pub constraint: Option<Ty>,
    pub type_: Ty,
    pub decl_errors: Option<errors::Errors>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum DeserializationError {
    /// The type was valid, but some component thereof was a decl_ty when we
    /// expected a locl_phase ty, or vice versa.
    WrongPhase(String),
    /// The specific type or some component thereof is not one that we support
    /// deserializing, usually because not enough information was serialized to be
    /// able to deserialize it again.
    NotSupported(String),
    /// The input JSON was invalid for some reason.
    DeserializationError(String),
}
