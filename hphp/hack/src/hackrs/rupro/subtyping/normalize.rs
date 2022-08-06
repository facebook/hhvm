// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::Deref;
use std::rc::Rc;

use im::HashSet;
use itertools::izip;
use oxidized::ast_defs::Variance;
use pos::Symbol;
use pos::TypeName;
use ty::local::Exact;
use ty::local::FunParam;
use ty::local::FunType;
use ty::local::Prim;
use ty::local::Ty;
use ty::local::Ty_;
use ty::local::Tyvar;
use ty::local_error::Primary;
use ty::local_error::TypingError;
use ty::prop::Prop;
use ty::reason::Reason;

use crate::inference_env::InferenceEnv;
use crate::subtyping::oracle::Oracle;
use crate::subtyping::solve;
use crate::subtyping::visited_goals::GoalResult;
use crate::subtyping::visited_goals::VisitedGoals;
use crate::typaram_env::TyparamEnv;
use crate::typing::typing_error::Result;

/// Some read-only configuration that influences normalization.
#[derive(Debug, Copy, Clone, Default, PartialEq, Eq)]
pub struct NormalizeConfig {
    pub ignore_generic_params: bool,
}

/// The normalization environment.
///
/// The normalization environment contains everything that's needed to normalize
/// subtyping (and other) propositions.
#[derive(Debug, Clone)]
pub struct NormalizeEnv<R: Reason> {
    /// The localized type for `$this`.
    pub this_ty: Option<Ty<R>>,

    pub visited_goals: VisitedGoals<R>,

    /// The inference environment. Basically a collection of type variables with
    /// variance annotation and constraints.
    pub inf_env: InferenceEnv<R>,

    /// The type parameter environment.
    pub tp_env: TyparamEnv<R>,

    /// An oracle that can answer external questions.
    pub decl_env: Rc<dyn Oracle<R>>,

    /// Some read-only configuration that influences normalization.
    pub config: NormalizeConfig,
}

impl<R: Reason> NormalizeEnv<R> {
    /// Create a new normalizition environment.
    pub fn new(
        this_ty: Option<Ty<R>>,
        inf_env: InferenceEnv<R>,
        tp_env: TyparamEnv<R>,
        decl_env: Rc<dyn Oracle<R>>,
    ) -> Self {
        Self {
            this_ty,
            inf_env,
            tp_env,
            decl_env,
            visited_goals: Default::default(),
            config: Default::default(),
        }
    }

