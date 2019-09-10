// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<942d78d146202dd31f6efe3094e3516c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;

use crate::aast;
use crate::ast_defs;
use crate::errors;
use crate::ident;
use crate::lazy;
use crate::nast;
use crate::pos;
use crate::s_map;
use crate::s_set;
use crate::tany_sentinel;

pub use crate::typing_reason as reason;

#[derive(Clone, Debug, OcamlRep)]
pub enum Visibility {
    Vpublic,
    Vprivate(String),
    Vprotected(String),
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum Exact {
    Exact,
    Nonexact,
}

#[derive(Clone, Debug, OcamlRep)]
pub enum PuKind {
    PuPlain,
    PuAtom(String),
}

#[derive(Clone, Debug, OcamlRep)]
pub struct Ty(pub reason::Reason, pub Box<Ty_>);

#[derive(Clone, Debug, OcamlRep)]
pub struct ShapeFieldType {
    pub optional: bool,
    pub ty: Ty,
}

#[derive(Clone, Debug, OcamlRep)]
pub enum Ty_ {
    Tthis,
    Tapply(nast::Sid, Vec<Ty>),
    Tgeneric(String),
    Taccess(TaccessType),
    Tarray(Option<Ty>, Option<Ty>),
    Tdarray(Ty, Ty),
    Tvarray(Ty),
    TvarrayOrDarray(Ty),
    Tmixed,
    Tnothing,
    Tlike(Ty),
    Tany(tany_sentinel::TanySentinel),
    Terr,
    Tnonnull,
    Tdynamic,
    Toption(Ty),
    Tprim(aast::Tprim),
    Tfun(FunType),
    Ttuple(Vec<Ty>),
    Tshape(ShapeKind, nast::shape_map::ShapeMap<ShapeFieldType>),
    TpuAccess(Ty, nast::Sid),
    Tvar(ident::Ident),
}

#[derive(Clone, Debug, OcamlRep)]
pub enum ArrayKind {
    AKany,
    AKvarray(Ty),
    AKdarray(Ty, Ty),
    AKvarrayOrDarray(Ty),
    AKvec(Ty),
    AKmap(Ty, Ty),
    AKempty,
}

#[derive(Clone, Debug, OcamlRep)]
pub enum AbstractKind {
    AKnewtype(String, Vec<Ty>),
    AKgeneric(String),
    AKdependent(DependentType),
}

#[derive(Clone, Debug, OcamlRep)]
pub enum DependentType {
    DTthis,
    DTcls(String),
    DTexpr(ident::Ident),
}

#[derive(Clone, Debug, OcamlRep)]
pub struct TaccessType(pub Ty, pub Vec<nast::Sid>);

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum ShapeKind {
    ClosedShape,
    OpenShape,
}

#[derive(Clone, Debug, OcamlRep)]
pub enum Reactivity {
    Nonreactive,
    Local(Option<Ty>),
    Shallow(Option<Ty>),
    Reactive(Option<Ty>),
    MaybeReactive(Box<Reactivity>),
    RxVar(Option<Box<Reactivity>>),
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum ValKind {
    Lval,
    LvalSubexpr,
    Other,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum ParamMutability {
    ParamOwnedMutable,
    ParamBorrowedMutable,
    ParamMaybeMutable,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum FunTparamsKind {
    FTKtparams,
    FTKinstantiatedTargs,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct FunType {
    pub pos: pos::Pos,
    pub deprecated: Option<String>,
    pub abstract_: bool,
    pub is_coroutine: bool,
    pub arity: FunArity,
    pub tparams: (Vec<Tparam>, FunTparamsKind),
    pub where_constraints: Vec<WhereConstraint>,
    pub params: FunParams,
    pub ret: PossiblyEnforcedTy,
    pub fun_kind: ast_defs::FunKind,
    pub reactive: Reactivity,
    pub return_disposable: bool,
    pub mutability: Option<ParamMutability>,
    pub returns_mutable: bool,
    pub decl_errors: Option<errors::Errors>,
    pub returns_void_to_rx: bool,
}

#[derive(Clone, Debug, OcamlRep)]
pub enum FunArity {
    Fstandard(isize, isize),
    Fvariadic(isize, FunParam),
    Fellipsis(isize, pos::Pos),
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum ParamMode {
    FPnormal,
    FPref,
    FPinout,
}

#[derive(Clone, Debug, OcamlRep)]
pub enum ParamRxAnnotation {
    ParamRxVar,
    ParamRxIfImpl(Ty),
}

#[derive(Clone, Debug, OcamlRep)]
pub struct PossiblyEnforcedTy {
    pub enforced: bool,
    pub type_: Ty,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct FunParam {
    pub pos: pos::Pos,
    pub name: Option<String>,
    pub type_: PossiblyEnforcedTy,
    pub kind: ParamMode,
    pub accept_disposable: bool,
    pub mutability: Option<ParamMutability>,
    pub rx_annotation: Option<ParamRxAnnotation>,
}

pub type FunParams = Vec<FunParam>;

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum XhpAttrTag {
    Required,
    Lateinit,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct XhpAttr {
    pub tag: Option<XhpAttrTag>,
    pub has_default: bool,
}

#[derive(Clone, Debug, OcamlRep)]
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
    pub type_: lazy::Lazy<Ty>,
    pub origin: String,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct ClassConst {
    pub synthesized: bool,
    pub abstract_: bool,
    pub pos: pos::Pos,
    pub type_: Ty,
    pub visibility: Visibility,
    pub expr: Option<nast::Expr>,
    pub origin: String,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct Requirement(pub pos::Pos, pub Ty);

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum ConsistentKind {
    Inconsistent,
    ConsistentConstruct,
    FinalClass,
}

#[derive(Clone, Debug, OcamlRep)]
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
    pub construct: (Option<ClassElt>, ConsistentKind),
    pub ancestors: s_map::SMap<Ty>,
    pub req_ancestors: Vec<Requirement>,
    pub req_ancestors_extends: s_set::SSet,
    pub extends: s_set::SSet,
    pub enum_type: Option<EnumType>,
    pub sealed_whitelist: Option<s_set::SSet>,
    pub decl_errors: Option<errors::Errors>,
}

#[derive(Clone, Debug, OcamlRep)]
pub enum TypeconstAbstractKind {
    TCAbstract(Option<Ty>),
    TCPartiallyAbstract,
    TCConcrete,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct TypeconstType {
    pub abstract_: TypeconstAbstractKind,
    pub visibility: Visibility,
    pub name: nast::Sid,
    pub constraint: Option<Ty>,
    pub type_: Option<Ty>,
    pub origin: String,
    pub enforceable: (pos::Pos, bool),
    pub reifiable: Option<pos::Pos>,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct PuEnumType {
    pub name: nast::Sid,
    pub is_final: bool,
    pub case_types: s_map::SMap<nast::Sid>,
    pub case_values: s_map::SMap<(nast::Sid, Ty)>,
    pub members: s_map::SMap<PuMemberType>,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct PuMemberType {
    pub atom: nast::Sid,
    pub types: s_map::SMap<(nast::Sid, Ty)>,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct EnumType {
    pub base: Ty,
    pub constraint: Option<Ty>,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct TypedefType {
    pub pos: pos::Pos,
    pub vis: aast::TypedefVisibility,
    pub tparams: Vec<Tparam>,
    pub constraint: Option<Ty>,
    pub type_: Ty,
    pub decl_errors: Option<errors::Errors>,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct Tparam {
    pub variance: ast_defs::Variance,
    pub name: ast_defs::Id,
    pub constraints: Vec<(ast_defs::ConstraintKind, Ty)>,
    pub reified: aast::ReifyKind,
    pub user_attributes: Vec<nast::UserAttribute>,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct WhereConstraint(pub Ty, pub ast_defs::ConstraintKind, pub Ty);

#[derive(Clone, Debug, OcamlRep)]
pub enum DeserializationError {
    WrongPhase(String),
    NotSupported(String),
    DeserializationError(String),
}
