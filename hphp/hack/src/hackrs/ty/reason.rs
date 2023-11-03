// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::Hash;
use std::sync::Arc;

use eq_modulo_pos::EqModuloPos;
use hcons::Conser;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use once_cell::sync::Lazy;
pub use oxidized::typing_reason::ArgPosition;
pub use oxidized::typing_reason::BlameSource;
use pos::BPos;
use pos::NPos;
use pos::Pos;
use pos::Positioned;
use pos::Symbol;
use pos::ToOxidized;
use pos::TypeConstName;
use pos::TypeName;
use serde::de::DeserializeOwned;
use serde::Deserialize;
use serde::Serialize;

use crate::decl;
use crate::local;
use crate::prop::Prop;
use crate::prop::PropF;
use crate::visitor::Walkable;

pub trait Reason:
    Eq
    + EqModuloPos
    + Hash
    + Clone
    + Walkable<Self>
    + std::fmt::Debug
    + Send
    + Sync
    + Serialize
    + DeserializeOwned
    + for<'a> From<oxidized_by_ref::typing_reason::T_<'a>>
    + for<'a> ToOxidized<'a, Output = oxidized_by_ref::typing_reason::T_<'a>>
    + ToOcamlRep
    + FromOcamlRep
    + 'static
{
    /// Position type.
    type Pos: Pos + Send + Sync + 'static;

    /// Make a new instance. If the implementing Reason is stateful,
    /// it will call cons() to obtain the ReasonImpl to construct the instance.
    fn mk(cons: impl FnOnce() -> ReasonImpl<Self, Self::Pos>) -> Self;

    fn none() -> Self;

    fn witness(pos: Self::Pos) -> Self {
        Self::mk(|| ReasonImpl::Rwitness(pos))
    }

    fn witness_from_decl(pos: Self::Pos) -> Self {
        Self::mk(|| ReasonImpl::RwitnessFromDecl(pos))
    }

    fn hint(pos: Self::Pos) -> Self {
        Self::mk(|| ReasonImpl::Rhint(pos))
    }

    fn instantiate(r1: Self, ty_name: TypeName, r2: Self) -> Self {
        Self::mk(|| ReasonImpl::Rinstantiate(r1, ty_name, r2))
    }

    fn class_class(pos: Self::Pos, ty_name: TypeName) -> Self {
        Self::mk(|| ReasonImpl::RclassClass(pos, ty_name))
    }

    fn no_return(pos: Self::Pos) -> Self {
        Self::mk(|| ReasonImpl::RnoReturn(pos))
    }

    fn implicit_upper_bound(pos: Self::Pos, sym: Symbol) -> Self {
        Self::mk(|| ReasonImpl::RimplicitUpperBound(pos, sym))
    }

    fn tyvar(pos: Self::Pos) -> Self {
        Self::mk(|| ReasonImpl::RtypeVariable(pos))
    }

    fn early_solve_failed(pos: Self::Pos) -> Self {
        Self::mk(|| ReasonImpl::RsolveFail(pos))
    }

    fn type_variable_generics(pos: Self::Pos, kind_name: Symbol, use_name: Symbol) -> Self {
        Self::mk(|| ReasonImpl::RtypeVariableGenerics(pos, kind_name, use_name))
    }

    fn pos(&self) -> &Self::Pos;

    fn decl_ty_conser() -> &'static Conser<decl::Ty_<Self>>;
    fn local_ty_conser() -> &'static Conser<local::Ty_<Self, local::Ty<Self>>>;
    fn prop_conser() -> &'static Conser<PropF<Self, Prop<Self>>>;

    fn from_oxidized(reason: oxidized_by_ref::typing_reason::T_<'_>) -> Self {
        Self::mk(|| {
            use oxidized_by_ref::typing_reason::Blame as OBlame;
            use oxidized_by_ref::typing_reason::T_ as OR;
            use ReasonImpl as RI;
            match reason {
                OR::Rnone => RI::Rnone,
                OR::RmissingClass(pos) => RI::RmissingClass(pos.into()),
                OR::Rwitness(pos) => RI::Rwitness(pos.into()),
                OR::RwitnessFromDecl(pos) => RI::RwitnessFromDecl(pos.into()),
                OR::Ridx(&(pos, r)) => RI::Ridx(pos.into(), r.into()),
                OR::RidxVector(pos) => RI::RidxVector(pos.into()),
                OR::RidxVectorFromDecl(pos) => RI::RidxVectorFromDecl(pos.into()),
                OR::Rforeach(pos) => RI::Rforeach(pos.into()),
                OR::Rasyncforeach(pos) => RI::Rasyncforeach(pos.into()),
                OR::Rarith(pos) => RI::Rarith(pos.into()),
                OR::RarithRet(pos) => RI::RarithRet(pos.into()),
                OR::RarithRetFloat(&(pos, r, arg_position)) => {
                    RI::RarithRetFloat(pos.into(), r.into(), arg_position)
                }
                OR::RarithRetNum(&(pos, r, arg_position)) => {
                    RI::RarithRetNum(pos.into(), r.into(), arg_position)
                }
                OR::RarithRetInt(pos) => RI::RarithRetInt(pos.into()),
                OR::RarithDynamic(pos) => RI::RarithDynamic(pos.into()),
                OR::RbitwiseDynamic(pos) => RI::RbitwiseDynamic(pos.into()),
                OR::RincdecDynamic(pos) => RI::RincdecDynamic(pos.into()),
                OR::Rcomp(pos) => RI::Rcomp(pos.into()),
                OR::RconcatRet(pos) => RI::RconcatRet(pos.into()),
                OR::RlogicRet(pos) => RI::RlogicRet(pos.into()),
                OR::Rbitwise(pos) => RI::Rbitwise(pos.into()),
                OR::RbitwiseRet(pos) => RI::RbitwiseRet(pos.into()),
                OR::RnoReturn(pos) => RI::RnoReturn(pos.into()),
                OR::RnoReturnAsync(pos) => RI::RnoReturnAsync(pos.into()),
                OR::RretFunKind(&(pos, fun_kind)) => RI::RretFunKind(pos.into(), fun_kind),
                OR::RretFunKindFromDecl(&(pos, fun_kind)) => {
                    RI::RretFunKindFromDecl(pos.into(), fun_kind)
                }
                OR::Rhint(pos) => RI::Rhint(pos.into()),
                OR::Rthrow(pos) => RI::Rthrow(pos.into()),
                OR::Rplaceholder(pos) => RI::Rplaceholder(pos.into()),
                OR::RretDiv(pos) => RI::RretDiv(pos.into()),
                OR::RyieldGen(pos) => RI::RyieldGen(pos.into()),
                OR::RyieldAsyncgen(pos) => RI::RyieldAsyncgen(pos.into()),
                OR::RyieldAsyncnull(pos) => RI::RyieldAsyncnull(pos.into()),
                OR::RyieldSend(pos) => RI::RyieldSend(pos.into()),
                OR::RlostInfo(&(sym, r, OBlame::Blame(&(pos, blame_source)))) => {
                    RI::RlostInfo(Symbol::new(sym), r.into(), Blame(pos.into(), blame_source))
                }
                OR::Rformat(&(pos, sym, r)) => RI::Rformat(pos.into(), Symbol::new(sym), r.into()),
                OR::RclassClass(&(pos, s)) => RI::RclassClass(pos.into(), TypeName(Symbol::new(s))),
                OR::RunknownClass(pos) => RI::RunknownClass(pos.into()),
                OR::RvarParam(pos) => RI::RvarParam(pos.into()),
                OR::RvarParamFromDecl(pos) => RI::RvarParamFromDecl(pos.into()),
                OR::RunpackParam(&(pos1, pos2, i)) => RI::RunpackParam(pos1.into(), pos2.into(), i),
                OR::RinoutParam(pos) => RI::RinoutParam(pos.into()),
                OR::Rinstantiate(&(r1, sym, r2)) => {
                    RI::Rinstantiate(r1.into(), TypeName(Symbol::new(sym)), r2.into())
                }
                OR::Rtypeconst(&(r1, pos_id, sym, r2)) => RI::Rtypeconst(
                    r1.into(),
                    pos_id.into(),
                    Symbol::new(sym.0.unwrap()),
                    r2.into(),
                ),
                OR::RtypeAccess(&(r, list)) => RI::RtypeAccess(
                    r.into(),
                    list.iter()
                        .map(|(&r, s)| (r.into(), Symbol::new(s.0.unwrap())))
                        .collect(),
                ),
                OR::RexprDepType(&(r, pos, edt_reason)) => {
                    RI::RexprDepType(r.into(), pos.into(), edt_reason.into())
                }
                OR::RnullsafeOp(pos) => RI::RnullsafeOp(pos.into()),
                OR::RtconstNoCstr(&pos_id) => RI::RtconstNoCstr(pos_id.into()),
                OR::Rpredicated(&(pos, s)) => RI::Rpredicated(pos.into(), Symbol::new(s)),
                OR::Ris(pos) => RI::Ris(pos.into()),
                OR::Ras(pos) => RI::Ras(pos.into()),
                OR::Requal(pos) => RI::Requal(pos.into()),
                OR::RvarrayOrDarrayKey(pos) => RI::RvarrayOrDarrayKey(pos.into()),
                OR::RvecOrDictKey(pos) => RI::RvecOrDictKey(pos.into()),
                OR::Rusing(pos) => RI::Rusing(pos.into()),
                OR::RdynamicProp(pos) => RI::RdynamicProp(pos.into()),
                OR::RdynamicCall(pos) => RI::RdynamicCall(pos.into()),
                OR::RdynamicConstruct(pos) => RI::RdynamicConstruct(pos.into()),
                OR::RidxDict(pos) => RI::RidxDict(pos.into()),
                OR::RsetElement(pos) => RI::RsetElement(pos.into()),
                OR::RmissingOptionalField(&(pos, s)) => {
                    RI::RmissingOptionalField(pos.into(), Symbol::new(s))
                }
                OR::RunsetField(&(pos, s)) => RI::RunsetField(pos.into(), Symbol::new(s)),
                OR::RcontravariantGeneric(&(r, s)) => {
                    RI::RcontravariantGeneric(r.into(), Symbol::new(s))
                }
                OR::RinvariantGeneric(&(r, s)) => RI::RinvariantGeneric(r.into(), Symbol::new(s)),
                OR::Rregex(pos) => RI::Rregex(pos.into()),
                OR::RimplicitUpperBound(&(pos, s)) => {
                    RI::RimplicitUpperBound(pos.into(), Symbol::new(s))
                }
                OR::RtypeVariable(pos) => RI::RtypeVariable(pos.into()),
                OR::RtypeVariableGenerics(&(pos, s1, s2)) => {
                    RI::RtypeVariableGenerics(pos.into(), Symbol::new(s1), Symbol::new(s2))
                }
                OR::RtypeVariableError(pos) => RI::RtypeVariableError(pos.into()),
                OR::RglobalTypeVariableGenerics(&(pos, s1, s2)) => {
                    RI::RglobalTypeVariableGenerics(pos.into(), Symbol::new(s1), Symbol::new(s2))
                }
                OR::RsolveFail(pos) => RI::RsolveFail(pos.into()),
                OR::RcstrOnGenerics(&(pos, pos_id)) => {
                    RI::RcstrOnGenerics(pos.into(), pos_id.into())
                }
                OR::RlambdaParam(&(pos, r)) => RI::RlambdaParam(pos.into(), r.into()),
                OR::Rshape(&(pos, s)) => RI::Rshape(pos.into(), Symbol::new(s)),
                OR::RshapeLiteral(pos) => RI::RshapeLiteral(pos.into()),
                OR::Renforceable(pos) => RI::Renforceable(pos.into()),
                OR::Rdestructure(pos) => RI::Rdestructure(pos.into()),
                OR::RkeyValueCollectionKey(pos) => RI::RkeyValueCollectionKey(pos.into()),
                OR::RglobalClassProp(pos) => RI::RglobalClassProp(pos.into()),
                OR::RglobalFunParam(pos) => RI::RglobalFunParam(pos.into()),
                OR::RglobalFunRet(pos) => RI::RglobalFunRet(pos.into()),
                OR::Rsplice(pos) => RI::Rsplice(pos.into()),
                OR::RetBoolean(pos) => RI::RetBoolean(pos.into()),
                OR::RdefaultCapability(pos) => RI::RdefaultCapability(pos.into()),
                OR::RconcatOperand(pos) => RI::RconcatOperand(pos.into()),
                OR::RinterpOperand(pos) => RI::RinterpOperand(pos.into()),
                OR::RdynamicCoercion(&r) => RI::RdynamicCoercion(r.into()),
                OR::RsupportDynamicType(pos) => RI::RsupportDynamicType(pos.into()),
                OR::RdynamicPartialEnforcement(&(pos, s, r)) => {
                    RI::RdynamicPartialEnforcement(pos.into(), Symbol::new(s), r.into())
                }
                OR::RrigidTvarEscape(&(pos, s1, s2, r)) => {
                    RI::RrigidTvarEscape(pos.into(), Symbol::new(s1), Symbol::new(s2), r.into())
                }
                OR::RopaqueTypeFromModule(&(pos, s, r)) => {
                    RI::RopaqueTypeFromModule(pos.into(), Symbol::new(s), r.into())
                }
                OR::RunsafeCast(pos) => RI::RunsafeCast(pos.into()),
                OR::Rinvalid => RI::Rinvalid,
                OR::RcapturedLike(pos) => RI::RcapturedLike(pos.into()),
                OR::RpessimisedInout(pos) => RI::RpessimisedInout(pos.into()),
                OR::RpessimisedReturn(pos) => RI::RpessimisedReturn(pos.into()),
                OR::RpessimisedProp(pos) => RI::RpessimisedProp(pos.into()),
            }
        })
    }
}