    /// Normalize the proposition `T <: U`
    pub fn simp_ty(&mut self, ty_sub: &Ty<R>, ty_sup: &Ty<R>) -> Result<Prop<R>> {
        use Ty_::*;
        let r_sub = ty_sub.reason();
        let ety_sup = self.inf_env.resolve_ty(ty_sup);
        let ety_sub = self.inf_env.resolve_ty(ty_sub);
        match ety_sup.deref() {
            // -- Super is var -------------------------------------------------
            Tvar(tv_sup) => {
                match ety_sub.deref() {
                    // union(YS) <: X <=> forall(YS,\Y. Y <: X)
                    Tunion(_) => self.simp_ty_common(&ety_sub, &ety_sup),
                    Toption(ty_sub_inner) => {
                        let ty_sub_inner = self.inf_env.resolve_ty(ty_sub_inner);
                        // We special case on `mixed <: Tvar _`, adding the entire `mixed` type
                        // as a lower bound. This enables clearer error messages when upper bounds
                        // are added to the type variable: transitive closure picks up the
                        // entire `mixed` type, and not separately consider `null` and `nonnull`
                        // TODO[mjt] - note the handling of encodings
                        if ty_sub_inner.is_nonnull() {
                            Ok(Prop::subtype(ety_sub, ety_sup))
                        } else {
                            // ?X <: Y <=> null <: Y , X <: Y
                            let ty_null = Ty::null(r_sub.clone());
                            self.simp_conj_sub(&ty_sub_inner, &ty_null, &ety_sup)
                        }
                    }
                    // X <: X <=> true
                    Tvar(tv_sub) if tv_sub == tv_sup => Ok(Prop::valid()),
                    Tvar(_)
                    | Tprim(_)
                    | Tgeneric(_, _)
                    | Tclass(_, _, _)
                    | Tintersection(_)
                    | Tfun(_)
                    | Tnonnull => Ok(Prop::subtype(ety_sub, ety_sup)),
                    Tany => {
                        unimplemented!("Subtype propositions involving `Tany` aren't implemented")
                    }
                }
            }

            // -- Super is union -----------------------------------------------
            Tunion(ty_sups) => {
                // TOOD[mjt] we don't have negation yet so we're dropping special
                // case code for t <: (#1 | arraykey) here
                match ety_sub.deref() {
                    // union(XS) <: union(YS) <=> forall(XS,\X. X <: union(YS))
                    // TODO[mjt] when flattening down `simp_ty_common`, lets start
                    // with the tyvar cases.
                    Tunion(_) | Tvar(_) => self.simp_ty_common(&ety_sub, &ety_sup),
                    Tgeneric(_, _) if self.config.ignore_generic_params => {
                        self.simp_ty_common(&ety_sub, &ety_sup)
                    }
                    Tprim(p) if matches!(p, Prim::Tnum) => {
                        let r = ety_sub.reason();
                        let ty_float = Ty::float(r.clone());
                        let ty_int = Ty::int(r.clone());
                        self.simp_conj_sub(&ty_float, &ty_int, ty_sup)
                    }
                    Toption(ty_sub_inner) => {
                        let r = ety_sub.reason();
                        let ty_null = Ty::null(r.clone());
                        let p = self.simp_conj_sub(ty_sub_inner, &ty_null, ty_sup)?;
                        if p.is_unsat() {
                            Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
                        } else {
                            Ok(p)
                        }
                    }
                    // TODO[mjt] this is a good example of where we are using
                    // a heuristic in hope of improving completeness; show
                    // comparison of DNF <: CNF case
                    // Try to handle the case
                    // `A & B <: (A & B) | C` <=> A&B <: A&B \/ A&B <:C
                    Tintersection(_)
                        if ty_sups
                            .iter()
                            .any(|ty| self.inf_env.resolve_ty(ty).is_intersection()) =>
                    {
                        self.simp_sub_union(&ety_sub, &ety_sup, ty_sups)
                    }

                    // Try to handle the case
                    // A & (B|C) <: B|C <=> A <: B|C \/ B|C <: B|C
                    Tintersection(ty_subs)
                        if ty_subs
                            .iter()
                            .any(|ty| self.inf_env.resolve_ty(ty).is_union()) =>
                    {
                        // It's sound to reduce t1 & t2 <: t to (t1 <: t) \/ (t2 <: t)
                        // but not complete.
                        self.simp_disjs_sub(ty_subs.iter(), &ety_sup)
                    }
                    Tintersection(_) => self.simp_sub_union(&ety_sub, &ety_sup, ty_sups),
                    _ => {
                        self.simp_sub_union(&ety_sub, &ety_sup, ty_sups)
                        // let p1 = self.simp_disjs_sup(&ety_sub, ty_sups.iter())?;
                        // let p2 = match ety_sub.deref() {
                        //     Tgeneric(_, _) => self.simp_ty_common(&ety_sub, ty_sup)?,
                        //     _ => Prop::invalid(TypingError::Primary(Primary::Subtype)),
                        // };
                        // Ok(p1.disj(p2, TypingError::Primary(Primary::Subtype)))
                    }
                }
            }

            // -- Super is option ----------------------------------------------
            // ?nonnull === mixed
            Toption(ty_sup_inner) if self.inf_env.resolve_ty(ty_sup_inner).is_nonnull() => {
                Ok(Prop::valid())
            }
            Toption(ty_sup_inner) => {
                match ety_sub.deref() {
                    Tprim(p_sub) => match p_sub {
                        Prim::Tnull => Ok(Prop::valid()),
                        Prim::Tvoid => {
                            let p1 = self.simp_ty(&ety_sub, ty_sup_inner)?;
                            if p1.is_valid() {
                                Ok(p1)
                            } else {
                                // TODO[mjt]: check on this rule! Here `simp_ty_common`
                                // is `?nonnull == mixed <: ety_sup` (as an implicit upper bound ?)
                                // but we already know that ty_sup is _not_ top, don't we?
                                let p2 = self.simp_ty_common(&ety_sub, &ety_sup)?;
                                Ok(p1.disj(p2, TypingError::Primary(Primary::Subtype)))
                            }
                        }
                        _ => self.simp_ty(&ety_sub, ty_sup_inner),
                    },
                    // ?t <: ?u iff t < ?u
                    // Why?
                    // Since t <: ?t and the transitivity of <: we have t <: ?u
                    // or
                    // If t <: ?u by covariance of ? we have ?t <: ?u and
                    // by idempotence we have ?t <: ??t
                    // so this step preserves the set of solutions
                    Toption(ty_sub_inner) => self.simp_ty(ty_sub_inner, &ety_sup),
                    // TODO[mjt] Again, this needlessly repeats pattern matching
                    // - lets pull the Tvar case out from `simp_ty_common`
                    Tvar(_) | Tunion(_) => self.simp_ty_common(&ety_sub, &ety_sup),
                    Tgeneric(_, _) if self.config.ignore_generic_params => {
                        self.simp_ty_common(&ety_sub, &ety_sup)
                    }
                    Tgeneric(_, _) => self.simp_disj_sup(&ety_sub, ty_sup_inner, &ety_sup),
                    // All of these cannot we have t </: null so we have
                    // t < ?u iff t <: u
                    Tnonnull | Tfun(_) | Tclass(_, _, _) => self.simp_ty(&ety_sub, ty_sup_inner),
                    Tintersection(_) =>
                    // TODO[mjt] there is another heuristic in OCaml for this case which
                    // mentions osciallations - follow up on this:
                    // A <: ?B iff A & nonnull <: B
                    // Only apply if B is a type variable or an intersection, to avoid oscillating
                    // forever between this case and the previous one.
                    {
                        self.simp_ty_common(&ety_sub, &ety_sup)
                    }
                    Tany => {
                        unimplemented!("Subtype propositions involving `Tany` aren't implemented")
                    }
                }
            }

            // -- Super is generic ---------------------------------------------
            Tgeneric(tp_name_sup, _) => match ety_sub.deref() {
                // If subtype and supertype are the same generic parameter, we're done
                Tgeneric(tp_name_sub, _) if tp_name_sub == tp_name_sup => Ok(Prop::valid()),
                Tvar(_) | Tunion(_) => self.simp_ty_common(&ety_sub, &ety_sup),
                Tgeneric(_, _)
                | Toption(_)
                | Tintersection(_)
                | Tfun(_)
                | Tclass(_, _, _)
                | Tnonnull
                | Tprim(_) => {
                    if self.config.ignore_generic_params {
                        Ok(Prop::subtype(ety_sub, ety_sup))
                    } else {
                        self.simp_ty_typaram(ety_sub, (ety_sup.reason(), tp_name_sup))
                    }
                }
                Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
            },

            // -- Super is nonnull ---------------------------------------------
            Tnonnull => match ety_sub.deref() {
                Tprim(p_sub) => {
                    use Prim::*;
                    // TODO[mjt] test for nullable on prims?
                    match p_sub {
                        Tnull | Tvoid => Ok(Prop::invalid(TypingError::Primary(Primary::Subtype))),
                        Tint | Tbool | Tfloat | Tstring | Tresource | Tnum | Tarraykey
                        | Tnoreturn => Ok(Prop::valid()),
                    }
                }
                Tnonnull | Tfun(_) | Tclass(_, _, _) => Ok(Prop::valid()),
                Tvar(_) | Tgeneric(_, _) | Toption(_) | Tunion(_) | Tintersection(_) => {
                    self.simp_ty_common(&ety_sub, &ety_sup)
                }
                Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
            },

            // -- Super is prim ------------------------------------------------
            Tprim(p_sup) => match ety_sub.deref() {
                Tprim(p_sub) => Ok(Self::simp_prim(p_sub, p_sup)),
                Toption(ty_sub_inner) if p_sup.is_tnull() => self.simp_ty(ty_sub_inner, &ety_sup),
                Tnonnull
                | Tfun(_)
                | Tclass(_, _, _)
                | Tvar(_)
                | Tgeneric(_, _)
                | Toption(_)
                | Tunion(_)
                | Tintersection(_) => self.simp_ty_common(&ety_sub, &ety_sup),
                Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
            },

            // -- Super is fun -------------------------------------------------
            Tfun(fn_sup) => match ety_sub.deref() {
                Tfun(fn_sub) => self.simp_fun(fn_sub, fn_sup),
                Tprim(_)
                | Toption(_)
                | Tnonnull
                | Tclass(_, _, _)
                | Tvar(_)
                | Tgeneric(_, _)
                | Tunion(_)
                | Tintersection(_) => self.simp_ty_common(&ety_sub, &ety_sup),
                Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
            },

            // -- Super is class -----------------------------------------------
            Tclass(cn_sup, exact_sup, tp_sups) => {
                match ety_sub.deref() {
                    Tclass(_,exact_sub,_)
                       if matches!(exact_sub, Exact::Nonexact) && matches!(exact_sup, Exact::Exact) =>
                        // class(_ , Nonexact, _) <: class(_, Exact, _) <=> false
                        Ok(Prop::invalid(TypingError::Primary(Primary::Subtype))),
                    Tclass(cn_sub, _exact_sub, tp_subs) if cn_sub.id() == cn_sup.id() => {
                        self.simp_equal_class(cn_sub.id(), tp_subs,  tp_sups)
                    }
                    Tclass(cn_sub, _exact_sub, tp_subs)  =>
                       match self.decl_env.get_ancestor(cn_sub.id(), tp_subs, cn_sup.id())? {
                        Some ( up ) => self.simp_ty(&up, &ety_sup),
                        _ => Ok(Prop::invalid(TypingError::Primary(Primary::Subtype))),
                    }
                    // TODO[mjt] make a call on whether we bring
                    // string <: Stringish
                    // string , arraykey ,int, float, num <: XHPChild
                    // special-cases across
                    Tprim(_) => Ok(Prop::invalid(TypingError::Primary(Primary::Subtype))),
                    Tfun(_)
                    | Toption(_)
                    | Tnonnull
                    | Tvar(_)
                    | Tgeneric(_, _)
                    | Tunion(_)
                    | Tintersection(_) => self.simp_ty_common(&ety_sub, &ety_sup),
                    Tany => {
                        unimplemented!("Subtype propositions involving `Tany` aren't implemented")
                    }
                }
            }

            // -- Super is intersection ----------------------------------------
            Tintersection(_) if ety_sub.is_union() => self.simp_ty_common(&ety_sub, &ety_sup),
            Tintersection(ty_sups) => self.simp_conjs_sup(&ety_sub, ty_sups),

            // -- Tany should not be in subtyping ------------------------------
            Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
        }
    }

