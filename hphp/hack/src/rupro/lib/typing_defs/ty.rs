// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub use crate::decl_defs::ty::{Exact, Prim};
use crate::reason::Reason;
use crate::typing_defs::tyvar::Tyvar;
use hcons::Hc;
use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};
use pos::{Positioned, Symbol, ToOxidized, TypeName};
use std::ops::Deref;

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
    /// Top
    Tmixed,

    /// Bottom
    Tnothing,

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
    Ty_::Tmixed => [],
    Ty_::Tnothing => [],
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

    pub fn mixed(r: R) -> Ty<R> {
        Self::new(r, Ty_::Tmixed)
    }

    pub fn nothing(r: R) -> Ty<R> {
        Self::new(r, Ty_::Tmixed)
    }

    pub fn prim(r: R, prim: Prim) -> Ty<R> {
        Self::new(r, Ty_::Tprim(prim))
    }

    pub fn null(r: R) -> Ty<R> {
        Self::prim(r, Prim::Tnull)
    }
    pub fn void(r: R) -> Ty<R> {
        Self::prim(r, Prim::Tvoid)
    }
    pub fn int(r: R) -> Ty<R> {
        Self::prim(r, Prim::Tint)
    }
    pub fn float(r: R) -> Ty<R> {
        Self::prim(r, Prim::Tfloat)
    }
    pub fn string(r: R) -> Ty<R> {
        Self::prim(r, Prim::Tstring)
    }
    pub fn num(r: R) -> Ty<R> {
        Self::prim(r, Prim::Tnum)
    }

    pub fn arraykey(r: R) -> Ty<R> {
        Self::prim(r, Prim::Tarraykey)
    }

    pub fn fun(r: R, ft: FunType<R>) -> Ty<R> {
        Self::new(r, Ty_::Tfun(ft))
    }

    pub fn union(r: R, tys: Vec<Ty<R>>) -> Self {
        let ln = tys.len();
        if ln == 0 {
            Self::nothing(r)
        } else {
            Self::new(r, Ty_::Tunion(tys))
        }
    }

    pub fn var(r: R, tv: Tyvar) -> Self {
        Self::new(r, Ty_::Tvar(tv))
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

impl<R: Reason> Deref for Ty<R> {
    type Target = Ty_<R, Ty<R>>;
    fn deref(&self) -> &Self::Target {
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
            Ty_::Tmixed => OTy_::Tmixed,
            Ty_::Tnothing => todo!(),
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
    fn to_ocamlrep<'a, A: Allocator>(&'a self, alloc: &'a A) -> OpaqueValue<'a> {
        // This implementation of `to_ocamlrep` (which allocates in an arena,
        // converts to OCaml, then drops the arena) violates a `ToOcamlRep`
        // requirement: we may not drop values after passing them to `alloc.add`
        // or invoking `to_ocamlrep` (else memoization will behave incorrectly
        // in `add_root`). This leads to bizarre behavior (particularly in
        // optimized builds).
        //
        // For example, suppose we're converting a typed AST via ToOcamlRep, and
        // it contains the types `int` and `float`. When converting `int`, we'll
        // construct an arena and arena-allocate Tint, in order to construct the
        // oxidized_by_ref value `Tprim(&Tint)`, and convert that to OCaml. The
        // `ocamlrep::Allocator` will remember that the address of the `&Tint`
        // pointer corresponds to a certain OCaml value, so that when it
        // encounters future instances of that pointer, it can use that same
        // OCaml value rather than allocating a new one. We'd then free the
        // arena once we're finished converting that type. When converting the
        // second type, we construct a new arena, arena-allocate Tfloat, and
        // attempt to construct `Tprim(&Tfloat)`. But if the new arena was
        // allocated in the same location as the old, it may choose the same
        // address for our arena-allocated `Tfloat` as our `Tint` was, and our
        // ocamlrep Allocator will incorrectly use the `Tint` OCaml value.
        //
        // This memoization behavior is only enabled if we invoke
        // `ocamlrep::Allocator::add_root`, so we must take care not to use it
        // (including indirectly, through macros like `ocaml_ffi`) on values
        // containing this type.
        let arena = &bumpalo::Bump::new();
        let ty = self.to_oxidized(arena);
        // SAFETY: Transmute away the lifetime to allow the arena-allocated
        // value to be converted to OCaml. Won't break type safety in Rust, but
        // will produce broken OCaml values if used with `add_root` (see above
        // comment).
        let ty = unsafe {
            std::mem::transmute::<
                &'_ oxidized_by_ref::typing_defs::Ty<'_>,
                &'a oxidized_by_ref::typing_defs::Ty<'a>,
            >(&ty)
        };
        ty.to_ocamlrep(alloc)
    }
}
