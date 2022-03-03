// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hcons::{Conser, Hc};
use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};
use once_cell::sync::OnceCell;
use pos::{BPos, NPos, Pos, Positioned, Symbol, TypeConstName, TypeName};
use std::hash::Hash;

use crate::decl_defs::DeclTy_;
use crate::typing_defs::{Ty, Ty_};
use crate::visitor::Walkable;

pub use oxidized::typing_reason::{ArgPosition, BlameSource};

pub trait Reason:
    Eq
    + Hash
    + Clone
    + ToOcamlRep
    + Walkable<Self>
    + std::fmt::Debug
    + Send
    + Sync
    + for<'a> From<oxidized_by_ref::typing_reason::T_<'a>>
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

    fn pos(&self) -> &Self::Pos;

    fn to_oxidized<'a>(
        &self,
        arena: &'a bumpalo::Bump,
    ) -> &'a oxidized_by_ref::typing_reason::Reason<'a>;

    fn cons_decl_ty(ty: DeclTy_<Self>) -> Hc<DeclTy_<Self>>;

    fn cons_ty(ty: Ty_<Self, Ty<Self>>) -> Hc<Ty_<Self, Ty<Self>>>;

    fn from_oxidized(reason: oxidized_by_ref::typing_reason::T_<'_>) -> Self {
        Self::mk(|| {
            use crate::reason::ReasonImpl as RI;
            use oxidized_by_ref::typing_reason::Blame as OBlame;
            use oxidized_by_ref::typing_reason::T_ as OR;
            match reason {
                OR::Rnone => RI::Rnone,
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
                OR::Rtypeconst(&(r1, pos_id, sym, r2)) => {
                    RI::Rtypeconst(r1.into(), pos_id.into(), Symbol::new(sym.0), r2.into())
                }
                OR::RtypeAccess(&(r, list)) => RI::RtypeAccess(
                    r.into(),
                    list.iter()
                        .map(|(&r, s)| (r.into(), Symbol::new(s.0)))
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
                OR::RglobalTypeVariableGenerics(&(pos, s1, s2)) => {
                    RI::RglobalTypeVariableGenerics(pos.into(), Symbol::new(s1), Symbol::new(s2))
                }
                OR::RsolveFail(pos) => RI::RsolveFail(pos.into()),
                OR::RcstrOnGenerics(&(pos, pos_id)) => {
                    RI::RcstrOnGenerics(pos.into(), pos_id.into())
                }
                OR::RlambdaParam(&(pos, r)) => RI::RlambdaParam(pos.into(), r.into()),
                OR::Rshape(&(pos, s)) => RI::Rshape(pos.into(), Symbol::new(s)),
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
            }
        })
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Blame<P>(pub P, pub BlameSource);

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
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

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
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
    RglobalTypeVariableGenerics(P, Symbol, Symbol),
    RsolveFail(P),
    RcstrOnGenerics(P, Positioned<TypeName, P>),
    RlambdaParam(P, R),
    Rshape(P, Symbol),
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
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct BReason(Box<ReasonImpl<BReason, BPos>>);

impl Reason for BReason {
    type Pos = BPos;

    fn mk(cons: impl FnOnce() -> ReasonImpl<Self, Self::Pos>) -> Self {
        let x = cons();
        Self(Box::new(x))
    }

    fn none() -> Self {
        BReason(Box::new(ReasonImpl::Rnone))
    }

    fn pos(&self) -> &BPos {
        use ReasonImpl::*;
        match &*self.0 {
            Rnone => unimplemented!(),
            Rwitness(p) | RwitnessFromDecl(p) | Rhint(p) => p,
            r => unimplemented!("BReason::pos: {:?}", r),
        }
    }

    fn to_oxidized<'a>(
        &self,
        _arena: &'a bumpalo::Bump,
    ) -> &'a oxidized_by_ref::typing_reason::Reason<'a> {
        unimplemented!()
    }

    #[inline]
    fn cons_decl_ty(ty: DeclTy_<BReason>) -> Hc<DeclTy_<BReason>> {
        static CONSER: OnceCell<Conser<DeclTy_<BReason>>> = OnceCell::new();
        CONSER.get_or_init(Conser::new).mk(ty)
    }

    #[inline]
    fn cons_ty(ty: Ty_<BReason, Ty<BReason>>) -> Hc<Ty_<BReason, Ty<BReason>>> {
        static CONSER: OnceCell<Conser<Ty_<BReason, Ty<BReason>>>> = OnceCell::new();
        CONSER.get_or_init(Conser::new).mk(ty)
    }
}

impl Walkable<BReason> for BReason {}

impl<'a> From<oxidized_by_ref::typing_reason::T_<'a>> for BReason {
    fn from(reason: oxidized_by_ref::typing_reason::T_<'a>) -> Self {
        Self::from_oxidized(reason)
    }
}

impl ToOcamlRep for BReason {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> OpaqueValue<'a> {
        unimplemented!()
    }
}

/// A stateless sentinal Reason.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
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

    fn to_oxidized<'a>(
        &self,
        _arena: &'a bumpalo::Bump,
    ) -> &'a oxidized_by_ref::typing_reason::Reason<'a> {
        &oxidized_by_ref::typing_reason::Reason::Rnone
    }

    #[inline]
    fn cons_decl_ty(ty: DeclTy_<NReason>) -> Hc<DeclTy_<NReason>> {
        static CONSER: OnceCell<Conser<DeclTy_<NReason>>> = OnceCell::new();
        CONSER.get_or_init(Conser::new).mk(ty)
    }

    #[inline]
    fn cons_ty(ty: Ty_<NReason, Ty<NReason>>) -> Hc<Ty_<NReason, Ty<NReason>>> {
        static CONSER: OnceCell<Conser<Ty_<NReason, Ty<NReason>>>> = OnceCell::new();
        CONSER.get_or_init(Conser::new).mk(ty)
    }
}

impl Walkable<NReason> for NReason {}

impl<'a> From<oxidized_by_ref::typing_reason::T_<'a>> for NReason {
    fn from(reason: oxidized_by_ref::typing_reason::T_<'a>) -> Self {
        Self::from_oxidized(reason)
    }
}

impl ToOcamlRep for NReason {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        let arena = &bumpalo::Bump::new();
        self.to_oxidized(arena).to_ocamlrep(alloc)
    }
}
