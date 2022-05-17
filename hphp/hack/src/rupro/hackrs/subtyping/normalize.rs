// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{config::Config, env::Env, solve, visited_goals::GoalResult};
use crate::typing::typing_error::Result;
use im::HashSet;
use itertools::{izip, Itertools};
use oxidized::ast_defs::Variance;
use pos::{Positioned, Symbol, TypeName};
use std::ops::Deref;
use ty::{
    local::{Exact, FunParam, FunType, Prim, Ty, Ty_, Tyvar},
    local_error::{Primary, TypingError},
    prop::Prop,
    reason::Reason,
};

/// Normalize the proposition `T <: U`
pub fn simp_ty<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    ty_sub: &Ty<R>,
    ty_sup: &Ty<R>,
) -> Result<Prop<R>> {
    use Ty_::*;
    let r_sub = ty_sub.reason();
    let ety_sup = env.inf_env.resolve_ty(ty_sup);
    let ety_sub = env.inf_env.resolve_ty(ty_sub);
    match ety_sup.deref() {
        // -- Super is var -----------------------------------------------------
        Tvar(tv_sup) => {
            match ety_sub.deref() {
                Tunion(_) => simp_ty_common(cfg, env, &ety_sub, &ety_sup),
                Toption(ty_sub_inner) => {
                    let ty_sub_inner = env.inf_env.resolve_ty(ty_sub_inner);
                    // We special case on `mixed <: Tvar _`, adding the entire `mixed` type
                    // as a lower bound. This enables clearer error messages when upper bounds
                    // are added to the type variable: transitive closure picks up the
                    // entire `mixed` type, and not separately consider `null` and `nonnull`
                    if ty_sub_inner.is_nonnull() {
                        Ok(Prop::subtype_ty(ety_sub, ety_sup))
                    } else {
                        let ty_null = Ty::null(r_sub.clone());
                        simp_conj_sub(cfg, env, &ty_sub_inner, &ty_null, &ety_sup)
                    }
                }
                Tvar(tv_sub) if tv_sub == tv_sup => Ok(Prop::valid()),
                _ => Ok(Prop::subtype_ty(ety_sub, ety_sup)),
            }
        }

        // -- Super is union ---------------------------------------------------
        Tunion(ty_sups) => {
            // TOOD[mjt] we don't have negation yet so we're dropping special
            // case code for t <: (#1 | arraykey) here
            match ety_sub.deref() {
                Tunion(_) | Tvar(_) => simp_ty_common(cfg, env, &ety_sub, &ety_sup),
                Tgeneric(_, _) if cfg.ignore_generic_params => {
                    simp_ty_common(cfg, env, &ety_sub, &ety_sup)
                }
                Tprim(p) if matches!(p, Prim::Tnum) => {
                    let r = ety_sub.reason();
                    let ty_float = Ty::float(r.clone());
                    let ty_int = Ty::int(r.clone());
                    simp_conj_sub(cfg, env, &ty_float, &ty_int, ty_sup)
                }
                Toption(ty_sub_inner) => {
                    let r = ety_sub.reason();
                    let ty_null = Ty::null(r.clone());
                    let p = simp_conj_sub(cfg, env, ty_sub_inner, &ty_null, ty_sup)?;
                    if p.is_unsat() {
                        Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
                    } else {
                        Ok(p)
                    }
                }
                _ => {
                    // TODO[mjt] this omits all the like-type pushing for
                    // sound dynamic
                    let p1 = simp_disjs_sup(cfg, env, &ety_sub, ty_sups.iter())?;
                    let p2 = match ety_sub.deref() {
                        // TODO[mjt] It's this kind of thing which makes
                        // understanding subtyping unnecessarily difficult; here
                        // we are matching on the subtype 3 times(!) - twice
                        // here and once in `simp_ty_common` when we really
                        // should pull the generic subtyping logic out instead
                        // and leave this match one level up.
                        // We should go through this and simplify the nested
                        // matches
                        Tgeneric(_, _) => simp_ty_common(cfg, env, &ety_sub, ty_sup)?,
                        _ => Prop::invalid(TypingError::Primary(Primary::Subtype)),
                    };
                    Ok(p1.disj(p2, TypingError::Primary(Primary::Subtype)))
                }
            }
        }

        // -- Super is option --------------------------------------------------
        Toption(ty_sup_inner) => {
            let ety_sup_inner = env.inf_env.resolve_ty(ty_sup_inner);
            // ?nonnull === mixed
            // TODO[mjt] why do we use this encoding?
            if ety_sup_inner.is_nonnull() {
                Ok(Prop::valid())
            } else {
                match ety_sub.deref() {
                    Tprim(p_sub) => match p_sub {
                        Prim::Tnull => Ok(Prop::valid()),
                        Prim::Tvoid => {
                            let p1 = simp_ty(cfg, env, &ety_sub, &ety_sup_inner)?;
                            if p1.is_valid() {
                                Ok(p1)
                            } else {
                                // TODO[mjt]: check on this rule! Here `simp_ty_common`
                                // is `?nonnull == mixed <: ety_sup` (as an implicit upper bound ?)
                                // but we already know that ty_sup is _not_ top, don't we?
                                let p2 = simp_ty_common(cfg, env, &ety_sub, &ety_sup)?;
                                Ok(p1.disj(p2, TypingError::Primary(Primary::Subtype)))
                            }
                        }
                        _ => simp_ty(cfg, env, &ety_sub, &ety_sup_inner),
                    },
                    // ?t <: ?u iff t < ?u
                    // Why?
                    // Since t <: ?t and the transitivity of <: we have t <: ?u
                    // or
                    // If t <: ?u by covariance of ? we have ?t <: ?u and
                    // by idempotence we have ?t <: ??t
                    // so this step preserves the set of solutions
                    Toption(ty_sub_inner) => simp_ty(cfg, env, ty_sub_inner, &ety_sup),
                    // TODO[mjt] Again, this needlessly repeats pattern matching
                    // - lets pull the Tvar case out from `simp_ty_common`
                    Tvar(_) | Tunion(_) => simp_ty_common(cfg, env, &ety_sub, &ety_sup),
                    Tgeneric(_, _) if cfg.ignore_generic_params => {
                        simp_ty_common(cfg, env, &ety_sub, &ety_sup)
                    }
                    Tgeneric(_, _) => simp_disj_sup(cfg, env, &ety_sub, &ety_sup_inner, &ety_sup),
                    // All of these cannot we have t </: null so we have
                    // t < ?u iff t <: u
                    Tnonnull | Tfun(_) | Tclass(_, _, _) => {
                        simp_ty(cfg, env, &ety_sub, &ety_sup_inner)
                    }
                    Tany => {
                        unimplemented!("Subtype propositions involving `Tany` aren't implemented")
                    }
                }
            }
        }
        // -- Super is generic -------------------------------------------------
        Tgeneric(tp_name_sup, _) => match ety_sub.deref() {
            // If subtype and supertype are the same generic parameter, we're done
            Tgeneric(tp_name_sub, _) if tp_name_sub == tp_name_sup => Ok(Prop::valid()),
            Tvar(_) | Tunion(_) => simp_ty_common(cfg, env, &ety_sub, &ety_sup),
            _ if cfg.ignore_generic_params => Ok(Prop::subtype_ty(ety_sub, ety_sup)),
            _ => simp_ty_typaram(cfg, env, ety_sub, (ety_sup.reason(), tp_name_sup)),
        },

        // -- Super is nonnull -------------------------------------------------
        Tnonnull => match ety_sub.deref() {
            Tprim(p_sub) => {
                use Prim::*;
                // TODO[mjt] test for nullable on prims?
                match p_sub {
                    Tnull | Tvoid => Ok(Prop::invalid(TypingError::Primary(Primary::Subtype))),
                    Tint | Tbool | Tfloat | Tstring | Tresource | Tnum | Tarraykey | Tnoreturn => {
                        Ok(Prop::valid())
                    }
                }
            }
            Tnonnull | Tfun(_) | Tclass(_, _, _) => Ok(Prop::valid()),
            Tvar(_) | Tgeneric(_, _) | Toption(_) | Tunion(_) => {
                simp_ty_common(cfg, env, &ety_sub, &ety_sup)
            }
            Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
        },

        // -- Super is prim ----------------------------------------------------
        Tprim(p_sup) => match ety_sub.deref() {
            Tprim(p_sub) => Ok(simp_prim(p_sub, p_sup)),
            Toption(ty_sub_inner) if p_sup.is_tnull() => simp_ty(cfg, env, ty_sub_inner, &ety_sup),
            Tnonnull
            | Tfun(_)
            | Tclass(_, _, _)
            | Tvar(_)
            | Tgeneric(_, _)
            | Toption(_)
            | Tunion(_) => simp_ty_common(cfg, env, &ety_sub, &ety_sup),
            Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
        },

        // -- Super is fun -----------------------------------------------------
        Tfun(fn_sup) => match ety_sub.deref() {
            Tfun(fn_sub) => simp_fun(cfg, env, fn_sub, fn_sup),
            Tprim(_)
            | Toption(_)
            | Tnonnull
            | Tclass(_, _, _)
            | Tvar(_)
            | Tgeneric(_, _)
            | Tunion(_) => simp_ty_common(cfg, env, &ety_sub, &ety_sup),
            Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
        },

        // -- Super is class ---------------------------------------------------
        Tclass(cn_sup, exact_sup, tp_sups) => {
            match ety_sub.deref() {
                Tclass(cn_sub, exact_sub, tp_subs) => simp_class(
                    cfg, env, cn_sub, exact_sub, tp_subs, cn_sup, exact_sup, tp_sups,
                ),
                // TODO[mjt] make a call on whether we bring
                // string <: Stringish
                // string , arraykey ,int, float, num <: XHPChild
                // special-cases across
                Tprim(_) => Ok(Prop::invalid(TypingError::Primary(Primary::Subtype))),
                Tfun(_) | Toption(_) | Tnonnull | Tvar(_) | Tgeneric(_, _) | Tunion(_) => {
                    simp_ty_common(cfg, env, &ety_sub, &ety_sup)
                }
                Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
            }
        }

        // -- Tany should not be in subtyping ----------------------------------
        Tany => unimplemented!("Subtype propositions involving `Tany` aren't implemented"),
    }
}