    // TODO[mjt] - I think there are a few examples where we attempt to
    // normalize then, if the resulting proposition is invalid, we try something
    // else. It probably makes more sense to add a combinator. It is also
    // worth looking where this is being used and why
    fn simp_sub_union(
        &mut self,
        ty_sub: &Ty<R>,
        ty_sup: &Ty<R>,
        ty_sups: &[Ty<R>],
    ) -> Result<Prop<R>> {
        // TODO[mjt] this omits all the like-type pushing for
        // sound dynamic
        let p1 = self.simp_disjs_sup(ty_sub, ty_sups.iter())?;
        let p2 = match ty_sub.deref() {
            Ty_::Tgeneric(_, _) => self.simp_ty_common(ty_sub, ty_sup)?,
            _ => Prop::invalid(TypingError::Primary(Primary::Subtype)),
        };
        Ok(p1.disj(p2, TypingError::Primary(Primary::Subtype)))
    }

    fn simp_ty_common(&mut self, ty_sub: &Ty<R>, ty_sup: &Ty<R>) -> Result<Prop<R>> {
        use Ty_::*;
        let ty_sub = self.inf_env.resolve_ty(ty_sub);
        let ty_sup = self.inf_env.resolve_ty(ty_sup);
        match ty_sub.deref() {
            Tvar(tv) => Ok(self.simp_tvar_ty(ty_sub.reason(), tv, &ty_sup)),
            Tprim(p_sub) => {
                if matches!(p_sub, Prim::Tvoid) {
                    // TODO[mjt]: implicit upper bound reason
                    let reason = R::implicit_upper_bound(
                        ty_sub.reason().pos().clone(),
                        Symbol::new("?nonnull"),
                    );
                    let ty_mixed = Ty::mixed(reason);
                    self.simp_ty(&ty_mixed, &ty_sup)
                } else {
                    Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
                }
            }
            Tunion(ty_subs) => self.simp_conjs_sub(ty_subs, &ty_sup),
            // TODO[mjt] we aren't getting anywhere near HKTs so we should tidy up Tgeneric
            Tgeneric(_, _) if self.config.ignore_generic_params => {
                Ok(Prop::subtype(ty_sub, ty_sup))
            }
            Tgeneric(tp_name_sub, _) => {
                self.simp_typaram_ty((ty_sub.reason(), tp_name_sub), ty_sup)
            }
            Tclass(_, _, _) | Toption(_) | Tnonnull | Tfun(_) => {
                Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
            }
            Tintersection(ty_subs) => {
                // It's sound to reduce t1 & t2 <: t to (t1 <: t) || (t2 <: t),
                // but not complete.
                self.simp_disjs_sub(ty_subs.iter(), &ty_sup)
            }
            Tany => {
                unimplemented!("Subtype propositions involving `Tany` are not implemented")
            }
        }
    }

