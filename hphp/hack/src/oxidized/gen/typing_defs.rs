// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<69875ba3b3c2b6dde614801e578fc3b8>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;

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

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum Visibility {
    Vpublic,
    Vprivate(String),
    Vprotected(String),
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum Exact {
    Exact,
    Nonexact,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum ValKind {
    Lval,
    LvalSubexpr,
    Other,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum ParamMutability {
    ParamOwnedMutable,
    ParamBorrowedMutable,
    ParamMaybeMutable,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum FunTparamsKind {
    FTKtparams,
    FTKinstantiatedTargs,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum ShapeKind {
    ClosedShape,
    OpenShape,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum ParamMode {
    FPnormal,
    FPref,
    FPinout,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum PuKind {
    PuPlain,
    PuAtom(String),
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum XhpAttrTag {
    Required,
    Lateinit,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct XhpAttr {
    pub tag: Option<XhpAttrTag>,
    pub has_default: bool,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum ConsistentKind {
    Inconsistent,
    ConsistentConstruct,
    FinalClass,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Ty(pub reason::Reason, pub Box<Ty_>);

pub type DeclTy = Ty;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShapeFieldType {
    pub optional: bool,
    pub ty: Ty,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum Ty_ {
    Tthis,
    Tapply(nast::Sid, Vec<DeclTy>),
    Tgeneric(String),
    Taccess(TaccessType),
    Tarray(Option<DeclTy>, Option<DeclTy>),
    Tdarray(DeclTy, DeclTy),
    Tvarray(DeclTy),
    TvarrayOrDarray(DeclTy),
    Tmixed,
    Tnothing,
    Tlike(DeclTy),
    Tany(tany_sentinel::TanySentinel),
    Terr,
    Tnonnull,
    Tdynamic,
    Toption(Ty),
    Tprim(aast::Tprim),
    Tfun(FunType<Ty>),
    Ttuple(Vec<Ty>),
    Tshape(ShapeKind, nast::shape_map::ShapeMap<ShapeFieldType>),
    TpuAccess(Ty, nast::Sid),
    Tvar(ident::Ident),
    Tunion(Vec<Ty>),
    Tintersection(Vec<Ty>),
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum DependentType {
    DTthis,
    DTcls(String),
    DTexpr(ident::Ident),
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct TaccessType(pub DeclTy, pub Vec<nast::Sid>);

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum Reactivity {
    Nonreactive,
    Local(Option<DeclTy>),
    Shallow(Option<DeclTy>),
    Reactive(Option<DeclTy>),
    MaybeReactive(Box<Reactivity>),
    RxVar(Option<Box<Reactivity>>),
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
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

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum FunArity<Ty> {
    Fstandard(isize, isize),
    Fvariadic(isize, FunParam<Ty>),
    Fellipsis(isize, pos::Pos),
}

pub type DeclFunArity = FunArity<DeclTy>;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum ParamRxAnnotation {
    ParamRxVar,
    ParamRxIfImpl(DeclTy),
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct PossiblyEnforcedTy<Ty> {
    pub enforced: bool,
    pub type_: Ty,
}

pub type DeclPossiblyEnforcedTy = PossiblyEnforcedTy<DeclTy>;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
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

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
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

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct FunElt {
    pub deprecated: Option<String>,
    pub type_: DeclTy,
    pub decl_errors: Option<errors::Errors>,
    pub pos: pos::Pos,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ClassConst {
    pub synthesized: bool,
    pub abstract_: bool,
    pub pos: pos::Pos,
    pub type_: DeclTy,
    pub expr: Option<nast::Expr>,
    pub origin: String,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Requirement(pub pos::Pos, pub DeclTy);

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
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

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum TypeconstAbstractKind {
    TCAbstract(Option<DeclTy>),
    TCPartiallyAbstract,
    TCConcrete,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct TypeconstType {
    pub abstract_: TypeconstAbstractKind,
    pub name: nast::Sid,
    pub constraint: Option<DeclTy>,
    pub type_: Option<DeclTy>,
    pub origin: String,
    pub enforceable: (pos::Pos, bool),
    pub reifiable: Option<pos::Pos>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct PuEnumType {
    pub name: nast::Sid,
    pub is_final: bool,
    pub case_types: s_map::SMap<nast::Sid>,
    pub case_values: s_map::SMap<(nast::Sid, DeclTy)>,
    pub members: s_map::SMap<PuMemberType>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct PuMemberType {
    pub atom: nast::Sid,
    pub types: s_map::SMap<(nast::Sid, DeclTy)>,
    pub exprs: s_map::SMap<nast::Sid>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct EnumType {
    pub base: DeclTy,
    pub constraint: Option<DeclTy>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct RecordDefType {
    pub name: nast::Sid,
    pub pos: pos::Pos,
    pub errors: Option<errors::Errors>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct TypedefType {
    pub pos: pos::Pos,
    pub vis: aast::TypedefVisibility,
    pub tparams: Vec<DeclTparam>,
    pub constraint: Option<DeclTy>,
    pub type_: DeclTy,
    pub decl_errors: Option<errors::Errors>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Tparam<Ty> {
    pub variance: ast_defs::Variance,
    pub name: ast_defs::Id,
    pub constraints: Vec<(ast_defs::ConstraintKind, Ty)>,
    pub reified: aast::ReifyKind,
    pub user_attributes: Vec<nast::UserAttribute>,
}

pub type DeclTparam = Tparam<DeclTy>;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct WhereConstraint<Ty>(pub Ty, pub ast_defs::ConstraintKind, pub Ty);

pub type DeclWhereConstraint = WhereConstraint<DeclTy>;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum DeserializationError {
    WrongPhase(String),
    NotSupported(String),
    DeserializationError(String),
}
