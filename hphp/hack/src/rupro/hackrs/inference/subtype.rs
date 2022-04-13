// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(unused_imports, unused_variables, dead_code)]
use super::solve;
use crate::inference_env::InferenceEnv;
use crate::special_names as SN;
use crate::typaram_env::TyparamEnv;
use crate::typing::env::typing_env_decls::TEnvDecls;
use crate::typing::typing_error::{Error, Result};
use crate::typing_decl_provider::{Class, TypeDecl};
use im::{HashMap, HashSet};
use pos::{Positioned, Symbol, TypeName};
use std::fmt::Debug;
use std::ops::Deref;
use std::rc::Rc;
use ty::local::{Exact, FunType, Prim, Ty, Ty_, Tyvar};
use ty::local_error::TypingError;
use ty::prop::{Cstr, CstrTy, Prop};
use ty::reason::Reason;

pub trait Oracle<R: Reason>: Debug {
    /// Get a class, return `None` if it can't be found.
    fn get_class(&self, name: TypeName) -> Result<Option<Rc<dyn Class<R>>>>;
}

#[derive(Debug, Copy, Clone, Default, PartialEq, Eq)]
pub struct SubtypeConfig {
    pub ignore_generic_params: bool,
}

#[derive(Debug)]
pub struct SubtypeEnv<R: Reason> {
    this_ty: Option<Ty<R>>,
    visited_goals: VisitedGoals<R>,
    inf_env: InferenceEnv<R>,
    tp_env: TyparamEnv<R>,
    decl_env: Box<dyn Oracle<R>>,
}

impl<R: Reason> SubtypeEnv<R> {
    pub fn new(decl_env: Box<dyn Oracle<R>>) -> Self {
        SubtypeEnv {
            this_ty: None,
            visited_goals: VisitedGoals::default(),
            inf_env: InferenceEnv::default(),
            tp_env: TyparamEnv::default(),
            decl_env,
        }
    }
}

/// Normalize the proposition `T <: U`
fn simp_ty<R: Reason>(
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
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
                        Ok(Prop::invalid(None))
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
                        _ => Prop::invalid(None),
                    };
                    Ok(p1.disj(p2, None))
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
                                Ok(p1.disj(p2, None))
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
                    Tnull | Tvoid => Ok(Prop::invalid(None)),
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
                Tprim(_) => Ok(Prop::invalid(None)),
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
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
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
                Ok(Prop::invalid(None))
            }
        }
        Tunion(ty_subs) => simp_conjs_sub(cfg, env, ty_subs, &ty_sup),
        // TODO[mjt] we aren't getting anywhere near HKTs so we should tidy up Tgeneric
        Tgeneric(_, _) if cfg.ignore_generic_params => Ok(Prop::subtype_ty(ty_sub, ty_sup)),
        Tgeneric(tp_name_sub, _) => {
            simp_typaram_ty(cfg, env, (ty_sub.reason(), tp_name_sub), ty_sup)
        }
        Tclass(_, _, _) | Toption(_) | Tnonnull | Tfun(_) | Tany => Ok(Prop::invalid(None)),
    }
}

/// Normalize the proposition `#1 <: T`
fn simp_tvar_ty<R: Reason>(
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
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
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
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
            unimplemented!("Subtype propositions involving classes aren't fully implemented")
        }
    } else if !exact_match {
        Ok(Prop::invalid(None))
    } else {
        unimplemented!("Subtype propositions involving classes aren't fully implemented")
    }
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
            _ => Prop::invalid(None),
        }
    }
}

/// Normalize the propostion `fn <: fn`
fn simp_fun<R: Reason>(
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
    fn_sub: &FunType<R>,
    fn_sup: &FunType<R>,
) -> Result<Prop<R>> {
    unimplemented!("Subtype propositions involving function types aren't implemented")
}

/// Normalize the proposition `#T <: U` where `#T` is a generic type parameter
fn simp_typaram_ty<R: Reason>(
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
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
        GoalResult::AlreadyVisited => Ok(Prop::invalid(None)),

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
                Ok(Prop::invalid(None))
            } else {
                Ok(p)
            }
        }
    }
}

/// Normalize the proposition `T <: #U` where `#U` is a generic type parameter
fn simp_ty_typaram<R: Reason>(
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
    ty_sub: Ty<R>,
    tp_sup: (&R, &TypeName),
) -> Result<Prop<R>> {
    match env
        .visited_goals
        .try_add_visited_generic_sup(tp_sup.1.clone(), &ty_sub)
    {
        // We've seen this type param before so must have gone round a
        // cycle so fail
        GoalResult::AlreadyVisited => Ok(Prop::invalid(None)),

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
                Ok(prop.disj(Prop::subtype_ty(ty_sub, ty_sup), None))
            }
        }
    }
}

/// Normalize the propostion `T1 <: U /\ T2 <: U`
fn simp_conj_sub<R: Reason>(
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
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
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
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
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
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
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
    ty_sub: &Ty<R>,
    ty_sups: I,
) -> Result<Prop<R>>
where
    R: Reason,
    I: IntoIterator<Item = &'a Ty<R>>,
{
    let mut prop = Prop::invalid(None);
    for ty_sup in ty_sups {
        let next = simp_ty(cfg, env, ty_sub, ty_sup)?;
        if next.is_valid() {
            prop = next;
            break;
        } else {
            prop = prop.disj(next, None);
        }
    }
    Ok(prop)
}