    /// Normalize the proposition `#1 <: T`
    fn simp_tvar_ty(&mut self, r_sub: &R, tv_sub: &Tyvar, ty_sup: &Ty<R>) -> Prop<R> {
        let ty_sup = solve::remove_tyvar_from_upper_bound(&mut self.inf_env, *tv_sub, ty_sup);
        if self.inf_env.is_mixed(&ty_sup) {
            // The tyvar occurred in the super type and removing it resulted in
            // ty_sup collapsing to top
            Prop::valid()
        } else {
            let ty_sub = Ty::var(r_sub.clone(), *tv_sub);
            Prop::subtype(ty_sub, ty_sup)
        }
    }

    /// Normalize the proposition `#class<...> <: #class<...>`
    fn simp_equal_class(
        &mut self,
        name: TypeName,
        tp_subs: &[Ty<R>],
        tp_sups: &[Ty<R>],
    ) -> Result<Prop<R>> {
        if tp_subs.is_empty() && tp_sups.is_empty() {
            // class(X, _ , []) <: class(X, _, []) <=> true
            // No type params & exactness agrees
            Ok(Prop::valid())
        } else if !self.decl_env.is_final(name)?.unwrap_or(false) {
            // class(X,_,_) <: class(X,_,_) <=> !is_final(X) | false
            Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
        } else if tp_subs.len() != tp_sups.len() {
            // class(X,_,XS) <: class(X,_,YS) <=> len(XS) != len(YS) | false
            // TODO[mjt] these are different cases due to the errors
            // we want to raise
            rupro_todo_mark!(TypingError);
            Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
        } else {
            // class(X,_,XS) <: class(X,_,YS) <=> decl_variance(X,VS) | sub(VS,XS,YS)
            // TODO[mjt] - we implicitly assume that naming has done its job!
            let variance_sub = self.decl_env.get_variance(name)?.unwrap();
            self.simp_conjs_with_variance(variance_sub, tp_subs, tp_sups)
        }
    }

    fn simp_conjs_with_variance(
        &mut self,
        variances: Vec<Variance>,
        ty_subs: &[Ty<R>],
        ty_sups: &[Ty<R>],
    ) -> Result<Prop<R>> {
        let mut prop = Prop::valid();
        // sub(Covariant::VS,X::XS,Y::YS) <=> X <: Y , sub(VS,XS,YS)
        // sub(Contravariant::VS,X::XS,Y::YS) <=> Y <: X, sub(VS,XS,YS)
        // sub(Invariant::VS,X::XS,Y::YS) <=> X <: Y , Y <: X, sub(VS,XS,YS)
        // sub(_,X::_,[]) <=> fail
        // sub(_,[],Y::_) <=> fail
        for (variance, ty_sub, ty_sup) in izip!(variances.iter(), ty_subs, ty_sups) {
            let next = self.simp_with_variance(variance, ty_sub, ty_sup)?;
            if next.is_unsat() {
                prop = next;
                break;
            } else {
                prop = prop.conj(next);
            }
        }
        Ok(prop)
    }