#[derive(Debug, Clone, PartialEq, Eq, EqModuloPos, Hash, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub struct Blame<P>(pub P, pub BlameSource);

#[derive(Debug, Clone, PartialEq, Eq, EqModuloPos, Hash, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub enum ExprDepTypeReason {
    ERexpr(isize),
    ERstatic,
    ERclass(Symbol),
    ERparent(Symbol),
    ERself(Symbol),
    ERpu(Symbol),
}

impl<'a> From<oxidized_by_ref::typing_reason::ExprDepTypeReason<'a>> for ExprDepTypeReason {
    fn from(edtr: oxidized_by_ref::typing_reason::ExprDepTypeReason<'a>) -> Self {
        use oxidized_by_ref::typing_reason::ExprDepTypeReason as Obr;
        match edtr {
            Obr::ERexpr(i) => ExprDepTypeReason::ERexpr(i),
            Obr::ERstatic => ExprDepTypeReason::ERstatic,
            Obr::ERclass(s) => ExprDepTypeReason::ERclass(Symbol::new(s)),
            Obr::ERparent(s) => ExprDepTypeReason::ERparent(Symbol::new(s)),
            Obr::ERself(s) => ExprDepTypeReason::ERself(Symbol::new(s)),
            Obr::ERpu(s) => ExprDepTypeReason::ERpu(Symbol::new(s)),
        }
    }
}

