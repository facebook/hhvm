// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<45037bfe169659208cc756f6ed7f9dcb>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::aast;
use crate::ast_defs;
use crate::ident;
use crate::nast;
use crate::pos;
use crate::tany_sentinel;

pub use crate::typing_reason as reason;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum Visibility {
    Vpublic,
    Vprivate(String),
    Vprotected(String),
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum Exact {
    Exact,
    Nonexact,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum ValKind {
    Lval,
    LvalSubexpr,
    Other,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum ParamMutability {
    ParamOwnedMutable,
    ParamBorrowedMutable,
    ParamMaybeMutable,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum FunTparamsKind {
    /// If ft_tparams is empty, the containing fun_type is a concrete function type.
    /// Otherwise, it is a generic function and ft_tparams specifies its type parameters.
    FTKtparams,
    /// The containing fun_type is a concrete function type which is an
    /// instantiation of a generic function with at least one reified type
    /// parameter. This means that the function requires explicit type arguments
    /// at every invocation, and ft_tparams specifies the type arguments with
    /// which the generic function was instantiated, as well as whether each
    /// explicit type argument must be reified.
    FTKinstantiatedTargs,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum ShapeKind {
    ClosedShape,
    OpenShape,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum ParamMode {
    FPnormal,
    FPinout,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum XhpAttrTag {
    Required,
    Lateinit,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct XhpAttr {
    pub tag: Option<XhpAttrTag>,
    pub has_default: bool,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum ConsistentKind {
    Inconsistent,
    ConsistentConstruct,
    FinalClass,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum DependentType {
    DTthis,
    DTcls(String),
    DTexpr(ident::Ident),
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum DestructureKind {
    ListDestructure,
    SplatUnpack,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct Tparam<Ty> {
    pub variance: ast_defs::Variance,
    pub name: ast_defs::Id,
    pub constraints: Vec<(ast_defs::ConstraintKind, Ty)>,
    pub reified: aast::ReifyKind,
    pub user_attributes: Vec<nast::UserAttribute>,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct WhereConstraint<Ty>(pub Ty, pub ast_defs::ConstraintKind, pub Ty);

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct Ty(pub reason::Reason, pub Box<Ty_>);

pub type DeclTy = Ty;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct ShapeFieldType {
    pub optional: bool,
    pub ty: Ty,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum Ty_ {
    Tthis,
    Tapply(nast::Sid, Vec<DeclTy>),
    Taccess(TaccessType),
    Tarray(Option<DeclTy>, Option<DeclTy>),
    Tdarray(DeclTy, DeclTy),
    Tvarray(DeclTy),
    TvarrayOrDarray(Option<DeclTy>, DeclTy),
    Tmixed,
    Tnothing,
    Tlike(DeclTy),
    TpuAccess(DeclTy, nast::Sid),
    Tany(tany_sentinel::TanySentinel),
    Terr,
    Tnonnull,
    Tdynamic,
    Toption(Ty),
    Tprim(aast::Tprim),
    Tfun(FunType<Ty>),
    Ttuple(Vec<Ty>),
    Tshape(ShapeKind, nast::shape_map::ShapeMap<ShapeFieldType>),
    Tvar(ident::Ident),
    Tgeneric(String),
    Tunion(Vec<Ty>),
    Tintersection(Vec<Ty>),
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct TaccessType(pub DeclTy, pub Vec<nast::Sid>);

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum Reactivity {
    Nonreactive,
    Local(Option<DeclTy>),
    Shallow(Option<DeclTy>),
    Reactive(Option<DeclTy>),
    MaybeReactive(Box<Reactivity>),
    RxVar(Option<Box<Reactivity>>),
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct FunType<Ty> {
    pub is_coroutine: bool,
    pub arity: FunArity<Ty>,
    pub tparams: (Vec<Tparam<Ty>>, FunTparamsKind),
    pub where_constraints: Vec<WhereConstraint<Ty>>,
    pub params: FunParams<Ty>,
    pub ret: PossiblyEnforcedTy<Ty>,
    pub fun_kind: ast_defs::FunKind,
    pub reactive: Reactivity,
    pub return_disposable: bool,
    pub mutability: Option<ParamMutability>,
    pub returns_mutable: bool,
    pub returns_void_to_rx: bool,
}

pub type DeclFunType = FunType<DeclTy>;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum FunArity<Ty> {
    Fstandard(isize, isize),
    Fvariadic(isize, FunParam<Ty>),
    Fellipsis(isize, pos::Pos),
}

pub type DeclFunArity = FunArity<DeclTy>;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum ParamRxAnnotation {
    ParamRxVar,
    ParamRxIfImpl(DeclTy),
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct PossiblyEnforcedTy<Ty> {
    pub enforced: bool,
    pub type_: Ty,
}

pub type DeclPossiblyEnforcedTy = PossiblyEnforcedTy<DeclTy>;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct FunParam<Ty> {
    pub pos: pos::Pos,
    pub name: Option<String>,
    pub type_: PossiblyEnforcedTy<Ty>,
    pub kind: ParamMode,
    pub accept_disposable: bool,
    pub mutability: Option<ParamMutability>,
    pub rx_annotation: Option<ParamRxAnnotation>,
}

pub type DeclFunParam = FunParam<DeclTy>;

pub type FunParams<Ty> = Vec<FunParam<Ty>>;

pub type DeclFunParams = FunParams<DeclTy>;