    fn simp_with_variance(
        &mut self,
        variance: &Variance,
        ty_sub: &Ty<R>,
        ty_sup: &Ty<R>,
    ) -> Result<Prop<R>> {
        let mut this_ty = None;
        std::mem::swap(&mut this_ty, &mut self.this_ty);

        let p = match variance {
            Variance::Covariant => self.simp_ty(ty_sub, ty_sup),
            Variance::Contravariant =>
            // TODO[mjt] update the reason on the supertype
            {
                self.simp_ty(ty_sup, ty_sub)
            }
            Variance::Invariant => {
                // TODO[mjt] update the reason on the supertype
                // TODO[mjt] if I use short-circuiting ? here will env be restored?
                match self.simp_ty(ty_sub, ty_sup) {
                    Ok(p1) if !p1.is_unsat() => self.simp_ty(ty_sup, ty_sub).map(|p2| p1.conj(p2)),
                    p1_res => p1_res,
                }
            }
        };
        std::mem::swap(&mut this_ty, &mut self.this_ty);
        p
    }

    /// Normalize the proposition `prim <: prim`
    /// TODO[mjt]: move to or wrap `subtype` test for prim impl?
    fn simp_prim(prim_sub: &Prim, prim_sup: &Prim) -> Prop<R> {
        if prim_sub == prim_sup {
            Prop::valid()
        } else {
            use Prim::*;
            match (prim_sub, prim_sup) {
                (Tint | Tfloat, Tnum) => Prop::valid(),
                (Tint | Tstring, Tarraykey) => Prop::valid(),
                _ => Prop::invalid(TypingError::Primary(Primary::Subtype)),
            }
        }
    }

    /// Normalize the propostion `fn <: fn`
    fn simp_fun(&mut self, fn_sub: &FunType<R>, fn_sup: &FunType<R>) -> Result<Prop<R>> {
        let mut prop = Prop::valid();
        for (param_sub, param_sup) in fn_sub.params.iter().zip(fn_sup.params.iter()) {
            let next = self.simp_fun_param(param_sub, param_sup)?;
            if next.is_unsat() {
                prop = next;
                break;
            } else {
                prop = prop.conj(next);
            }
        }
        if !prop.is_unsat() {
            let next = self.simp_ty(&fn_sub.ret, &fn_sup.ret)?;
            if next.is_unsat() {
                Ok(next)
            } else {
                Ok(prop.conj(next))
            }
        } else {
            Ok(prop)
        }
    }

    fn simp_fun_param(
        &mut self,
        fn_param_sub: &FunParam<R>,
        fn_param_sup: &FunParam<R>,
    ) -> Result<Prop<R>> {
        // functions are contravariant in their parameters
        self.simp_ty(&fn_param_sup.ty, &fn_param_sub.ty)
    }

    /// Normalize the proposition `#T <: U` where `#T` is a generic type parameter
    fn simp_typaram_ty(&mut self, tp_sub: (&R, &TypeName), ty_sup: Ty<R>) -> Result<Prop<R>> {
        match self
            .visited_goals
            .try_add_visited_generic_sub(tp_sub.1.clone(), &ty_sup)
        {
            // We've seen this type param before so must have gone round a
            // cycle so fail
            // TODO[mjt] do we want indicate failure here? Doesn't a cycle indicate
            // something is wrong?
            GoalResult::AlreadyVisited => Ok(Prop::invalid(TypingError::Primary(Primary::Subtype))),

            // Otherwise, we collect all the upper bounds ("as" constraints) on
            // the generic parameter, and check each of these in turn against
            // ty_super until one of them succeeds
            GoalResult::NewGoal => {
                let bounds = self
                    .tp_env
                    .upper_bounds(tp_sub.1)
                    .map_or(HashSet::default(), |bs| bs.clone());
                let p = if bounds.is_empty() {
                    let reason =
                        R::implicit_upper_bound(tp_sub.0.pos().clone(), Symbol::new("?nonnull"));
                    let ty_mixed = Ty::mixed(reason);
                    self.simp_ty(&ty_mixed, &ty_sup)?
                } else {
                    self.simp_disjs_sub(bounds.iter(), &ty_sup)?
                };
                // TODO[mjt] we check this here to get a nicer message, presumably?
                if p.is_unsat() {
                    Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
                } else {
                    Ok(p)
                }
            }
        }
    }

    /// Normalize the proposition `T <: #U` where `#U` is a generic type parameter
    fn simp_ty_typaram(&mut self, ty_sub: Ty<R>, tp_sup: (&R, &TypeName)) -> Result<Prop<R>> {
        match self
            .visited_goals
            .try_add_visited_generic_sup(tp_sup.1.clone(), &ty_sub)
        {
            // We've seen this type param before so must have gone round a
            // cycle so fail
            GoalResult::AlreadyVisited => Ok(Prop::invalid(TypingError::Primary(Primary::Subtype))),

            // Collect all the lower bounds ("super" constraints) on the
            // generic parameter, and check ty_sub against each of them in turn
            // until one of them succeeds
            GoalResult::NewGoal => {
                let bounds = self
                    .tp_env
                    .lower_bounds(tp_sup.1)
                    .map_or(HashSet::default(), |bs| bs.clone());
                let prop = self.simp_disjs_sup(&ty_sub, bounds.iter())?;
                if prop.is_valid() {
                    Ok(prop)
                } else {
                    let ty_sup = Ty::generic(tp_sup.0.clone(), tp_sup.1.clone(), vec![]);
                    Ok(prop.disj(
                        Prop::subtype(ty_sub, ty_sup),
                        TypingError::Primary(Primary::Subtype),
                    ))
                }
            }
        }
    }