fn simp_ty_common<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    ty_sub: &Ty<R>,
    ty_sup: &Ty<R>,
) -> Result<Prop<R>> {
    use Ty_::*;
    let ty_sub = env.inf_env.resolve_ty(ty_sub);
    let ty_sup = env.inf_env.resolve_ty(ty_sup);
    match ty_sub.deref() {
        Tvar(tv) => Ok(simp_tvar_ty(cfg, env, ty_sub.reason(), tv, &ty_sup)),
        Tprim(p_sub) => {
            if matches!(p_sub, Prim::Tvoid) {
                // TODO[mjt]: implicit upper bound reason
                let reason =
                    R::implicit_upper_bound(ty_sub.reason().pos().clone(), Symbol::new("?nonnull"));
                let ty_mixed = Ty::mixed(reason);
                simp_ty(cfg, env, &ty_mixed, &ty_sup)
            } else {
                Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
            }
        }
        Tunion(ty_subs) => simp_conjs_sub(cfg, env, ty_subs, &ty_sup),
        // TODO[mjt] we aren't getting anywhere near HKTs so we should tidy up Tgeneric
        Tgeneric(_, _) if cfg.ignore_generic_params => Ok(Prop::subtype_ty(ty_sub, ty_sup)),
        Tgeneric(tp_name_sub, _) => {
            simp_typaram_ty(cfg, env, (ty_sub.reason(), tp_name_sub), ty_sup)
        }
        Tclass(_, _, _) | Toption(_) | Tnonnull | Tfun(_) | Tany => {
            Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
        }
    }
}

