// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub use crate::decl::ty::{Exact, Prim};
use crate::local::tyvar::Tyvar;
use crate::reason::Reason;
use crate::visitor::{Visitor, Walkable};
use hcons::Hc;
use im::HashSet;
use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};
use oxidized::ast_defs::Variance;
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
    Toption(TY),
    Tnonnull,
}

walkable!(impl<R: Reason, TY> for Ty_<R, TY> =>  {
    Ty_::Tprim(_) => [],
    Ty_::Tfun(fun_type) => [fun_type],
    Ty_::Tany => [],
    Ty_::Tgeneric(_, args) => [args],
    Ty_::Tclass(_, _, args) => [args],
    Ty_::Tunion(args) => [args],
    Ty_::Toption(arg) => [arg],
    Ty_::Tvar(_) => [],
    Ty_::Tnonnull => [],
});

impl<R: Reason> hcons::Consable for Ty_<R, Ty<R>> {
    #[inline]
    fn conser() -> &'static hcons::Conser<Ty_<R, Ty<R>>> {
        R::local_ty_conser()
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Ty<R: Reason>(R, Hc<Ty_<R, Ty<R>>>);

walkable!(Ty<R> as visit_local_ty => [0, 1]);

impl<R: Reason> Ty<R> {
    #[inline]
    pub fn new(reason: R, ty: Ty_<R, Ty<R>>) -> Self {
        Self(reason, Hc::new(ty))
    }

    pub fn with_reason(self, reason: R) -> Self {
        Self(reason, self.1)
    }

    pub fn map_reason<F>(self, f: F) -> Self
    where
        F: FnOnce(R) -> R,
    {
        Self(f(self.0), self.1)
    }