    /// Normalize the propostion `T1 <: U /\ T2 <: U`
    fn simp_conj_sub(
        &mut self,
        ty_sub1: &Ty<R>,
        ty_sub2: &Ty<R>,
        ty_sup: &Ty<R>,
    ) -> Result<Prop<R>> {
        let p1 = self.simp_ty(ty_sub1, ty_sup)?;
        if p1.is_unsat() {
            Ok(p1)
        } else {
            let p2 = self.simp_ty(ty_sub2, ty_sup)?;
            Ok(p1.conj(p2))
        }
    }

    /// Normalize the proposition `T <: U1 \/  T <: U2`
    fn simp_disj_sup(
        &mut self,
        ty_sub: &Ty<R>,
        ty_sup1: &Ty<R>,
        ty_sup2: &Ty<R>,
    ) -> Result<Prop<R>> {
        let p1 = self.simp_ty(ty_sub, ty_sup1)?;
        if p1.is_valid() {
            Ok(p1)
        } else {
            let p2 = self.simp_ty(ty_sub, ty_sup2)?;
            Ok(p1.conj(p2))
        }
    }

    /// Normalize the proposition `T1 <: U /\  T2 <: U /\ ... /\ Tn <: U`
    fn simp_conjs_sub(&mut self, ty_subs: &[Ty<R>], ty_sup: &Ty<R>) -> Result<Prop<R>> {
        let mut prop = Prop::valid();
        for ty_sub in ty_subs {
            let next = self.simp_ty(ty_sub, ty_sup)?;
            if next.is_unsat() {
                prop = next;
                break;
            } else {
                prop = prop.conj(next);
            }
        }
        Ok(prop)
    }

    fn simp_conjs_sup(&mut self, ty_sub: &Ty<R>, ty_sups: &[Ty<R>]) -> Result<Prop<R>> {
        let mut prop = Prop::valid();
        for ty_sup in ty_sups {
            let next = self.simp_ty(ty_sub, ty_sup)?;
            if next.is_unsat() {
                prop = next;
                break;
            } else {
                prop = prop.conj(next);
            }
        }
        Ok(prop)
    }

    /// Normalize the proposition `T <: U1 \/  T <: U2`
    fn simp_disjs_sup<'a>(
        &mut self,
        ty_sub: &Ty<R>,
        ty_sups: impl Iterator<Item = &'a Ty<R>>,
    ) -> Result<Prop<R>> {
        let mut prop = Prop::invalid(TypingError::Primary(Primary::Subtype));
        for ty_sup in ty_sups {
            let next = self.simp_ty(ty_sub, ty_sup)?;
            if next.is_valid() {
                prop = next;
                break;
            } else {
                prop = prop.disj(next, TypingError::Primary(Primary::Subtype));
            }
        }
        Ok(prop)
    }

    /// Normalize the proposition `T <: U1 \/  T <: U2`
    fn simp_disjs_sub<'a>(
        &mut self,
        ty_subs: impl Iterator<Item = &'a Ty<R>>,
        ty_sup: &Ty<R>,
    ) -> Result<Prop<R>> {
        let mut prop = Prop::invalid(TypingError::Primary(Primary::Subtype));
        for ty_sub in ty_subs {
            let next = self.simp_ty(ty_sub, ty_sup)?;
            if next.is_valid() {
                prop = next;
                break;
            } else {
                prop = prop.disj(next, TypingError::Primary(Primary::Subtype));
            }
        }
        Ok(prop)
    }
}

#[cfg(test)]
mod tests {
    use oxidized::typing_defs_flags::FunTypeFlags;
    use pos::Pos;
    use ty::prop::Cstr;
    use ty::prop::PropF;
    use ty::reason::NReason;
    use utils::core::IdentGen;

    use super::*;
    use crate::subtyping::oracle::NoClasses;

    fn default_env<R: Reason>() -> NormalizeEnv<R> {
        NormalizeEnv {
            this_ty: None,
            visited_goals: VisitedGoals::default(),
            inf_env: InferenceEnv::default(),
            tp_env: TyparamEnv::default(),
            decl_env: Rc::new(NoClasses),
            config: Default::default(),
        }
    }