/// Normalize the proposition `#1 <: T`
fn simp_tvar_ty<R: Reason>(
    _cfg: &Config,
    env: &mut Env<R>,
    r_sub: &R,
    tv_sub: &Tyvar,
    ty_sup: &Ty<R>,
) -> Prop<R> {
    let ty_sup = solve::remove_tyvar_from_upper_bound(&mut env.inf_env, *tv_sub, ty_sup);
    if env.inf_env.is_mixed(&ty_sup) {
        // The tyvar occurred in the super type and removing it resulted in
        // ty_sup collapsing to top
        Prop::valid()
    } else {
        let ty_sub = Ty::var(r_sub.clone(), *tv_sub);
        Prop::subtype_ty(ty_sub, ty_sup)
    }
}

/// Normalize the proposition `#class<...> <: #class<...>`
fn simp_class<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    cn_sub: &Positioned<TypeName, R::Pos>,
    exact_sub: &Exact,
    tp_subs: &[Ty<R>],
    cn_sup: &Positioned<TypeName, R::Pos>,
    exact_sup: &Exact,
    tp_sups: &[Ty<R>],
) -> Result<Prop<R>> {
    let exact_match = match (exact_sub, exact_sup) {
        (Exact::Nonexact, Exact::Exact) => false,
        _ => true,
    };
    if cn_sub.id() == cn_sup.id() {
        if tp_subs.is_empty() && tp_sups.is_empty() && exact_match {
            Ok(Prop::valid())
        } else {
            let class_def_sub = env.decl_env.get_class(cn_sub.id())?;
            // If class is final then exactness is superfluous
            let is_final = class_def_sub.clone().map_or(false, |cls| cls.is_final());
            if !(exact_match || is_final) {
                Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
            } else if tp_subs.len() != tp_sups.len() {
                // TODO[mjt] these are different cases due to the errors
                // we want to raise
                Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
            } else {
                let variance_sub = if tp_subs.is_empty() {
                    vec![]
                } else {
                    class_def_sub.map_or_else(
                        || vec![Variance::Invariant; tp_subs.len()],
                        |cls| cls.get_tparams().iter().map(|t| (t.variance)).collect_vec(),
                    )
                };
                simp_conjs_with_variance(cfg, env, cn_sub, variance_sub, tp_subs, tp_sups)
            }
        }
    } else if !exact_match {
        // class in subtype position is non-exact and class is supertype position is exact
        Ok(Prop::invalid(TypingError::Primary(Primary::Subtype)))
    } else {
        let class_sub_opt = env.decl_env.get_class(cn_sub.id())?;
        if let Some(class_sub) = class_sub_opt {
            // is our class is subtype positition a declared subtype of the
            // class in supertype position
            match class_sub.get_ancestor(&cn_sup.id()) {
                Some(_dty_sub) => {
                    // instantiate the declared type at the type parameters for
                    // the class in subtype position
                    // TODO[mjt] class localization
                    unimplemented!(
                        "Proposition normalization is not fully implemented for class types"
                    )
                }
                //    None if class_sub.kind
                None => Ok(Prop::invalid(TypingError::Primary(Primary::Subtype))),
            }
        } else {
            // This should have been caught already, in the naming phase
            // TODO[mjt] should we surface some of these implicit assumptions
            // in the Result?
            Ok(Prop::valid())
        }
    }
}

