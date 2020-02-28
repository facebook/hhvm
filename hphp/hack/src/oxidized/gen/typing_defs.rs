// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<3049cda5c93aa0748313a536c9919dc1>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs_core::*;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct ClassElt {
    pub abstract_: bool,
    pub final_: bool,
    pub xhp_attr: Option<XhpAttr>,
    /// This field has different meanings in shallow mode and eager mode:
    /// In shallow mode, true if this method has attribute __Override.
    /// In eager mode, true if this method is originally defined in a trait,
    /// AND has the override attribute, AND the trait does not inherit any
    /// other method of that name.
    pub override_: bool,
    /// true if this static property has attribute __LSB
    pub lsb: bool,
    /// true if this method has attribute __MemoizeLSB
    pub memoizelsb: bool,
    /// true if this elt arose from require-extends or other mechanisms
    /// of hack "synthesizing" methods that were not written by the
    /// programmer. The eventual purpose of this is to make sure that
    /// elts that *are* written by the programmer take precedence over
    /// synthesized elts.
    pub synthesized: bool,
    pub visibility: Visibility,
    pub const_: bool,
    pub lateinit: bool,
    pub type_: lazy::Lazy<DeclTy>,
    /// identifies the class from which this elt originates
    pub origin: String,
    pub deprecated: Option<String>,
    pub pos: lazy::Lazy<pos::Pos>,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct FunElt {
    pub deprecated: Option<String>,
    pub type_: DeclTy,
    pub decl_errors: Option<errors::Errors>,
    pub pos: pos::Pos,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct ClassConst {
    pub synthesized: bool,
    pub abstract_: bool,
    pub pos: pos::Pos,
    pub type_: DeclTy,
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
#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct Requirement(pub pos::Pos, pub DeclTy);

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
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
    pub tparams: Vec<DeclTparam>,
    pub where_constraints: Vec<DeclWhereConstraint>,
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
    pub ancestors: s_map::SMap<DeclTy>,
    pub req_ancestors: Vec<Requirement>,
    /// the extends of req_ancestors
    pub req_ancestors_extends: s_set::SSet,
    pub extends: s_set::SSet,
    pub enum_type: Option<EnumType>,
    pub sealed_whitelist: Option<s_set::SSet>,
    pub decl_errors: Option<errors::Errors>,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum TypeconstAbstractKind {
    TCAbstract(Option<DeclTy>),
    TCPartiallyAbstract,
    TCConcrete,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct TypeconstType {
    pub abstract_: TypeconstAbstractKind,
    pub name: nast::Sid,
    pub constraint: Option<DeclTy>,
    pub type_: Option<DeclTy>,
    pub origin: String,
    pub enforceable: (pos::Pos, bool),
    pub reifiable: Option<pos::Pos>,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct PuEnumType {
    pub name: nast::Sid,
    pub is_final: bool,
    pub case_types: s_map::SMap<(nast::Sid, aast::ReifyKind)>,
    pub case_values: s_map::SMap<(nast::Sid, DeclTy)>,
    pub members: s_map::SMap<PuMemberType>,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct PuMemberType {
    pub atom: nast::Sid,
    pub types: s_map::SMap<(nast::Sid, DeclTy)>,
    pub exprs: s_map::SMap<nast::Sid>,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct EnumType {
    pub base: DeclTy,
    pub constraint: Option<DeclTy>,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum RecordFieldReq {
    ValueRequired,
    HasDefaultValue,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct RecordDefType {
    pub name: nast::Sid,
    pub extends: Option<nast::Sid>,
    pub fields: Vec<(nast::Sid, RecordFieldReq)>,
    pub abstract_: bool,
    pub pos: pos::Pos,
    pub errors: Option<errors::Errors>,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct TypedefType {
    pub pos: pos::Pos,
    pub vis: aast::TypedefVisibility,
    pub tparams: Vec<DeclTparam>,
    pub constraint: Option<DeclTy>,
    pub type_: DeclTy,
    pub decl_errors: Option<errors::Errors>,
}

pub type DeclTparam = Tparam<DeclTy>;

pub type DeclWhereConstraint = WhereConstraint<DeclTy>;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
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