    pub fn shallow_match(&self, other: &Self) -> bool {
        match (self.deref(), other.deref()) {
            (Ty_::Tnonnull, Ty_::Tnonnull) | (Ty_::Tany, Ty_::Tany) => true,
            (Ty_::Tprim(p1), Ty_::Tprim(p2)) => p1 == p2,
            (Ty_::Tclass(cn_sub, exact_sub, _), Ty_::Tclass(cn_sup, exact_sup, _)) => {
                cn_sub.id() == cn_sup.id() && exact_sub == exact_sup
            }
            //   TODO[mjt] compares function flags here
            (Ty_::Tfun(_fty_sub), Ty_::Tfun(_fty_sup)) => true,
            _ => false,
        }
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

    pub fn option(r: R, ty: Ty<R>) -> Self {
        match ty.deref() {
            Ty_::Toption(_) => ty,
            Ty_::Tunion(tys) if tys.is_empty() => Self::null(r),
            _ => Self::new(r, Ty_::Toption(ty)),
        }
    }

    pub fn class(r: R, cname: Positioned<TypeName, R::Pos>, exact: Exact, tys: Vec<Self>) -> Self {
        Self::new(r, Ty_::Tclass(cname, exact, tys))
    }

    pub fn union(r: R, tys: Vec<Ty<R>>) -> Self {
        if tys.len() == 1 {
            tys.into_iter().next().unwrap()
        } else {
            Self::new(r, Ty_::Tunion(tys))
        }
    }

    pub fn intersection(_: R, tys: Vec<Ty<R>>) -> Self {
        if tys.len() == 1 {
            tys.into_iter().next().unwrap()
        } else {
            unimplemented!("Intersection types are not implemented")
        }
    }

    pub fn var(r: R, tv: Tyvar) -> Self {
        Self::new(r, Ty_::Tvar(tv))
    }

    pub fn any(r: R) -> Ty<R> {
        Self::new(r, Ty_::Tany)
    }

    pub fn generic(r: R, ty_name: TypeName, args: Vec<Ty<R>>) -> Self {
        Self::new(r, Ty_::Tgeneric(ty_name, args))
    }

    pub fn nonnull(r: R) -> Ty<R> {
        Self::new(r, Ty_::Tnonnull)
    }

    pub fn is_nonnull(&self) -> bool {
        matches!(self.deref(), Ty_::Tnonnull)
    }

    pub fn is_var(&self) -> bool {
        matches!(self.deref(), Ty_::Tvar(_))
    }

    pub fn mixed(r: R) -> Ty<R> {
        let inner = Self::nonnull(r.clone());
        Self::option(r, inner)
    }

    pub fn nothing(r: R) -> Ty<R> {
        Self::union(r, vec![])
    }

    pub fn reason(&self) -> &R {
        &self.0
    }

    pub fn generic_name(&self) -> Option<&TypeName> {
        match self.deref() {
            Ty_::Tgeneric(name, _) => Some(name),
            _ => None,
        }
    }

    pub fn tyvar_opt(&self) -> Option<&Tyvar> {
        match self.deref() {
            Ty_::Tvar(tv) => Some(tv),
            _ => None,
        }
    }
    pub fn node(&self) -> &Hc<Ty_<R, Ty<R>>> {
        &self.1
    }

    pub fn occurs(&self, tv: Tyvar) -> bool {
        TyvarOccurs::new(tv, self).occurs
    }

    pub fn tyvars<F>(&self, get_tparam_variance: F) -> (HashSet<Tyvar>, HashSet<Tyvar>)
    where
        F: Fn(TypeName) -> Option<Vec<Variance>>,
    {
        let mut covs = HashSet::default();
        let mut contravs = HashSet::default();
        self.tyvars_help(
            Variance::Covariant,
            &mut covs,
            &mut contravs,
            &get_tparam_variance,
        );
        (covs, contravs)
    }

    fn tyvars_help<F>(
        &self,
        variance: Variance,
        covs: &mut HashSet<Tyvar>,
        contravs: &mut HashSet<Tyvar>,
        get_tparam_variance: &F,
    ) where
        F: Fn(TypeName) -> Option<Vec<Variance>>,
    {
        match self.deref() {
            Ty_::Tvar(tv) => match variance {
                Variance::Covariant => {
                    covs.insert(*tv);
                }
                Variance::Contravariant => {
                    contravs.insert(*tv);
                }
                Variance::Invariant => {
                    covs.insert(*tv);
                    contravs.insert(*tv);
                }
            },
            Ty_::Toption(ty) => ty.tyvars_help(variance, covs, contravs, get_tparam_variance),
            Ty_::Tunion(tys) => tys
                .iter()
                .for_each(|ty| ty.tyvars_help(variance, covs, contravs, get_tparam_variance)),
            Ty_::Tfun(ft) => {
                for fp in &ft.params {
                    // TODO[mjt] handle inout params when we have them
                    // for now treat all contravariantly
                    fp.ty
                        .tyvars_help(variance.negate(), covs, contravs, get_tparam_variance);
                }
                ft.ret
                    .tyvars_help(variance, covs, contravs, get_tparam_variance)
            }
            Ty_::Tclass(cn, _, typarams) if !typarams.is_empty() => {
                if let Some(vars) = get_tparam_variance(cn.id()) {
                    typarams.iter().zip(vars.iter()).for_each(|(tp, variance)| {
                        tp.tyvars_help(*variance, covs, contravs, get_tparam_variance)
                    })
                }
            }
            Ty_::Tclass(_, _, _)
            | Ty_::Tgeneric(_, _)
            | Ty_::Tany
            | Ty_::Tnonnull
            | Ty_::Tprim(_) => {}
        }
    }
}

impl<R: Reason> Deref for Ty<R> {
    type Target = Ty_<R, Ty<R>>;
    fn deref(&self) -> &Self::Target {
        &self.1
    }
}

struct TyvarOccurs {
    tv: Tyvar,
    occurs: bool,
}

impl TyvarOccurs {
    fn new<R: Reason>(tv: Tyvar, ty: &Ty<R>) -> Self {
        let mut acc = TyvarOccurs { tv, occurs: false };
        ty.accept(&mut acc);
        acc
    }
}

impl<R: Reason> Visitor<R> for TyvarOccurs {
    fn object(&mut self) -> &mut dyn Visitor<R> {
        self
    }