fn simp_conjs_with_variance<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    cn: &Positioned<TypeName, R::Pos>,
    variances: Vec<Variance>,
    ty_subs: &[Ty<R>],
    ty_sups: &[Ty<R>],
) -> Result<Prop<R>> {
    let mut prop = Prop::valid();
    for (variance, ty_sub, ty_sup) in izip!(variances.iter(), ty_subs, ty_sups) {
        let next = simp_with_variance(cfg, env, cn, variance, ty_sub, ty_sup)?;
        if next.is_unsat() {
            prop = next;
            break;
        } else {
            prop = prop.conj(next);
        }
    }
    Ok(prop)
}

fn simp_with_variance<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    _cn: &Positioned<TypeName, R::Pos>,
    variance: &Variance,
    ty_sub: &Ty<R>,
    ty_sup: &Ty<R>,
) -> Result<Prop<R>> {
    let mut this_ty = None;
    std::mem::swap(&mut this_ty, &mut env.this_ty);

    let p = match variance {
        Variance::Covariant => simp_ty(cfg, env, ty_sub, ty_sup),
        Variance::Contravariant =>
        // TODO[mjt] update the reason on the supertype
        {
            simp_ty(cfg, env, ty_sup, ty_sub)
        }
        Variance::Invariant => {
            // TODO[mjt] update the reason on the supertype
            // TODO[mjt] if I use short-circuiting ? here will env be restored?
            match simp_ty(cfg, env, ty_sub, ty_sup) {
                Ok(p1) if !p1.is_unsat() => simp_ty(cfg, env, ty_sup, ty_sub).map(|p2| p1.conj(p2)),
                p1_res => p1_res,
            }
        }
    };
    std::mem::swap(&mut this_ty, &mut env.this_ty);
    p
}

