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
    Eq + Hash + Clone + ToOcamlRep + Walkable<Self> + std::fmt::Debug + Send + Sync + 'static
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

impl ToOcamlRep for NReason {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        let arena = &bumpalo::Bump::new();
        self.to_oxidized(arena).to_ocamlrep(alloc)
    }
}