    fn visit_local_ty(&mut self, ty: &Ty<R>) {
        match ty.deref() {
            Ty_::Tvar(tv2) if self.tv == *tv2 => {
                self.occurs = true;
            }
            _ => {}
        }
        if !self.occurs {
            ty.recurse(self.object())
        }
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
            Ty_::Toption(_) => todo!(),
            Ty_::Tunion(_) => todo!(),
            Ty_::Tfun(_) => todo!(),
            Ty_::Tany => todo!(),
            Ty_::Tnonnull => todo!(),
            Ty_::Tgeneric(x, argl) => OTy_::Tgeneric(&*arena.alloc((
                &*arena.alloc_str(x.as_str()),
                &*arena.alloc_slice_fill_iter(argl.iter().map(|_ty| todo!())),
            ))),
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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::reason::NReason;
    use pos::{NPos, Pos};
    use utils::core::IdentGen;

    #[test]
    fn test_non_var() {
        let ty_int = Ty::int(NReason::none());
        let (covs, contravs) = ty_int.tyvars(|_| None);
        assert!(covs.is_empty());
        assert!(contravs.is_empty());
    }

    #[test]
    fn test_var() {
        let gen = IdentGen::new();
        let tv0: Tyvar = gen.make().into();
        let ty_v0 = Ty::var(NReason::none(), tv0.clone());
        let (covs, contravs) = ty_v0.tyvars(|_| None);
        assert!(covs.contains(&tv0));
        assert!(contravs.is_empty());
    }

    #[test]
    fn test_union() {
        let gen = IdentGen::new();
        let tv0: Tyvar = gen.make().into();
        let ty_v0 = Ty::var(NReason::none(), tv0.clone());
        let ty_union = Ty::union(NReason::none(), vec![ty_v0]);
        let (covs, contravs) = ty_union.tyvars(|_| None);
        assert!(covs.contains(&tv0));
        assert!(contravs.is_empty());
    }

    #[test]
    fn test_fn_ty() {
        let gen = IdentGen::new();
        let tv0: Tyvar = gen.make().into();
        let tv1: Tyvar = gen.make().into();
        let tv2: Tyvar = gen.make().into();

        let params = vec![FunParam {
            pos: NPos::none(),
            name: None,
            ty: Ty::var(NReason::none(), tv0.clone()),
        }];
        let ret = Ty::var(NReason::none(), tv1.clone());

        // #0 -> #1
        let ty_fn1 = Ty::fun(NReason::none(), FunType { params, ret });
        let (covs, contravs) = ty_fn1.tyvars(|_| None);
        assert!(covs.contains(&tv1));
        assert!(contravs.contains(&tv0));

        // (#0 -> #1) -> #2
        let ty_fn2 = Ty::fun(
            NReason::none(),
            FunType {
                params: vec![FunParam {
                    pos: NPos::none(),
                    name: None,
                    ty: ty_fn1,
                }],
                ret: Ty::var(NReason::none(), tv2.clone()),
            },
        );
        let (covs, contravs) = ty_fn2.tyvars(|_| None);
        assert!(covs.contains(&tv0));
        assert!(covs.contains(&tv2));
        assert!(contravs.contains(&tv1));
    }

    #[test]
    fn test_occurs() {
        let gen = IdentGen::new();
        let tv: Tyvar = gen.make().into();
        let ty_v = Ty::var(NReason::none(), tv.clone());

        let tint = Ty::int(NReason::none());
        assert!(!tint.occurs(tv));

        let tunion = Ty::union(NReason::none(), vec![ty_v, tint]);
        assert!(tunion.occurs(tv));

        let tclass = Ty::class(
            NReason::none(),
            Positioned::new(Pos::none(), TypeName::new("C")),
            Exact::Exact,
            vec![tunion],
        );
        assert!(tclass.occurs(tv));
    }
}