/// Normalize the proposition `prim <: prim`
/// TODO[mjt]: move to or wrap `subtype` test for prim impl?
fn simp_prim<R: Reason>(prim_sub: &Prim, prim_sup: &Prim) -> Prop<R> {
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
fn simp_fun<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    fn_sub: &FunType<R>,
    fn_sup: &FunType<R>,
) -> Result<Prop<R>> {
    let mut prop = Prop::valid();
    for (param_sub, param_sup) in fn_sub.params.iter().zip(fn_sup.params.iter()) {
        let next = simp_fun_param(cfg, env, param_sub, param_sup)?;
        if next.is_unsat() {
            prop = next;
            break;
        } else {
            prop = prop.conj(next);
        }
    }
    if !prop.is_unsat() {
        let next = simp_ty(cfg, env, &fn_sub.ret, &fn_sup.ret)?;
        if next.is_unsat() {
            Ok(next)
        } else {
            Ok(prop.conj(next))
        }
    } else {
        Ok(prop)
    }
}

fn simp_fun_param<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    fn_param_sub: &FunParam<R>,
    fn_param_sup: &FunParam<R>,
) -> Result<Prop<R>> {
    // functions are contravariant in their parameters
    simp_ty(cfg, env, &fn_param_sup.ty, &fn_param_sub.ty)
}

/// Normalize the proposition `#T <: U` where `#T` is a generic type parameter
fn simp_typaram_ty<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    tp_sub: (&R, &TypeName),
    ty_sup: Ty<R>,
) -> Result<Prop<R>> {
    match env
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
            let bounds = env
                .tp_env
                .upper_bounds(tp_sub.1)
                .map_or(HashSet::default(), |bs| bs.clone());
            let p = if bounds.is_empty() {
                let reason =
                    R::implicit_upper_bound(tp_sub.0.pos().clone(), Symbol::new("?nonnull"));
                let ty_mixed = Ty::mixed(reason);
                simp_ty(cfg, env, &ty_mixed, &ty_sup)?
            } else {
                simp_disjs_sub(cfg, env, bounds.iter(), &ty_sup)?
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
fn simp_ty_typaram<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    ty_sub: Ty<R>,
    tp_sup: (&R, &TypeName),
) -> Result<Prop<R>> {
    match env
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
            let bounds = env
                .tp_env
                .lower_bounds(tp_sup.1)
                .map_or(HashSet::default(), |bs| bs.clone());
            let prop = simp_disjs_sup(cfg, env, &ty_sub, bounds.iter())?;
            if prop.is_valid() {
                Ok(prop)
            } else {
                let ty_sup = Ty::generic(tp_sup.0.clone(), tp_sup.1.clone(), vec![]);
                Ok(prop.disj(
                    Prop::subtype_ty(ty_sub, ty_sup),
                    TypingError::Primary(Primary::Subtype),
                ))
            }
        }
    }
}

/// Normalize the propostion `T1 <: U /\ T2 <: U`
fn simp_conj_sub<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    ty_sub1: &Ty<R>,
    ty_sub2: &Ty<R>,
    ty_sup: &Ty<R>,
) -> Result<Prop<R>> {
    let p1 = simp_ty(cfg, env, ty_sub1, ty_sup)?;
    if p1.is_unsat() {
        Ok(p1)
    } else {
        let p2 = simp_ty(cfg, env, ty_sub2, ty_sup)?;
        Ok(p1.conj(p2))
    }
}

/// Normalize the proposition `T <: U1 \/  T <: U2`
fn simp_disj_sup<R: Reason>(
    cfg: &Config,
    env: &mut Env<R>,
    ty_sub: &Ty<R>,
    ty_sup1: &Ty<R>,
    ty_sup2: &Ty<R>,
) -> Result<Prop<R>> {
    let p1 = simp_ty(cfg, env, ty_sub, ty_sup1)?;
    if p1.is_valid() {
        Ok(p1)
    } else {
        let p2 = simp_ty(cfg, env, ty_sub, ty_sup2)?;
        Ok(p1.conj(p2))
    }
}