    #[test]
    fn test_tvar_ty() {
        let mut env = default_env();
        let gen = IdentGen::new();

        let tv_0: Tyvar = gen.make().into();

        let also_tv_0: Tyvar = tv_0.clone();
        let r_tv_0 = NReason::none();
        let ty_v0 = Ty::var(NReason::none(), tv_0);

        // #0 <: #0 => true
        let p0 = env.simp_tvar_ty(&r_tv_0, &also_tv_0, &ty_v0);
        assert!(p0.is_valid());

        // #0 <: int => #0 <: int
        let ty_int = Ty::int(NReason::none());
        let p1 = env.simp_tvar_ty(&r_tv_0, &also_tv_0, &ty_int);
        assert!(if let PropF::Atom(cstr) = p1.deref() {
            matches!(cstr, Cstr::Subtype(_, _))
        } else {
            false
        });
        // #0 <: int | 0 => true
        let ty_int_or_tv0 = Ty::union(NReason::none(), vec![ty_v0.clone(), ty_int.clone()]);
        let p2 = env.simp_tvar_ty(&r_tv_0, &also_tv_0, &ty_int_or_tv0);
        assert!(p2.is_valid());

        // #0 <: (int | (int | 0)) => true
        let ty_int_or_int_or_tv0 = Ty::union(NReason::none(), vec![ty_int.clone(), ty_int_or_tv0]);
        let p3 = env.simp_tvar_ty(&r_tv_0, &also_tv_0, &ty_int_or_int_or_tv0);
        assert!(p3.is_valid());

        // #0 <: ?#0 => true
        let ty_v0_opt = Ty::option(NReason::none(), ty_v0.clone());
        let p4 = env.simp_tvar_ty(&r_tv_0, &also_tv_0, &ty_v0_opt);
        assert!(p4.is_valid());
    }

    #[test]
    fn test_prim() {
        let mut env = default_env();

        // Subtypes of arraykey
        let ty_arraykey = Ty::arraykey(NReason::none());
        // int <: arraykey
        let ty_int = Ty::int(NReason::none());
        assert!(env.simp_ty(&ty_int, &ty_arraykey).unwrap().is_valid());
        // string <: arraykey
        let ty_string = Ty::string(NReason::none());
        assert!(env.simp_ty(&ty_string, &ty_arraykey).unwrap().is_valid());

        // Subtypes of num
        let ty_num = Ty::num(NReason::none());
        // int <: num
        assert!(env.simp_ty(&ty_int, &ty_num).unwrap().is_valid());
        // float <: num
        let ty_float = Ty::float(NReason::none());
        assert!(env.simp_ty(&ty_float, &ty_num).unwrap().is_valid());

        // num === (int | float)
        let ty_num_as_union = Ty::union(NReason::none(), vec![ty_int.clone(), ty_float.clone()]);
        assert!(env.simp_ty(&ty_num, &ty_num_as_union).unwrap().is_valid());

        // Subtypes of nonnull
        let ty_nonnull = Ty::nonnull(NReason::none());
        // int <: nonnull
        assert!(env.simp_ty(&ty_int, &ty_nonnull).unwrap().is_valid());
        // float <: nonnull
        assert!(env.simp_ty(&ty_int, &ty_nonnull).unwrap().is_valid());
        // string <: nonnull
        assert!(env.simp_ty(&ty_int, &ty_nonnull).unwrap().is_valid());
        // null </: nonnull
        let ty_null = Ty::null(NReason::none());
        assert!(env.simp_ty(&ty_null, &ty_nonnull).unwrap().is_unsat());
        // void </: nonnull
        let ty_void = Ty::void(NReason::none());
        assert!(env.simp_ty(&ty_void, &ty_nonnull).unwrap().is_unsat());
    }

    #[test]
    fn test_fn() {
        let mut env = default_env();

        let ty_fn1 = Ty::fun(
            NReason::none(),
            FunType {
                tparams: vec![].into_boxed_slice(),
                params: vec![FunParam {
                    name: None,
                    pos: Pos::none(),
                    ty: Ty::arraykey(NReason::none()),
                }],
                flags: FunTypeFlags::empty(),
                ret: Ty::float(NReason::none()),
            },
        );

        let ty_fn2 = Ty::fun(
            NReason::none(),
            FunType {
                tparams: vec![].into_boxed_slice(),
                params: vec![FunParam {
                    name: None,
                    pos: Pos::none(),
                    ty: Ty::int(NReason::none()),
                }],
                flags: FunTypeFlags::empty(),
                ret: Ty::num(NReason::none()),
            },
        );

        // arraykey -> float <: int -> num
        // int <: arraykey & float <: num
        assert!(env.simp_ty(&ty_fn1, &ty_fn2).unwrap().is_valid());

        // int -> num </: arraykey -> float
        assert!(env.simp_ty(&ty_fn2, &ty_fn1).unwrap().is_unsat());

        // refl
        assert!(env.simp_ty(&ty_fn1, &ty_fn1).unwrap().is_valid())
    }

    #[test]
    fn test_disjs_sub() {
        let mut env = default_env();
        let ty_num = Ty::num(NReason::none());
        let ty_subs_all_ok = vec![Ty::int(NReason::none()), Ty::float(NReason::none())];
        let ty_subs_one_ok = vec![Ty::string(NReason::none()), Ty::float(NReason::none())];
        let ty_subs_none_ok = vec![Ty::string(NReason::none()), Ty::null(NReason::none())];
        assert!(
            env.simp_disjs_sub(ty_subs_all_ok.iter(), &ty_num)
                .unwrap()
                .is_valid()
        );
        assert!(
            env.simp_disjs_sub(ty_subs_one_ok.iter(), &ty_num)
                .unwrap()
                .is_valid()
        );
        assert!(
            env.simp_disjs_sub(ty_subs_none_ok.iter(), &ty_num)
                .unwrap()
                .is_unsat()
        );
    }