impl<'a> ToOxidized<'a> for ExprDepTypeReason {
    type Output = oxidized_by_ref::typing_reason::ExprDepTypeReason<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use oxidized_by_ref::typing_reason::ExprDepTypeReason as Obr;
        match self {
            ExprDepTypeReason::ERexpr(i) => Obr::ERexpr(*i),
            ExprDepTypeReason::ERstatic => Obr::ERstatic,
            ExprDepTypeReason::ERclass(s) => Obr::ERclass(s.to_oxidized(arena)),
            ExprDepTypeReason::ERparent(s) => Obr::ERparent(s.to_oxidized(arena)),
            ExprDepTypeReason::ERself(s) => Obr::ERself(s.to_oxidized(arena)),
            ExprDepTypeReason::ERpu(s) => Obr::ERpu(s.to_oxidized(arena)),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, EqModuloPos, Hash, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub enum ReasonImpl<R, P> {
    Rnone,
    Rwitness(P),
    RwitnessFromDecl(P),
    /// Used as an index into a vector-like array or string.
    /// Position of indexing, reason for the indexed type
    Ridx(P, R),
    RidxVector(P),
    /// Used as an index, in the Vector case
    RidxVectorFromDecl(P),
    /// Because it is iterated in a foreach loop
    Rforeach(P),
    /// Because it is iterated "await as" in foreach
    Rasyncforeach(P),
    Rarith(P),
    RarithRet(P),
    /// pos, arg float typing reason, arg position
    RarithRetFloat(P, R, oxidized::typing_reason::ArgPosition),
    /// pos, arg num typing reason, arg position
    RarithRetNum(P, R, oxidized::typing_reason::ArgPosition),
    RarithRetInt(P),
    RarithDynamic(P),
    RbitwiseDynamic(P),
    RincdecDynamic(P),
    Rcomp(P),
    RconcatRet(P),
    RlogicRet(P),
    Rbitwise(P),
    RbitwiseRet(P),
    RnoReturn(P),
    RnoReturnAsync(P),
    RretFunKind(P, oxidized::ast_defs::FunKind),
    RretFunKindFromDecl(P, oxidized::ast_defs::FunKind),
    Rhint(P),
    Rthrow(P),
    Rplaceholder(P),
    RretDiv(P),
    RyieldGen(P),
    RyieldAsyncgen(P),
    RyieldAsyncnull(P),
    RyieldSend(P),
    RlostInfo(Symbol, R, Blame<P>),
    Rformat(P, Symbol, R),
    RclassClass(P, TypeName),
    RunknownClass(P),
    RvarParam(P),
    RvarParamFromDecl(P),
    /// splat pos, fun def pos, number of args before splat
    RunpackParam(P, P, isize),
    RinoutParam(P),
    Rinstantiate(R, TypeName, R),
    Rtypeconst(R, Positioned<TypeConstName, P>, Symbol, R),
    RtypeAccess(R, Vec<(R, Symbol)>),
    RexprDepType(R, P, ExprDepTypeReason),
    /// ?-> operator is used
    RnullsafeOp(P),
    RtconstNoCstr(Positioned<TypeConstName, P>),
    Rpredicated(P, Symbol),
    Ris(P),
    Ras(P),
    Requal(P),
    RvarrayOrDarrayKey(P),
    RvecOrDictKey(P),
    Rusing(P),
    RdynamicProp(P),
    RdynamicCall(P),
    RdynamicConstruct(P),
    RidxDict(P),
    RsetElement(P),
    RmissingOptionalField(P, Symbol),
    RunsetField(P, Symbol),
    RcontravariantGeneric(R, Symbol),
    RinvariantGeneric(R, Symbol),
    Rregex(P),
    RimplicitUpperBound(P, Symbol),
    RtypeVariable(P),
    RtypeVariableGenerics(P, Symbol, Symbol),
    RtypeVariableError(P),
    RglobalTypeVariableGenerics(P, Symbol, Symbol),
    RsolveFail(P),
    RcstrOnGenerics(P, Positioned<TypeName, P>),
    RlambdaParam(P, R),
    Rshape(P, Symbol),
    RshapeLiteral(P),
    Renforceable(P),
    Rdestructure(P),
    RkeyValueCollectionKey(P),
    RglobalClassProp(P),
    RglobalFunParam(P),
    RglobalFunRet(P),
    Rsplice(P),
    RetBoolean(P),
    RdefaultCapability(P),
    RconcatOperand(P),
    RinterpOperand(P),
    RdynamicCoercion(R),
    RsupportDynamicType(P),
    RdynamicPartialEnforcement(P, Symbol, R),
    RrigidTvarEscape(P, Symbol, Symbol, R),
    RopaqueTypeFromModule(P, Symbol, R),
    RmissingClass(P),
    RunsafeCast(P),
    Rinvalid,
    RcapturedLike(P),
    RpessimisedInout(P),
    RpessimisedReturn(P),
    RpessimisedProp(P),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub struct BReason(Arc<ReasonImpl<BReason, BPos>>);

impl Reason for BReason {
    type Pos = BPos;

    fn mk(cons: impl FnOnce() -> ReasonImpl<Self, Self::Pos>) -> Self {
        let x = cons();
        Self(Arc::new(x))
    }

    fn none() -> Self {
        BReason(Arc::new(ReasonImpl::Rnone))
    }

    fn pos(&self) -> &BPos {
        use ReasonImpl::*;
        match &*self.0 {
            Rnone => unimplemented!(),
            Rwitness(p) | RwitnessFromDecl(p) | Rhint(p) => p,
            r => unimplemented!("BReason::pos: {:?}", r),
        }
    }

    #[inline]
    fn decl_ty_conser() -> &'static Conser<decl::Ty_<BReason>> {
        static CONSER: Lazy<Conser<decl::Ty_<BReason>>> = Lazy::new(Conser::new);
        &CONSER
    }

    #[inline]
    fn local_ty_conser() -> &'static Conser<local::Ty_<BReason, local::Ty<BReason>>> {
        static CONSER: Lazy<Conser<local::Ty_<BReason, local::Ty<BReason>>>> =
            Lazy::new(Conser::new);
        &CONSER
    }

    #[inline]
    fn prop_conser() -> &'static Conser<PropF<BReason, Prop<BReason>>> {
        static CONSER: Lazy<Conser<PropF<BReason, Prop<BReason>>>> = Lazy::new(Conser::new);
        &CONSER
    }
}