/// Normalize the proposition `T1 <: U /\  T2 <: U /\ ... /\ Tn <: U`
fn simp_conjs_sub<'a, R, I>(
    cfg: &Config,
    env: &mut Env<R>,
    ty_subs: I,
    ty_sup: &Ty<R>,
) -> Result<Prop<R>>
where
    R: Reason,
    I: IntoIterator<Item = &'a Ty<R>>,
{
    let mut prop = Prop::valid();
    for ty_sub in ty_subs {
        let next = simp_ty(cfg, env, ty_sub, ty_sup)?;
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
fn simp_disjs_sup<'a, R, I>(
    cfg: &Config,
    env: &mut Env<R>,
    ty_sub: &Ty<R>,
    ty_sups: I,
) -> Result<Prop<R>>
where
    R: Reason,
    I: IntoIterator<Item = &'a Ty<R>>,
{
    let mut prop = Prop::invalid(TypingError::Primary(Primary::Subtype));
    for ty_sup in ty_sups {
        let next = simp_ty(cfg, env, ty_sub, ty_sup)?;
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
fn simp_disjs_sub<'a, R, I>(
    cfg: &Config,
    env: &mut Env<R>,
    ty_subs: I,
    ty_sup: &Ty<R>,
) -> Result<Prop<R>>
where
    R: Reason,
    I: IntoIterator<Item = &'a Ty<R>>,
{
    let mut prop = Prop::invalid(TypingError::Primary(Primary::Subtype));
    for ty_sub in ty_subs {
        let next = simp_ty(cfg, env, ty_sub, ty_sup)?;
        if next.is_valid() {
            prop = next;
            break;
        } else {
            prop = prop.disj(next, TypingError::Primary(Primary::Subtype));
        }
    }
    Ok(prop)
}

#[cfg(test)]
mod tests {
    use super::*;
    use pos::Pos;
    use ty::{prop::PropF, reason::NReason};
    use utils::core::IdentGen;

    #[test]
    fn test_tvar_ty() {
        let mut env = Env::default();
        let cfg = Config::default();
        let gen = IdentGen::new();

        let tv_0: Tyvar = gen.make().into();

        let also_tv_0: Tyvar = tv_0.clone();
        let r_tv_0 = NReason::none();
        let ty_v0 = Ty::var(NReason::none(), tv_0);

        // #0 <: #0 => true
        let p0 = simp_tvar_ty(&cfg, &mut env, &r_tv_0, &also_tv_0, &ty_v0);
        assert!(p0.is_valid());

        // #0 <: int => #0 <: int
        let ty_int = Ty::int(NReason::none());
        let p1 = simp_tvar_ty(&cfg, &mut env, &r_tv_0, &also_tv_0, &ty_int);
        assert!(matches!(p1.deref(), PropF::Subtype(_, _)));

        // #0 <: int | 0 => true
        let ty_int_or_tv0 = Ty::union(NReason::none(), vec![ty_v0.clone(), ty_int.clone()]);
        let p2 = simp_tvar_ty(&cfg, &mut env, &r_tv_0, &also_tv_0, &ty_int_or_tv0);
        assert!(p2.is_valid());

        // #0 <: (int | (int | 0)) => true
        let ty_int_or_int_or_tv0 = Ty::union(NReason::none(), vec![ty_int.clone(), ty_int_or_tv0]);
        let p3 = simp_tvar_ty(&cfg, &mut env, &r_tv_0, &also_tv_0, &ty_int_or_int_or_tv0);
        assert!(p3.is_valid());

        // #0 <: ?#0 => true
        let ty_v0_opt = Ty::option(NReason::none(), ty_v0.clone());
        let p4 = simp_tvar_ty(&cfg, &mut env, &r_tv_0, &also_tv_0, &ty_v0_opt);
        assert!(p4.is_valid());
    }

    #[test]
    fn test_prim() {
        let mut env = Env::default();
        let cfg = Config::default();

        // Subtypes of arraykey
        let ty_arraykey = Ty::arraykey(NReason::none());
        // int <: arraykey
        let ty_int = Ty::int(NReason::none());
        assert!(
            simp_ty(&cfg, &mut env, &ty_int, &ty_arraykey)
                .unwrap()
                .is_valid()
        );
        // string <: arraykey
        let ty_string = Ty::string(NReason::none());
        assert!(
            simp_ty(&cfg, &mut env, &ty_string, &ty_arraykey)
                .unwrap()
                .is_valid()
        );

        // Subtypes of num
        let ty_num = Ty::num(NReason::none());
        // int <: num
        assert!(
            simp_ty(&cfg, &mut env, &ty_int, &ty_num)
                .unwrap()
                .is_valid()
        );
        // float <: num
        let ty_float = Ty::float(NReason::none());
        assert!(
            simp_ty(&cfg, &mut env, &ty_float, &ty_num)
                .unwrap()
                .is_valid()
        );

        // num === (int | float)
        let ty_num_as_union = Ty::union(NReason::none(), vec![ty_int.clone(), ty_float.clone()]);
        assert!(
            simp_ty(&cfg, &mut env, &ty_num, &ty_num_as_union)
                .unwrap()
                .is_valid()
        );

        // Subtypes of nonnull
        let ty_nonnull = Ty::nonnull(NReason::none());
        // int <: nonnull
        assert!(
            simp_ty(&cfg, &mut env, &ty_int, &ty_nonnull)
                .unwrap()
                .is_valid()
        );
        // float <: nonnull
        assert!(
            simp_ty(&cfg, &mut env, &ty_int, &ty_nonnull)
                .unwrap()
                .is_valid()
        );
        // string <: nonnull
        assert!(
            simp_ty(&cfg, &mut env, &ty_int, &ty_nonnull)
                .unwrap()
                .is_valid()
        );
        // null </: nonnull
        let ty_null = Ty::null(NReason::none());
        assert!(
            simp_ty(&cfg, &mut env, &ty_null, &ty_nonnull)
                .unwrap()
                .is_unsat()
        );
        // void </: nonnull
        let ty_void = Ty::void(NReason::none());
        assert!(
            simp_ty(&cfg, &mut env, &ty_void, &ty_nonnull)
                .unwrap()
                .is_unsat()
        );
    }

    #[test]
    fn test_fn() {
        let mut env = Env::default();
        let cfg = Config::default();

        let ty_fn1 = Ty::fun(
            NReason::none(),
            FunType {
                params: vec![FunParam {
                    name: None,
                    pos: Pos::none(),
                    ty: Ty::arraykey(NReason::none()),
                }],
                ret: Ty::float(NReason::none()),
            },
        );

        let ty_fn2 = Ty::fun(
            NReason::none(),
            FunType {
                params: vec![FunParam {
                    name: None,
                    pos: Pos::none(),
                    ty: Ty::int(NReason::none()),
                }],
                ret: Ty::num(NReason::none()),
            },
        );

        // arraykey -> float <: int -> num
        // int <: arraykey & float <: num
        assert!(
            simp_ty(&cfg, &mut env, &ty_fn1, &ty_fn2)
                .unwrap()
                .is_valid()
        );

        // int -> num </: arraykey -> float
        assert!(
            simp_ty(&cfg, &mut env, &ty_fn2, &ty_fn1)
                .unwrap()
                .is_unsat()
        );

        // refl
        assert!(
            simp_ty(&cfg, &mut env, &ty_fn1, &ty_fn1)
                .unwrap()
                .is_valid()
        )
    }

    #[test]
    fn test_disjs_sub() {
        let mut env = Env::default();
        let cfg = Config::default();
        let ty_num = Ty::num(NReason::none());
        let ty_subs_all_ok = vec![Ty::int(NReason::none()), Ty::float(NReason::none())];
        let ty_subs_one_ok = vec![Ty::string(NReason::none()), Ty::float(NReason::none())];
        let ty_subs_none_ok = vec![Ty::string(NReason::none()), Ty::null(NReason::none())];
        assert!(
            simp_disjs_sub(&cfg, &mut env, ty_subs_all_ok.iter(), &ty_num)
                .unwrap()
                .is_valid()
        );
        assert!(
            simp_disjs_sub(&cfg, &mut env, ty_subs_one_ok.iter(), &ty_num)
                .unwrap()
                .is_valid()
        );
        assert!(
            simp_disjs_sub(&cfg, &mut env, ty_subs_none_ok.iter(), &ty_num)
                .unwrap()
                .is_unsat()
        );
    }

    #[test]
    fn test_ty_param_sup() {
        let mut env = Env::default();
        let cfg = Config::default();
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
            simp_typaram_ty(&cfg, &mut env, (&tp_reason, &tp_name), ty_arraykey)
                .unwrap()
                .is_valid()
        );
    }

    #[test]
    fn test_ty_param_sub() {
        let mut env = Env::default();
        let cfg = Config::default();
        let tp_name = TypeName::new("T");
        let tp_reason = NReason::none();
        let ty_int = Ty::int(NReason::none());
        let ty_arraykey = Ty::arraykey(NReason::none());

        env.tp_env.add_lower_bound(tp_name.clone(), ty_arraykey);
        assert!(
            simp_ty_typaram(&cfg, &mut env, ty_int, (&tp_reason, &tp_name))
                .unwrap()
                .is_valid()
        );
    }
}