    #[test]
    fn test_ty_param_sup() {
        let mut env = default_env();
        let tp_name = TypeName::new("T");
        let tp_reason = NReason::none();
        let ty_int = Ty::int(NReason::none());
        let ty_arraykey = Ty::arraykey(NReason::none());

        env.tp_env.add_upper_bound(tp_name.clone(), ty_int);
        // subtyping our typaram against arraykey will:
        // add it to the visited supertypes
        // check that we can subtype all existing upper bounds against it
        // since our only upper bound is int, this is int <: arraykey
        assert!(
            env.simp_typaram_ty((&tp_reason, &tp_name), ty_arraykey)
                .unwrap()
                .is_valid()
        );
    }

    #[test]
    fn test_ty_param_sub() {
        let mut env = default_env();
        let tp_name = TypeName::new("T");
        let tp_reason = NReason::none();
        let ty_int = Ty::int(NReason::none());
        let ty_arraykey = Ty::arraykey(NReason::none());

        env.tp_env.add_lower_bound(tp_name.clone(), ty_arraykey);
        assert!(
            env.simp_ty_typaram(ty_int, (&tp_reason, &tp_name))
                .unwrap()
                .is_valid()
        );
    }

    #[test]
    fn test_union_sub() {
        let mut env = default_env();
        let ty_int = Ty::int(NReason::none());
        let ty_string = Ty::string(NReason::none());
        let ty_bool = Ty::bool(NReason::none());
        let ty_sub1 = Ty::union(NReason::none(), vec![ty_string.clone(), ty_int]);
        let ty_sub2 = Ty::union(NReason::none(), vec![ty_string, ty_bool]);
        let ty_sup = Ty::arraykey(NReason::none());
        //  (t1 | ... | tn) <: 2 <=>  t1 <: t /\  ... /\ tn <: t
        assert!(env.simp_ty(&ty_sub1, &ty_sup).unwrap().is_valid());
        assert!(env.simp_ty(&ty_sub2, &ty_sup).unwrap().is_unsat());
    }
    #[test]
    fn test_union_sup() {
        let mut env = default_env();

        let ty_sub = Ty::int(NReason::none());
        let ty_arraykey = Ty::arraykey(NReason::none());
        let ty_string = Ty::string(NReason::none());
        let ty_bool = Ty::bool(NReason::none());
        let ty_sup1 = Ty::union(NReason::none(), vec![ty_string.clone(), ty_arraykey]);
        let ty_sup2 = Ty::union(NReason::none(), vec![ty_string, ty_bool]);

        //  t <: (t1 | ... | tn) <=>  t <: t1 \/  ... \/ t <: tn
        assert!(env.simp_ty(&ty_sub, &ty_sup1).unwrap().is_valid());
        assert!(env.simp_ty(&ty_sub, &ty_sup2).unwrap().is_unsat());
    }

    #[test]
    fn test_intersect_sub() {
        let mut env = default_env();

        let ty_float = Ty::float(NReason::none());
        let ty_string = Ty::string(NReason::none());
        let ty_bool = Ty::bool(NReason::none());

        let ty_sub1 = Ty::intersection(NReason::none(), vec![ty_string, ty_float.clone()]);
        let ty_sub2 = Ty::intersection(NReason::none(), vec![ty_bool, ty_float]);
        let ty_sup = Ty::arraykey(NReason::none());

        //  (t1 & ... & tn) <: t <=>  t1 <: t \/  ... \/ tn <: t
        assert!(env.simp_ty(&ty_sub1, &ty_sup).unwrap().is_valid());
        assert!(env.simp_ty(&ty_sub2, &ty_sup).unwrap().is_unsat());
    }

    #[test]
    fn test_intersect_sup() {
        let mut env = default_env();

        let ty_num = Ty::num(NReason::none());
        let ty_arraykey = Ty::arraykey(NReason::none());
        let ty_bool = Ty::bool(NReason::none());

        let ty_sub = Ty::int(NReason::none());
        let ty_sup1 = Ty::intersection(NReason::none(), vec![ty_num, ty_arraykey.clone()]);
        let ty_sup2 = Ty::intersection(NReason::none(), vec![ty_bool, ty_arraykey]);

        // t <: (t1 & ... & tn) <=>  t <: t1 /\  ... /\ t <: tn
        assert!(env.simp_ty(&ty_sub, &ty_sup1).unwrap().is_valid());
        assert!(env.simp_ty(&ty_sub, &ty_sup2).unwrap().is_unsat());
    }

    #[test]
    fn test_intersect_sub_union() {
        let mut env = default_env();

        let ty_arraykey = Ty::arraykey(NReason::none());
        let ty_bool = Ty::bool(NReason::none());
        let ty_float = Ty::float(NReason::none());

        // (arraykey & bool) <: float | (arraykey & bool)
        let ty_sub1 = Ty::intersection(NReason::none(), vec![ty_arraykey.clone(), ty_bool.clone()]);
        let ty_sup1 = Ty::union(NReason::none(), vec![ty_float.clone(), ty_sub1.clone()]);
        assert!(env.simp_ty(&ty_sub1, &ty_sup1).unwrap().is_valid());

        // float & (arraykey | bool) <: (arraykey | bool)
        let ty_sup2 = Ty::union(NReason::none(), vec![ty_arraykey, ty_bool]);
        let ty_sub2 = Ty::intersection(NReason::none(), vec![ty_float, ty_sup2.clone()]);
        assert!(env.simp_ty(&ty_sub2, &ty_sup2).unwrap().is_valid());
    }
}