impl Walkable<BReason> for BReason {}

impl<'a> From<oxidized_by_ref::typing_reason::Reason<'a>> for BReason {
    fn from(reason: oxidized_by_ref::typing_reason::Reason<'a>) -> Self {
        Self::from_oxidized(reason)
    }
}

impl<'a> ToOxidized<'a> for BReason {
    type Output = oxidized_by_ref::typing_reason::Reason<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use oxidized_by_ref::typing_reason::Blame as OBlame;
        use oxidized_by_ref::typing_reason::Reason as OR;
        use ReasonImpl as RI;
        match &*self.0 {
            RI::Rnone => OR::Rnone,
            RI::Rwitness(pos) => OR::Rwitness(pos.to_oxidized(arena)),
            RI::RwitnessFromDecl(pos) => OR::RwitnessFromDecl(pos.to_oxidized(arena)),
            RI::Ridx(pos, r) => {
                OR::Ridx(arena.alloc((pos.to_oxidized(arena), r.to_oxidized(arena))))
            }
            RI::RidxVector(pos) => OR::RidxVector(pos.to_oxidized(arena)),
            RI::RidxVectorFromDecl(pos) => OR::RidxVectorFromDecl(pos.to_oxidized(arena)),
            RI::Rforeach(pos) => OR::Rforeach(pos.to_oxidized(arena)),
            RI::Rasyncforeach(pos) => OR::Rasyncforeach(pos.to_oxidized(arena)),
            RI::Rarith(pos) => OR::Rarith(pos.to_oxidized(arena)),
            RI::RarithRet(pos) => OR::RarithRet(pos.to_oxidized(arena)),
            RI::RarithRetFloat(pos, r, arg_position) => OR::RarithRetFloat(arena.alloc((
                pos.to_oxidized(arena),
                r.to_oxidized(arena),
                *arg_position,
            ))),
            RI::RarithRetNum(pos, r, arg_position) => OR::RarithRetNum(arena.alloc((
                pos.to_oxidized(arena),
                r.to_oxidized(arena),
                *arg_position,
            ))),
            RI::RarithRetInt(pos) => OR::RarithRetInt(pos.to_oxidized(arena)),
            RI::RarithDynamic(pos) => OR::RarithDynamic(pos.to_oxidized(arena)),
            RI::RbitwiseDynamic(pos) => OR::RbitwiseDynamic(pos.to_oxidized(arena)),
            RI::RincdecDynamic(pos) => OR::RincdecDynamic(pos.to_oxidized(arena)),
            RI::Rcomp(pos) => OR::Rcomp(pos.to_oxidized(arena)),
            RI::RconcatRet(pos) => OR::RconcatRet(pos.to_oxidized(arena)),
            RI::RlogicRet(pos) => OR::RlogicRet(pos.to_oxidized(arena)),
            RI::Rbitwise(pos) => OR::Rbitwise(pos.to_oxidized(arena)),
            RI::RbitwiseRet(pos) => OR::RbitwiseRet(pos.to_oxidized(arena)),
            RI::RnoReturn(pos) => OR::RnoReturn(pos.to_oxidized(arena)),
            RI::RnoReturnAsync(pos) => OR::RnoReturnAsync(pos.to_oxidized(arena)),
            RI::RretFunKind(pos, fun_kind) => {
                OR::RretFunKind(arena.alloc((pos.to_oxidized(arena), *fun_kind)))
            }
            RI::RretFunKindFromDecl(pos, fun_kind) => {
                OR::RretFunKindFromDecl(arena.alloc((pos.to_oxidized(arena), *fun_kind)))
            }
            RI::Rhint(pos) => OR::Rhint(pos.to_oxidized(arena)),
            RI::Rthrow(pos) => OR::Rthrow(pos.to_oxidized(arena)),
            RI::Rplaceholder(pos) => OR::Rplaceholder(pos.to_oxidized(arena)),
            RI::RretDiv(pos) => OR::RretDiv(pos.to_oxidized(arena)),
            RI::RyieldGen(pos) => OR::RyieldGen(pos.to_oxidized(arena)),
            RI::RyieldAsyncgen(pos) => OR::RyieldAsyncgen(pos.to_oxidized(arena)),
            RI::RyieldAsyncnull(pos) => OR::RyieldAsyncnull(pos.to_oxidized(arena)),
            RI::RyieldSend(pos) => OR::RyieldSend(pos.to_oxidized(arena)),
            RI::RlostInfo(sym, r, Blame(pos, blame_source)) => OR::RlostInfo(arena.alloc((
                sym.to_oxidized(arena),
                r.to_oxidized(arena),
                OBlame::Blame(arena.alloc((pos.to_oxidized(arena), *blame_source))),
            ))),
            RI::Rformat(pos, sym, r) => OR::Rformat(arena.alloc((
                pos.to_oxidized(arena),
                sym.to_oxidized(arena),
                r.to_oxidized(arena),
            ))),
            RI::RclassClass(pos, s) => {
                OR::RclassClass(arena.alloc((pos.to_oxidized(arena), s.to_oxidized(arena))))
            }
            RI::RunknownClass(pos) => OR::RunknownClass(pos.to_oxidized(arena)),
            RI::RvarParam(pos) => OR::RvarParam(pos.to_oxidized(arena)),
            RI::RvarParamFromDecl(pos) => OR::RvarParamFromDecl(pos.to_oxidized(arena)),
            RI::RunpackParam(pos1, pos2, i) => OR::RunpackParam(arena.alloc((
                pos1.to_oxidized(arena),
                pos2.to_oxidized(arena),
                *i,
            ))),
            RI::RinoutParam(pos) => OR::RinoutParam(pos.to_oxidized(arena)),
            RI::Rinstantiate(r1, type_name, r2) => OR::Rinstantiate(arena.alloc((
                r1.to_oxidized(arena),
                type_name.to_oxidized(arena),
                r2.to_oxidized(arena),
            ))),
            RI::Rtypeconst(r1, pos_id, sym, r2) => OR::Rtypeconst(arena.alloc((
                r1.to_oxidized(arena),
                pos_id.to_oxidized(arena),
                &*arena.alloc(oxidized_by_ref::lazy::Lazy(Some(sym.to_oxidized(arena)))),
                r2.to_oxidized(arena),
            ))),
            RI::RtypeAccess(r, list) => OR::RtypeAccess(arena.alloc((
                r.to_oxidized(arena),
                &*arena.alloc_slice_fill_iter(list.iter().map(|(r, s)| {
                    (
                        &*arena.alloc(r.to_oxidized(arena)),
                        &*arena.alloc(oxidized_by_ref::lazy::Lazy(Some(s.to_oxidized(arena)))),
                    )
                })),
            ))),
            RI::RexprDepType(r, pos, edt_reason) => OR::RexprDepType(arena.alloc((
                r.to_oxidized(arena),
                pos.to_oxidized(arena),
                edt_reason.to_oxidized(arena),
            ))),
            RI::RnullsafeOp(pos) => OR::RnullsafeOp(pos.to_oxidized(arena)),
            RI::RtconstNoCstr(pos_id) => OR::RtconstNoCstr(arena.alloc(pos_id.to_oxidized(arena))),
            RI::Rpredicated(pos, s) => {
                OR::Rpredicated(arena.alloc((pos.to_oxidized(arena), s.to_oxidized(arena))))
            }
            RI::Ris(pos) => OR::Ris(pos.to_oxidized(arena)),
            RI::Ras(pos) => OR::Ras(pos.to_oxidized(arena)),
            RI::Requal(pos) => OR::Requal(pos.to_oxidized(arena)),
            RI::RvarrayOrDarrayKey(pos) => OR::RvarrayOrDarrayKey(pos.to_oxidized(arena)),
            RI::RvecOrDictKey(pos) => OR::RvecOrDictKey(pos.to_oxidized(arena)),
            RI::Rusing(pos) => OR::Rusing(pos.to_oxidized(arena)),
            RI::RdynamicProp(pos) => OR::RdynamicProp(pos.to_oxidized(arena)),
            RI::RdynamicCall(pos) => OR::RdynamicCall(pos.to_oxidized(arena)),
            RI::RdynamicConstruct(pos) => OR::RdynamicConstruct(pos.to_oxidized(arena)),
            RI::RidxDict(pos) => OR::RidxDict(pos.to_oxidized(arena)),
            RI::RsetElement(pos) => OR::RsetElement(pos.to_oxidized(arena)),
            RI::RmissingOptionalField(pos, s) => OR::RmissingOptionalField(
                arena.alloc((pos.to_oxidized(arena), s.to_oxidized(arena))),
            ),
            RI::RunsetField(pos, s) => {
                OR::RunsetField(arena.alloc((pos.to_oxidized(arena), s.to_oxidized(arena))))
            }
            RI::RcontravariantGeneric(r, s) => {
                OR::RcontravariantGeneric(arena.alloc((r.to_oxidized(arena), s.to_oxidized(arena))))
            }
            RI::RinvariantGeneric(r, s) => {
                OR::RinvariantGeneric(arena.alloc((r.to_oxidized(arena), s.to_oxidized(arena))))
            }
            RI::Rregex(pos) => OR::Rregex(pos.to_oxidized(arena)),
            RI::RimplicitUpperBound(pos, s) => {
                OR::RimplicitUpperBound(arena.alloc((pos.to_oxidized(arena), s.to_oxidized(arena))))
            }
            RI::RtypeVariable(pos) => OR::RtypeVariable(pos.to_oxidized(arena)),
            RI::RtypeVariableGenerics(pos, s1, s2) => OR::RtypeVariableGenerics(arena.alloc((
                pos.to_oxidized(arena),
                s1.to_oxidized(arena),
                s2.to_oxidized(arena),
            ))),
            RI::RglobalTypeVariableGenerics(pos, s1, s2) => {
                OR::RglobalTypeVariableGenerics(arena.alloc((
                    pos.to_oxidized(arena),
                    s1.to_oxidized(arena),
                    s2.to_oxidized(arena),
                )))
            }
            RI::RtypeVariableError(pos) => OR::RtypeVariableError(pos.to_oxidized(arena)),
            RI::RsolveFail(pos) => OR::RsolveFail(pos.to_oxidized(arena)),
            RI::RcstrOnGenerics(pos, pos_id) => OR::RcstrOnGenerics(
                arena.alloc((pos.to_oxidized(arena), pos_id.to_oxidized(arena))),
            ),
            RI::RlambdaParam(pos, r) => {
                OR::RlambdaParam(arena.alloc((pos.to_oxidized(arena), r.to_oxidized(arena))))
            }
            RI::Rshape(pos, s) => {
                OR::Rshape(arena.alloc((pos.to_oxidized(arena), s.to_oxidized(arena))))
            }
            RI::RshapeLiteral(pos) => OR::RshapeLiteral(pos.to_oxidized(arena)),
            RI::Renforceable(pos) => OR::Renforceable(pos.to_oxidized(arena)),
            RI::Rdestructure(pos) => OR::Rdestructure(pos.to_oxidized(arena)),
            RI::RkeyValueCollectionKey(pos) => OR::RkeyValueCollectionKey(pos.to_oxidized(arena)),
            RI::RglobalClassProp(pos) => OR::RglobalClassProp(pos.to_oxidized(arena)),
            RI::RglobalFunParam(pos) => OR::RglobalFunParam(pos.to_oxidized(arena)),
            RI::RglobalFunRet(pos) => OR::RglobalFunRet(pos.to_oxidized(arena)),
            RI::Rsplice(pos) => OR::Rsplice(pos.to_oxidized(arena)),
            RI::RetBoolean(pos) => OR::RetBoolean(pos.to_oxidized(arena)),
            RI::RdefaultCapability(pos) => OR::RdefaultCapability(pos.to_oxidized(arena)),
            RI::RconcatOperand(pos) => OR::RconcatOperand(pos.to_oxidized(arena)),
            RI::RinterpOperand(pos) => OR::RinterpOperand(pos.to_oxidized(arena)),
            RI::RdynamicCoercion(r) => OR::RdynamicCoercion(arena.alloc(r.to_oxidized(arena))),
            RI::RsupportDynamicType(pos) => OR::RsupportDynamicType(pos.to_oxidized(arena)),
            RI::RdynamicPartialEnforcement(pos, s, r) => {
                OR::RdynamicPartialEnforcement(arena.alloc((
                    pos.to_oxidized(arena),
                    s.to_oxidized(arena),
                    r.to_oxidized(arena),
                )))
            }
            RI::RrigidTvarEscape(pos, s1, s2, r) => OR::RrigidTvarEscape(arena.alloc((
                pos.to_oxidized(arena),
                s1.to_oxidized(arena),
                s2.to_oxidized(arena),
                r.to_oxidized(arena),
            ))),
            RI::RopaqueTypeFromModule(pos, s1, r) => OR::RopaqueTypeFromModule(arena.alloc((
                pos.to_oxidized(arena),
                s1.to_oxidized(arena),
                r.to_oxidized(arena),
            ))),
            RI::RmissingClass(pos) => OR::RmissingClass(pos.to_oxidized(arena)),
            RI::RunsafeCast(pos) => OR::RunsafeCast(pos.to_oxidized(arena)),
            RI::Rinvalid => OR::Rinvalid,
            RI::RcapturedLike(pos) => OR::RcapturedLike(pos.to_oxidized(arena)),
            RI::RpessimisedInout(pos) => OR::RpessimisedInout(pos.to_oxidized(arena)),
            RI::RpessimisedReturn(pos) => OR::RpessimisedReturn(pos.to_oxidized(arena)),
            RI::RpessimisedProp(pos) => OR::RpessimisedProp(pos.to_oxidized(arena)),
        }
    }
}