/// Normalize the proposition `T <: U1 \/  T <: U2`
fn simp_disjs_sub<'a, R, I>(
    cfg: &SubtypeConfig,
    env: &mut SubtypeEnv<R>,
    ty_subs: I,
    ty_sup: &Ty<R>,
) -> Result<Prop<R>>
where
    R: Reason,
    I: IntoIterator<Item = &'a Ty<R>>,
{
    let mut prop = Prop::invalid(None);
    for ty_sub in ty_subs {
        let next = simp_ty(cfg, env, ty_sub, ty_sup)?;
        if next.is_valid() {
            prop = next;
            break;
        } else {
            prop = prop.disj(next, None);
        }
    }
    Ok(prop)
}

#[derive(Debug, Clone, PartialEq, Eq)]
struct VisitedGoals<R: Reason>(HashMap<TypeName, (HashSet<Ty<R>>, HashSet<Ty<R>>)>);

impl<R: Reason> Default for VisitedGoals<R> {
    fn default() -> Self {
        VisitedGoals(HashMap::new())
    }
}

enum GoalResult {
    AlreadyVisited,
    NewGoal,
}
impl<R: Reason> VisitedGoals<R> {
    /// Add `ty` to the set of visited subtype constraint returning the added bound if
    /// if has not already been visited
    fn try_add_visited_generic_sup(&mut self, tp_name: TypeName, ty: &Ty<R>) -> GoalResult {
        let (lower, _upper) = self
            .0
            .entry(tp_name)
            .or_insert((HashSet::default(), HashSet::default()));
        lower
            .insert(ty.clone())
            .map_or(GoalResult::NewGoal, |_| GoalResult::AlreadyVisited)
    }

    /// Add `ty` to the set of visited supertype constraint returning the added bound if
    /// if has not already been visited
    fn try_add_visited_generic_sub(&mut self, tp_name: TypeName, ty: &Ty<R>) -> GoalResult {
        let (_lower, upper) = self
            .0
            .entry(tp_name)
            .or_insert((HashSet::default(), HashSet::default()));
        upper
            .insert(ty.clone())
            .map_or(GoalResult::NewGoal, |_| GoalResult::AlreadyVisited)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pos::NPos;
    use ty::prop::PropF;
    use ty::reason::NReason;
    use utils::core::IdentGen;

    #[derive(Debug)]
    struct NoClasses;

    impl NoClasses {
        fn new() -> Self {
            NoClasses
        }
    }
    impl<R: Reason> Oracle<R> for NoClasses {
        fn get_class(&self, _name: TypeName) -> Result<Option<Rc<dyn Class<R>>>> {
            Ok(None)
        }
    }

    #[test]
    fn test_visited_goals() {
        let mut goals = VisitedGoals::default();
        let tp_name = TypeName::new("T");
        let ty_int = Ty::int(NReason::none());
        let added_super = goals.try_add_visited_generic_sub(tp_name, &ty_int);
        assert!(matches!(added_super, GoalResult::NewGoal));
    }

    #[test]
    fn test_tvar_ty() {
        let mut env = SubtypeEnv::new(Box::new(NoClasses::new()));
        let cfg = SubtypeConfig::default();
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
        let mut env = SubtypeEnv::new(Box::new(NoClasses::new()));
        let cfg = SubtypeConfig::default();

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
    fn test_disjs_sub() {
        let mut env = SubtypeEnv::new(Box::new(NoClasses::new()));
        let cfg = SubtypeConfig::default();
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
    fn test_typaram_visited() {
        let mut env = SubtypeEnv::new(Box::new(NoClasses::new()));
        let cfg = SubtypeConfig::default();
        let tp = TypeName::new("T");
        let tp_reason = NReason::none();
        let ty_int = Ty::int(NReason::none());

        // if we attempt to subtype T against some supertype that already
        // appears in its set of visited supertypes, we expect the propositionn
        // to be invalid
        env.visited_goals
            .0
            .entry(tp)
            .or_insert((HashSet::default(), HashSet::default()))
            .1
            .insert(ty_int.clone());
        assert!(
            simp_typaram_ty(&cfg, &mut env, (&tp_reason, &tp), ty_int.clone())
                .unwrap()
                .is_unsat()
        );

        env.visited_goals
            .0
            .entry(tp)
            .or_insert((HashSet::default(), HashSet::default()))
            .0
            .insert(ty_int.clone());
        assert!(
            simp_ty_typaram(&cfg, &mut env, ty_int, (&tp_reason, &tp))
                .unwrap()
                .is_unsat()
        );
    }

    #[test]
    fn test_ty_param_sup() {
        let mut env = SubtypeEnv::new(Box::new(NoClasses::new()));
        let cfg = SubtypeConfig::default();
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
        let mut env = SubtypeEnv::new(Box::new(NoClasses::new()));
        let cfg = SubtypeConfig::default();
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
