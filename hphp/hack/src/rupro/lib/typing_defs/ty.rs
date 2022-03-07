// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::reason::Reason;
use crate::typing_defs::tyvar::Tyvar;
use hcons::Hc;
use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};
use pos::{Positioned, Symbol, ToOxidized, TypeName};

pub use crate::decl_defs::ty::{Exact, Prim};

// TODO: Share the representation from decl_defs
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunParam<R: Reason> {
    pub pos: R::Pos,
    pub name: Option<Symbol>,
    pub ty: Ty<R>,
}

walkable!(FunParam<R> => [ty]);

// TODO: Share the representation from decl_defs
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunType<R: Reason> {
    pub params: Vec<FunParam<R>>,
    pub ret: Ty<R>,
}

walkable!(FunType<R> => [params, ret]);

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

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Ty_<R: Reason, TY> {
    /// A primitive type
    Tprim(Prim),

    /// A wrapper around `FunType`, which contains the full type information
    /// for a function, method, lambda, etc.
    Tfun(FunType<R>),

    /// Any type
    /// TODO: any and err are a bit weird in that they are not actually types
    /// but rather they represent a set of inconsistent bounds on a tyvar
    /// we might want to rethink them prefering a sum type _or_
    /// distinguishing types with `Tany` from those without
    Tany,

    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.add_generic_parameters_and_constraints. The list denotes
    /// type arguments.
    Tgeneric(TypeName, Vec<TY>),

    /// An instance of a class or interface, ty list are the arguments
    /// If exact=Exact, then this represents instances of *exactly* this class
    /// If exact=Nonexact, this also includes subclasses
    Tclass(Positioned<TypeName, R::Pos>, Exact, Vec<TY>),

    Tvar(Tyvar),

    Tunion(Vec<TY>),
}

walkable!(impl<R: Reason, TY> for Ty_<R, TY> =>  {
    Ty_::Tprim(_) => [],
    Ty_::Tfun(fun_type) => [fun_type],
    Ty_::Tany => [],
    Ty_::Tgeneric(_, args) => [args],
    Ty_::Tclass(_, _, args) => [args],
    Ty_::Tunion(args) => [args],
    Ty_::Tvar(_) => [],
});

impl<R: Reason> hcons::Consable for Ty_<R, Ty<R>> {
    #[inline]
    fn conser() -> &'static hcons::Conser<Ty_<R, Ty<R>>> {
        R::ty_conser()
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Ty<R: Reason>(R, Hc<Ty_<R, Ty<R>>>);

walkable!(Ty<R> as visit_ty => [0, 1]);

impl<R: Reason> Ty<R> {
    #[inline]
    pub fn new(reason: R, ty: Ty_<R, Ty<R>>) -> Self {
        Self(reason, Hc::new(ty))
    }

    pub fn prim(r: R, prim: Prim) -> Ty<R> {
        Self::new(r, Ty_::Tprim(prim))
    }

    pub fn void(r: R) -> Ty<R> {
        Self::prim(r, Prim::Tvoid)
    }

    pub fn any(r: R) -> Ty<R> {
        Self::new(r, Ty_::Tany)
    }

    pub fn reason(&self) -> &R {
        &self.0
    }

    pub fn node(&self) -> &Hc<Ty_<R, Ty<R>>> {
        &self.1
    }
}

impl<'a, R: Reason> ToOxidized<'a> for Ty<R> {
    type Output = oxidized_by_ref::typing_defs::Ty<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use oxidized_by_ref::typing_defs::Ty_ as OTy_;
        let r = arena.alloc(self.reason().to_oxidized(arena));
        let ty = match &**self.node() {
            Ty_::Tvar(tv) => OTy_::Tvar((*tv).into()),
            Ty_::Tprim(x) => OTy_::Tprim(arena.alloc(*x)),
            Ty_::Tunion(_) => todo!(),
            Ty_::Tfun(_) => todo!(),
            Ty_::Tany => todo!(),
            Ty_::Tgeneric(_, _) => todo!(),
            Ty_::Tclass(pos_id, exact, tys) => OTy_::Tclass(&*arena.alloc((
                pos_id.to_oxidized(arena),
                *exact,
                &*arena.alloc_slice_fill_iter(
                    tys.iter().map(|ty| &*arena.alloc(ty.to_oxidized(arena))),
                ),
            ))),
        };
        oxidized_by_ref::typing_defs::Ty(r, ty)
    }
}

impl<R: Reason> ToOcamlRep for Ty<R> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        let arena = &bumpalo::Bump::new();
        self.to_oxidized(arena).to_ocamlrep(alloc)
    }
}
