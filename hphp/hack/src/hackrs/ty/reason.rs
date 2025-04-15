// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use core::panic;
use std::fmt::Debug;
use std::hash::Hash;
use std::sync::Arc;

use eq_modulo_pos::EqModuloPos;
use hcons::Conser;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use once_cell::sync::Lazy;
use oxidized::ast::FunKind;
pub use oxidized::typing_reason::ArgPosition;
pub use oxidized::typing_reason::BlameSource;
use pos::BPos;
use pos::NPos;
use pos::Pos;
use pos::Symbol;
use pos::ToOxidized;
use pos::ToOxidizedByRef;
use pos::TypeName;
use serde::Deserialize;
use serde::Serialize;
use serde::de::DeserializeOwned;

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
    + for<'a> From<oxidized::typing_reason::T_>
    + for<'a> From<oxidized_by_ref::typing_reason::T_<'a>>
    + for<'a> ToOxidizedByRef<'a, Output = oxidized_by_ref::typing_reason::T_<'a>>
    + ToOxidized<Output = oxidized::typing_reason::T_>
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

    fn witness_from_decl(pos: Self::Pos) -> Self {
        Self::mk(|| ReasonImpl::FromWitnessDecl(WitnessDecl::WitnessFromDecl(pos)))
    }

    fn hint(pos: Self::Pos) -> Self {
        Self::mk(|| ReasonImpl::FromWitnessDecl(WitnessDecl::Hint(pos)))
    }

    fn instantiate(r1: Self, ty_name: TypeName, r2: Self) -> Self {
        Self::mk(|| ReasonImpl::Instantiate(r1, ty_name, r2))
    }

    fn class_class(pos: Self::Pos, ty_name: TypeName) -> Self {
        Self::mk(|| ReasonImpl::FromWitnessDecl(WitnessDecl::ClassClass(pos, ty_name)))
    }

    fn pos(&self) -> &Self::Pos;

    fn decl_ty_conser() -> &'static Conser<decl::Ty_<Self>>;
    fn local_ty_conser() -> &'static Conser<local::Ty_<Self, local::Ty<Self>>>;
    fn prop_conser() -> &'static Conser<PropF<Self, Prop<Self>>>;

    fn from_oxidized(reason: oxidized::typing_reason::T_) -> Self {
        Self::mk(|| {
            use ReasonImpl as RI;
            use oxidized::typing_reason::T_ as OR;
            use oxidized::typing_reason::WitnessDecl as WD;

            match reason {
                OR::NoReason => RI::NoReason,
                OR::FromWitnessDecl(WD::WitnessFromDecl(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::WitnessFromDecl(pos.into()))
                }

                OR::FromWitnessDecl(WD::Hint(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::Hint(pos.into()))
                }

                OR::FromWitnessDecl(WD::ClassClass(pos, s)) => RI::FromWitnessDecl(
                    WitnessDecl::ClassClass(pos.into(), TypeName(Symbol::new(s))),
                ),

                OR::FromWitnessDecl(WD::VarParamFromDecl(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::VarParamFromDecl(pos.into()))
                }

                OR::FromWitnessDecl(WD::TupleFromSplat(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::TupleFromSplat(pos.into()))
                }

                OR::FromWitnessDecl(WD::VecOrDictKey(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::VecOrDictKey(pos.into()))
                }

                OR::FromWitnessDecl(WD::RetFunKindFromDecl(pos, fun_kind)) => {
                    RI::FromWitnessDecl(WitnessDecl::RetFunKindFromDecl(pos.into(), fun_kind))
                }

                OR::Instantiate(r1, sym, r2) => {
                    RI::Instantiate((*r1).into(), TypeName(Symbol::new(sym)), (*r2).into())
                }
                _ => {
                    panic!("Error occurred: {:#?}", reason)
                }
            }
        })
    }

    fn from_oxidized_by_ref(reason: oxidized_by_ref::typing_reason::T_<'_>) -> Self {
        Self::mk(|| {
            use ReasonImpl as RI;
            use oxidized_by_ref::typing_reason::T_ as OR;
            use oxidized_by_ref::typing_reason::WitnessDecl as WD;

            match reason {
                OR::NoReason => RI::NoReason,
                OR::FromWitnessDecl(&WD::WitnessFromDecl(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::WitnessFromDecl(pos.into()))
                }

                OR::FromWitnessDecl(&WD::Hint(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::Hint(pos.into()))
                }

                OR::FromWitnessDecl(&WD::ClassClass(&(pos, s))) => RI::FromWitnessDecl(
                    WitnessDecl::ClassClass(pos.into(), TypeName(Symbol::new(s))),
                ),

                OR::FromWitnessDecl(&WD::VarParamFromDecl(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::VarParamFromDecl(pos.into()))
                }

                OR::FromWitnessDecl(&WD::TupleFromSplat(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::TupleFromSplat(pos.into()))
                }

                OR::FromWitnessDecl(&WD::VecOrDictKey(pos)) => {
                    RI::FromWitnessDecl(WitnessDecl::VecOrDictKey(pos.into()))
                }

                OR::FromWitnessDecl(&WD::RetFunKindFromDecl(&(pos, fun_kind))) => {
                    RI::FromWitnessDecl(WitnessDecl::RetFunKindFromDecl(pos.into(), fun_kind))
                }

                OR::Instantiate(&(r1, sym, r2)) => {
                    RI::Instantiate(r1.into(), TypeName(Symbol::new(sym)), r2.into())
                }
                _ => {
                    panic!("Error occurred: {:#?}", reason)
                }
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

impl<'a> ToOxidizedByRef<'a> for ExprDepTypeReason {
    type Output = oxidized_by_ref::typing_reason::ExprDepTypeReason<'a>;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use oxidized_by_ref::typing_reason::ExprDepTypeReason as Obr;
        match self {
            ExprDepTypeReason::ERexpr(i) => Obr::ERexpr(*i),
            ExprDepTypeReason::ERstatic => Obr::ERstatic,
            ExprDepTypeReason::ERclass(s) => Obr::ERclass(s.to_oxidized_by_ref(arena)),
            ExprDepTypeReason::ERparent(s) => Obr::ERparent(s.to_oxidized_by_ref(arena)),
            ExprDepTypeReason::ERself(s) => Obr::ERself(s.to_oxidized_by_ref(arena)),
            ExprDepTypeReason::ERpu(s) => Obr::ERpu(s.to_oxidized_by_ref(arena)),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, EqModuloPos, Hash, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub enum WitnessDecl<P> {
    WitnessFromDecl(P),
    IdxVectorFromDecl(P),
    Hint(P),
    ClassClass(P, TypeName),
    VarParamFromDecl(P),
    TupleFromSplat(P),
    VecOrDictKey(P),
    RetFunKindFromDecl(P, FunKind),
}

#[derive(Debug, Clone, PartialEq, Eq, EqModuloPos, Hash, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub enum ReasonImpl<R, P> {
    FromWitnessDecl(WitnessDecl<P>),
    Instantiate(R, TypeName, R),
    NoReason,
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
        BReason(Arc::new(ReasonImpl::NoReason))
    }

    fn pos(&self) -> &BPos {
        use ReasonImpl::*;
        match &*self.0 {
            NoReason => unimplemented!(),
            FromWitnessDecl(WitnessDecl::WitnessFromDecl(p)) => p,
            FromWitnessDecl(WitnessDecl::Hint(p)) => p,
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

impl From<oxidized::typing_reason::T_> for BReason {
    fn from(reason: oxidized::typing_reason::T_) -> Self {
        Self::from_oxidized(reason)
    }
}

impl<'a> From<oxidized_by_ref::typing_reason::Reason<'a>> for BReason {
    fn from(reason: oxidized_by_ref::typing_reason::Reason<'a>) -> Self {
        Self::from_oxidized_by_ref(reason)
    }
}

impl<'a> ToOxidizedByRef<'a> for BReason {
    type Output = oxidized_by_ref::typing_reason::Reason<'a>;
    // Unused
    fn to_oxidized_by_ref(&self, _arena: &'a bumpalo::Bump) -> Self::Output {
        oxidized_by_ref::typing_reason::Reason::NoReason
    }
}

impl ToOxidized for BReason {
    type Output = oxidized::typing_reason::Reason;
    // Unused
    fn to_oxidized(self) -> Self::Output {
        oxidized::typing_reason::Reason::NoReason
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

impl From<oxidized::typing_reason::T_> for NReason {
    fn from(reason: oxidized::typing_reason::T_) -> Self {
        Self::from_oxidized(reason)
    }
}
impl<'a> From<oxidized_by_ref::typing_reason::T_<'a>> for NReason {
    fn from(reason: oxidized_by_ref::typing_reason::T_<'a>) -> Self {
        Self::from_oxidized_by_ref(reason)
    }
}

impl<'a> ToOxidizedByRef<'a> for NReason {
    type Output = oxidized_by_ref::typing_reason::Reason<'a>;

    fn to_oxidized_by_ref(&self, _arena: &'a bumpalo::Bump) -> Self::Output {
        oxidized_by_ref::typing_reason::Reason::NoReason
    }
}

impl ToOxidized for NReason {
    type Output = oxidized::typing_reason::Reason;

    fn to_oxidized(self) -> Self::Output {
        oxidized::typing_reason::Reason::NoReason
    }
}

impl ToOcamlRep for NReason {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        oxidized_by_ref::typing_reason::Reason::NoReason.to_ocamlrep(alloc)
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