impl EqModuloPos for BReason {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        self.0.eq_modulo_pos(&rhs.0)
    }
    fn eq_modulo_pos_and_reason(&self, _rhs: &Self) -> bool {
        true
    }
}

/// A stateless sentinal Reason.
#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct NReason;

impl Reason for NReason {
    type Pos = NPos;

    fn mk(_cons: impl FnOnce() -> ReasonImpl<Self, Self::Pos>) -> Self {
        NReason
    }

    fn none() -> Self {
        NReason
    }

    fn pos(&self) -> &NPos {
        &NPos
    }

    #[inline]
    fn decl_ty_conser() -> &'static Conser<decl::Ty_<NReason>> {
        static CONSER: Lazy<Conser<decl::Ty_<NReason>>> = Lazy::new(Conser::new);
        &CONSER
    }

    #[inline]
    fn local_ty_conser() -> &'static Conser<local::Ty_<NReason, local::Ty<NReason>>> {
        static CONSER: Lazy<Conser<local::Ty_<NReason, local::Ty<NReason>>>> =
            Lazy::new(Conser::new);
        &CONSER
    }

    #[inline]
    fn prop_conser() -> &'static Conser<PropF<NReason, Prop<NReason>>> {
        static CONSER: Lazy<Conser<PropF<NReason, Prop<NReason>>>> = Lazy::new(Conser::new);
        &CONSER
    }
}

impl Walkable<NReason> for NReason {}

impl<'a> From<oxidized_by_ref::typing_reason::T_<'a>> for NReason {
    fn from(reason: oxidized_by_ref::typing_reason::T_<'a>) -> Self {
        Self::from_oxidized(reason)
    }
}

impl<'a> ToOxidized<'a> for NReason {
    type Output = oxidized_by_ref::typing_reason::Reason<'a>;

    fn to_oxidized(&self, _arena: &'a bumpalo::Bump) -> Self::Output {
        oxidized_by_ref::typing_reason::Reason::Rnone
    }
}

impl ToOcamlRep for NReason {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        oxidized_by_ref::typing_reason::Reason::Rnone.to_ocamlrep(alloc)
    }
}

impl FromOcamlRep for NReason {
    fn from_ocamlrep(_value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        Ok(Self)
    }
}

impl EqModuloPos for NReason {
    fn eq_modulo_pos(&self, _rhs: &Self) -> bool {
        true
    }
    fn eq_modulo_pos_and_reason(&self, _rhs: &Self) -> bool {
        true
    }
}
