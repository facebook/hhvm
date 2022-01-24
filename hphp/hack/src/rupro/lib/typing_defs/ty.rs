// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(dead_code)]
use std::convert::From;

use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};

use crate::hcons::Consed;
use crate::pos::{PosId, Symbol};
use crate::reason::Reason;

pub type Prim = crate::decl_defs::Prim;

pub type FunParam<R> = crate::decl_defs::FunParam<R, Ty<R>>;

pub type FunType<R> = crate::decl_defs::FunType<R, Ty<R>>;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum ParamMode {
    FPnormal,
    FPinout,
}

impl From<&oxidized::ast_defs::ParamKind> for ParamMode {
    fn from(pk: &oxidized::ast_defs::ParamKind) -> Self {
        match pk {
            oxidized::ast::ParamKind::Pinout(_) => Self::FPinout,
            oxidized::ast::ParamKind::Pnormal => Self::FPnormal,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Exact {
    Exact,
    Nonexact,
}

impl Exact {
    pub fn to_oxidized(&self) -> oxidized::typing_defs::Exact {
        match &self {
            Self::Exact => oxidized::typing_defs::Exact::Exact,
            Self::Nonexact => oxidized::typing_defs::Exact::Nonexact,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Ty_<R: Reason, TY> {
    /// A primitive type
    Tprim(Prim),
    /// A wrapper around `FunType`, which contains the full type information
    /// for a function, method, lambda, etc.
    Tfun(FunType<R>),
    /// Any type
    Tany,
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.add_generic_parameters_and_constraints. The list denotes
    /// type arguments.
    Tgeneric(Symbol, Vec<TY>),
    /// An instance of a class or interface, ty list are the arguments
    /// If exact=Exact, then this represents instances of *exactly* this class
    /// If exact=Nonexact, this also includes subclasses
    Tclass(PosId<R::Pos>, Exact, Vec<TY>),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Ty<R: Reason>(R, Consed<Ty_<R, Ty<R>>>);

impl<R: Reason> Ty<R> {
    pub fn new(reason: R, consed: Consed<Ty_<R, Ty<R>>>) -> Self {
        Self(reason, consed)
    }

    pub fn reason(&self) -> &R {
        &self.0
    }

    pub fn node(&self) -> &Consed<Ty_<R, Ty<R>>> {
        &self.1
    }

    pub fn to_oxidized(&self) -> oxidized::typing_defs::Ty {
        use oxidized::typing_defs::Ty_ as OTy_;
        let r = self.reason().to_oxidized();
        let ty = match &**self.node() {
            Ty_::Tprim(x) => OTy_::Tprim(*x),
            Ty_::Tfun(_) => todo!(),
            Ty_::Tany => todo!(),
            Ty_::Tgeneric(_, _) => todo!(),
            Ty_::Tclass(pos_id, exact, tys) => OTy_::Tclass(
                pos_id.to_oxidized(),
                exact.to_oxidized(),
                tys.iter().map(|ty| ty.to_oxidized()).collect(),
            ),
        };
        oxidized::typing_defs::Ty(r, Box::new(ty))
    }
}

impl<R: Reason> ToOcamlRep for Ty<R> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        self.to_oxidized().to_ocamlrep(alloc)
    }
}
