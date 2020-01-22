// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<99504a1f17208ef7035c26c55acc2559>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::aast;
use crate::ast_defs;
use crate::errors;
use crate::lazy;
use crate::nast;
use crate::pos;
use crate::s_map;
use crate::s_set;
use crate::typing_defs_core;

pub use typing_defs_core::ConsistentKind;
pub use typing_defs_core::DeclFunArity;
pub use typing_defs_core::DeclFunParam;
pub use typing_defs_core::DeclFunParams;
pub use typing_defs_core::DeclFunType;
pub use typing_defs_core::DeclPossiblyEnforcedTy;
pub use typing_defs_core::DeclTy;
pub use typing_defs_core::DependentType;
pub use typing_defs_core::DestructureKind;
pub use typing_defs_core::Exact;
pub use typing_defs_core::FunArity;
pub use typing_defs_core::FunParam;
pub use typing_defs_core::FunParams;
pub use typing_defs_core::FunTparamsKind;
pub use typing_defs_core::FunType;
pub use typing_defs_core::ParamMode;
pub use typing_defs_core::ParamMutability;
pub use typing_defs_core::ParamRxAnnotation;
pub use typing_defs_core::PossiblyEnforcedTy;
pub use typing_defs_core::Reactivity;
pub use typing_defs_core::ShapeFieldType;
pub use typing_defs_core::ShapeKind;
pub use typing_defs_core::TaccessType;
pub use typing_defs_core::Tparam;
pub use typing_defs_core::Ty;
pub use typing_defs_core::Ty_;
pub use typing_defs_core::ValKind;
pub use typing_defs_core::Visibility;
pub use typing_defs_core::WhereConstraint;
pub use typing_defs_core::XhpAttr;
pub use typing_defs_core::XhpAttrTag;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct ClassElt {
    pub abstract_: bool,
    pub final_: bool,
    pub xhp_attr: Option<XhpAttr>,
    pub override_: bool,
    pub lsb: bool,
    pub memoizelsb: bool,
    pub synthesized: bool,
    pub visibility: Visibility,
    pub const_: bool,
    pub lateinit: bool,
    pub type_: lazy::Lazy<DeclTy>,
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
    pub origin: String,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct Requirement(pub pos::Pos, pub DeclTy);

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct ClassType {
    pub need_init: bool,
    pub members_fully_known: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub const_: bool,
    pub ppl: bool,
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
    pub construct: (Option<ClassElt>, ConsistentKind),
    pub ancestors: s_map::SMap<DeclTy>,
    pub req_ancestors: Vec<Requirement>,
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
    WrongPhase(String),
    NotSupported(String),
    DeserializationError(String),
}
