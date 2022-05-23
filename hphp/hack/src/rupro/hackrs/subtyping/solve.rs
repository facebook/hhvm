// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(unused_imports, unused_variables, dead_code)]
use super::NormalizeEnv;
use crate::inference_env::InferenceEnv;
use crate::subtyping::Subtyper;
use crate::typing::typing_error::Result;
use im::HashSet;
use itertools::{Either, Itertools};
use oxidized::ast_defs::Variance;
use std::ops::Deref;
use std::rc::Rc;
use ty::local::{FunParam, FunType, Ty, Ty_, Tyvar, Variance as V};
use ty::local_error::TypingError;
use ty::prop::CstrTy;
use ty::reason::Reason;

pub fn solve<R: Reason>(
    env: &mut NormalizeEnv<R>,
    tv: Tyvar,
    reason: &R,
) -> Result<Option<Vec<TypingError<R>>>> {
    let mut errs = vec![];
    // Repeatedly solve & then resolve the tyvar. We stop when:
    // - We encounter a cylcle (`resolve` returns an `err`)
    // - The tyvar is bound to a concrete type
    // - We get back the same tyvar
    while env
        .inf_env
        .resolve(reason, tv)
        .map(|ty| ty.tyvar_opt().map_or(false, |tv2| tv != *tv2))
        .unwrap_or(false)
    {
        solve_step(env, tv, reason)?
            .into_iter()
            .for_each(|err| errs.push(err));
    }
    if errs.is_empty() {
        Ok(None)
    } else {
        Ok(Some(errs))
    }
}

pub fn always_solve_wrt_variance_or_down<R: Reason>(
    env: &mut NormalizeEnv<R>,
    tv: Tyvar,
    reason: &R,
) -> Result<Option<Vec<TypingError<R>>>> {
    let errs1 = solve(env, tv, reason)?;
    let err2 = if !env.inf_env.is_solved(&tv) {
        bind_to_lower_bound(env, tv, reason, false)?
    } else {
        None
    };
    match (errs1, err2) {
        (Some(mut errs1), Some(err2)) => {
            errs1.push(err2);
            Ok(Some(errs1))
        }
        (errs1, err2) => Ok(errs1.or_else(|| err2.map(|e| vec![e]))),
    }
}

fn solve_step<R: Reason>(
    env: &mut NormalizeEnv<R>,
    tv: Tyvar,
    reason: &R,
) -> Result<Option<TypingError<R>>> {
    match try_bind_equal_bound(env, tv, reason, false)? {
        TryBindResult::Unbound => {
            try_bind_variance(env, tv, reason).map(|bind_res| bind_res.typing_error())
        }
        res => Ok(res.typing_error()),
    }
}

/// The result of trying to solve a type variable
enum TryBindResult<R: Reason> {
    Bound(Option<TypingError<R>>),
    Unbound,
}

impl<R: Reason> TryBindResult<R> {
    pub fn typing_error(self) -> Option<TypingError<R>> {
        match self {
            Self::Bound(err_opt) => err_opt,
            _ => None,
        }
    }
}

/// Solve a type variable by binding to its lower or upper bound, depending on
/// its variance
fn try_bind_variance<R: Reason>(
    env: &mut NormalizeEnv<R>,
    tv: Tyvar,
    reason: &R,
) -> Result<TryBindResult<R>> {
    match env.inf_env.variance(&tv) {
        V::Bivariant | V::Covariant => {
            let err_opt = bind_to_lower_bound(env, tv, reason, false)?;
            Ok(TryBindResult::Bound(err_opt))
        }
        V::Contravariant => {
            let err_opt = bind_to_upper_bound(env, tv, reason)?;
            Ok(TryBindResult::Bound(err_opt))
        }
        V::Invariant => Ok(TryBindResult::Unbound),
    }
}

/// Solve a type variable by binding it to the union of its lower bounds.
/// If freshen=true, first freshen the covariant and contravariant components of
/// the bounds.
/// TODO[mjt]: move to env impl
fn bind_to_lower_bound<R: Reason>(
    env: &mut NormalizeEnv<R>,
    tv: Tyvar,
    reason: &R,
    freshen: bool,
) -> Result<Option<TypingError<R>>> {
    let (cstrs, tys): (Vec<_>, Vec<_>) = env
        .inf_env
        .lower_bounds(&tv)
        .unwrap_or_default()
        .into_iter()
        .partition_map(|cty| match cty {
            CstrTy::Cstr(cstr) => Either::Left(cstr),
            CstrTy::Locl(lty) => Either::Right(remove_tyvar_from_lower_bound(
                &mut env.inf_env,
                tv.clone(),
                &lty,
            )),
        });
    // Don't solve if the lower bounds contains a `CstrTy::Cstr`
    if !cstrs.is_empty() {
        Ok(None)
    } else {
        // TODO[mjt] simplify unions
        let ty = Ty::union(reason.clone(), tys.clone());
        // 'freshen' type parameters if necessary and add the propositions
        let ty = if freshen {
            let newty = freshen_inside(env, &ty)?;
            // TODO[hverr, mjt]: self should probably be a subtyper
            let mut subtyper = Subtyper::new(
                &mut env.inf_env,
                &mut env.tp_env,
                env.decl_env.clone(),
                None,
            );
            // Since we have only added fresh type variables, this cannot fail
            let freshen_ty_err_opt_ = subtyper.subtype(&ty, &newty)?;
            newty
        } else {
            ty
        };

        // remove any tyvars in lower bounds from upper bounds
        let tvs_lower = tys
            .iter()
            .filter_map(|ty| match ty.deref() {
                Ty_::Tvar(tv) => Some(*tv),
                _ => None,
            })
            .collect::<HashSet<Tyvar>>();
        env.inf_env.remove_tyvar_upper_bound(tv, &tvs_lower);

        // bind to the supplied type
        Ok(bind_help(env, tv, ty))
    }
}

/// Solve a type variable by binding it to the intersection of its upper bounds.
/// TODO[mjt]: move to env impl
fn bind_to_upper_bound<R: Reason>(
    env: &mut NormalizeEnv<R>,
    tv: Tyvar,
    reason: &R,
) -> Result<Option<TypingError<R>>> {
    let (cstrs, tys): (Vec<_>, Vec<_>) = env
        .inf_env
        .upper_bounds(&tv)
        .unwrap_or_default()
        .into_iter()
        .partition_map(|cty| match cty {
            CstrTy::Cstr(cstr) => Either::Left(cstr),
            CstrTy::Locl(lty) => Either::Right(remove_tyvar_from_upper_bound(
                &mut env.inf_env,
                tv.clone(),
                &lty,
            )),
        });

    // Don't solve if the upper bounds contains a `CstrTy::Cstr`
    if !cstrs.is_empty() {
        Ok(None)
    } else {
        // TODO[mjt] simplify intersections
        let ty = Ty::intersection(reason.clone(), tys);

        // remove from lower bound if we have a tyvar
        match ty.deref() {
            Ty_::Tvar(tv2) => {
                let tvs = HashSet::unit(*tv2);
                env.inf_env.remove_tyvar_lower_bound(tv, &tvs);
            }
            _ => {}
        }

        // bind to the supplied type
        Ok(bind_help(env, tv, ty))
    }
}

fn try_bind_equal_bound<R: Reason>(
    env: &mut NormalizeEnv<R>,
    tv: Tyvar,
    reason: &R,
    freshen: bool,
) -> Result<TryBindResult<R>> {
    let tvs = HashSet::unit(tv);
    env.inf_env.remove_tyvar_lower_bound(tv, &tvs);
    env.inf_env.remove_tyvar_upper_bound(tv, &tvs);
    let lbs: HashSet<CstrTy<R>> = env
        .inf_env
        .lower_bounds(&tv)
        .unwrap_or_default()
        .iter()
        .map(|cty| env.inf_env.resolve_cstr_ty(cty))
        .collect();
    let ubs: HashSet<CstrTy<R>> = env
        .inf_env
        .upper_bounds(&tv)
        .unwrap_or_default()
        .iter()
        .map(|cty| env.inf_env.resolve_cstr_ty(cty))
        .collect();
    let eqbs = lbs.clone().intersection(ubs.clone());
    // TODO[mjt] this might be a good time to think about how we want to handle
    // Tany / Terr
    match eqbs.iter().next().and_then(|cty| cty.ty_opt()) {
        // We have a common type in the upper and lower bounds, bind to it
        Some(ty) => Ok(TryBindResult::Bound(bind_help(env, tv, ty.clone()))),
        // No common type so try and find a shallow match
        _ if freshen => match find_shallow_match(env, &lbs, &ubs) {
            Some(ty) => {
                // create a new type with fresh tyvars
                let ty = freshen_inside(env, &ty)?;
                let ty_v = Ty::var(reason.clone(), tv);

                // TODO[hverr,mjt]: self should probably be a Subtyper
                // i.e. this function should be on impl Subtyper
                let mut subtyper = Subtyper::new(
                    &mut env.inf_env,
                    &mut env.tp_env,
                    env.decl_env.clone(),
                    None,
                );

                // enforce equality
                let ty_err_sub_ = subtyper.subtype(&ty_v, &ty);
                let ty_err_sup_ = subtyper.subtype(&ty, &ty_v);
                // bind
                Ok(TryBindResult::Bound(bind_help(env, tv, ty)))
            }
            // There is no shallow match in the upper and lower bounds so
            // our tyvar remains unbound
            _ => Ok(TryBindResult::Unbound),
        },
        // We had no common type and we're allowed to 'freshen' so
        // our tyvar remains unbound
        _ => Ok(TryBindResult::Unbound),
    }
}

/// Find the first element in the lower bounds which has a 'shallow match' in
/// the upper bounds
fn find_shallow_match<R: Reason>(
    env: &mut NormalizeEnv<R>,
    lower_bounds: &HashSet<CstrTy<R>>,
    upper_bounds: &HashSet<CstrTy<R>>,
) -> Option<Ty<R>> {
    lower_bounds
        .iter()
        .find(|lb| {
            let elb = env.inf_env.resolve_cstr_ty(lb);
            upper_bounds.iter().any(|ub| {
                let eub = env.inf_env.resolve_cstr_ty(ub);
                elb.shallow_match(&eub)
            })
        })?
        .ty_opt()
        .cloned()
    // .map(|ty| ty.clone())
}

#[inline]
fn bind_help<R: Reason>(env: &mut NormalizeEnv<R>, tv: Tyvar, ty: Ty<R>) -> Option<TypingError<R>> {
    let get_tparam_variance = |cn| {
        let class_res = env.decl_env.get_class(cn);
        match class_res {
            Ok(cls_opt) => {
                cls_opt.map(|cls| cls.get_tparams().iter().map(|tp| tp.variance).collect())
            }
            _ => None,
        }
    };
    env.inf_env
        .bind_update_variance(None, tv, ty, &get_tparam_variance)
}

// TODO[mjt] should we accumulate and return the propositions resulting from
// 'freshening' as well as the resulting type? This would avoid traversing the
// type again in subsequentg calls to subtyping
fn freshen_inside<R: Reason>(env: &mut NormalizeEnv<R>, ty: &Ty<R>) -> Result<Ty<R>> {
    match env.inf_env.resolve_ty(ty).deref() {
        Ty_::Tclass(cname, exact, tys) if !tys.is_empty() => {
            let ty_opt = env.decl_env.get_class(cname.id())?.map(|cls| {
                let params = cls
                    .get_tparams()
                    .iter()
                    .zip(tys.iter())
                    .map(|(tp, ty)| {
                        if matches!(tp.variance, Variance::Invariant) {
                            ty.clone()
                        } else {
                            env.inf_env
                                .fresh(V::Invariant, ty.reason().pos().clone(), None)
                        }
                    })
                    .collect();
                Ty::class(ty.reason().clone(), cname.clone(), *exact, params)
            });
            Ok(ty_opt.unwrap_or_else(|| ty.clone()))
        }
        Ty_::Tfun(ft) => freshen_inside(env, &ft.ret).and_then(|ret| {
            ft.params
                .iter()
                .map(|fp| freshen_inside(env, &fp.ty).map(|ty| FunParam { ty, ..fp.clone() }))
                .collect::<Result<_>>()
                .map(|params| {
                    Ty::fun(
                        ty.reason().clone(),
                        FunType {
                            ret,
                            params,
                            ..ft.clone()
                        },
                    )
                })
        }),
        Ty_::Toption(ty_inner) => {
            freshen_inside(env, ty_inner).map(|ty_inner| Ty::option(ty.reason().clone(), ty_inner))
        }
        Ty_::Tunion(tys) => tys
            .iter()
            .map(|ty| freshen_inside(env, ty))
            .collect::<Result<_>>()
            .map(|tys| Ty::union(ty.reason().clone(), tys)),
        Ty_::Tclass(_, _, _)
        | Ty_::Tnonnull
        | Ty_::Tgeneric(_, _)
        | Ty_::Tany
        | Ty_::Tvar(_)
        | Ty_::Tprim(_) => Ok(ty.clone()),
    }
}

pub fn remove_tyvar_from_upper_bound<R: Reason>(
    env: &mut InferenceEnv<R>,
    tv: Tyvar,
    ty: &Ty<R>,
) -> Ty<R> {
    remove_tyvar_from_upper_bound_help(env, tv, ty).unwrap_or_else(|| Ty::mixed(R::none()))
}

fn remove_tyvar_from_upper_bound_help<R: Reason>(
    env: &mut InferenceEnv<R>,
    tv: Tyvar,
    ty: &Ty<R>,
) -> Option<Ty<R>> {
    use Ty_::*;
    let ty = env.resolve_ty(ty);
    match ty.deref() {
        Tvar(tv2) if tv == *tv2 => None,
        Toption(ty2) => {
            let ety2 = remove_tyvar_from_upper_bound_help(env, tv, ty2)?;
            Some(Ty::option(ty.reason().clone(), ety2))
        }
        Tunion(tys) => {
            let tys_out = tys
                .iter()
                .map(|ty| remove_tyvar_from_upper_bound_help(env, tv, ty))
                .collect::<Option<_>>()?;
            Some(Ty::union(ty.reason().clone(), tys_out))
        }
        _ => Some(ty),
    }
}

pub fn remove_tyvar_from_lower_bound<R: Reason>(
    env: &mut InferenceEnv<R>,
    tv: Tyvar,
    ty: &Ty<R>,
) -> Ty<R> {
    remove_tyvar_from_lower_bound_help(env, tv, ty).unwrap_or_else(|| Ty::nothing(R::none()))
}

fn remove_tyvar_from_lower_bound_help<R: Reason>(
    env: &mut InferenceEnv<R>,
    tv: Tyvar,
    ty: &Ty<R>,
) -> Option<Ty<R>> {
    use Ty_::*;
    let ty = env.resolve_ty(ty);
    match ty.deref() {
        Tvar(tv2) if tv == *tv2 => None,
        Tunion(tys) => {
            let tys_out = tys
                .iter()
                .filter_map(|ty| remove_tyvar_from_lower_bound_help(env, tv, ty))
                .collect();
            Some(Ty::union(ty.reason().clone(), tys_out))
        }
        _ => Some(ty),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::subtyping::oracle::NoClasses;
    use crate::subtyping::visited_goals::VisitedGoals;
    use crate::subtyping::Subtyper;
    use crate::typaram_env::TyparamEnv;
    use pos::NPos;
    use pos::Pos;
    use ty::prop::PropF;
    use ty::reason::NReason;
    use utils::core::IdentGen;

    fn default_normalize_env<R: Reason>() -> NormalizeEnv<R> {
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
    fn test_remove_upper() {
        let mut env = InferenceEnv::default();
        let gen = IdentGen::new();

        let tv_0: Tyvar = gen.make().into();
        let ty_v0 = Ty::var(NReason::none(), tv_0);
        let tv_1: Tyvar = gen.make().into();
        let ty_v1 = Ty::var(NReason::none(), tv_1);
        let ty_no_v0 = Ty::option(
            NReason::none(),
            Ty::union(
                NReason::none(),
                vec![
                    Ty::int(NReason::none()),
                    Ty::union(NReason::none(), vec![Ty::float(NReason::none()), ty_v1]),
                ],
            ),
        );
        let ty_with_v0 = Ty::option(
            NReason::none(),
            Ty::union(
                NReason::none(),
                vec![
                    Ty::int(NReason::none()),
                    Ty::union(NReason::none(), vec![Ty::float(NReason::none()), ty_v0]),
                ],
            ),
        );

        assert!(remove_tyvar_from_upper_bound_help(&mut env, tv_0, &ty_no_v0).is_some());
        assert!(remove_tyvar_from_upper_bound_help(&mut env, tv_0, &ty_with_v0).is_none());
    }

    #[test]
    fn test_solve_equal_bound() {
        let mut env = default_normalize_env();
        let tv = env.inf_env.fresh_var(V::Covariant, NPos::none());
        let ty_v = Ty::var(NReason::none(), tv);
        let ty_int = Ty::int(NReason::none());
        let ty_arraykey = Ty::arraykey(NReason::none());

        let mut subtyper = Subtyper::new(
            &mut env.inf_env,
            &mut env.tp_env,
            env.decl_env.clone(),
            None,
        );

        // #1 <: int
        let r1 = subtyper.subtype(&ty_v, &ty_int);
        assert!(r1.unwrap().is_none());

        // #1 <: arraykey
        let r2 = subtyper.subtype(&ty_v, &ty_arraykey);
        assert!(r2.unwrap().is_none());

        // int <: #1
        let r3 = subtyper.subtype(&ty_int, &ty_v);
        assert!(r3.unwrap().is_none());

        let r4 = try_bind_equal_bound(&mut env, tv, &NReason::none(), false);
        assert!(matches!(r4.unwrap(), TryBindResult::Bound(_)));
        assert!(env.inf_env.binding(&tv).unwrap() == &ty_int);
    }

    #[test]
    fn test_solve_upper_bound() {
        let mut env = default_normalize_env();
        let tv = env.inf_env.fresh_var(V::Contravariant, NPos::none());
        assert!(matches!(env.inf_env.variance(&tv), V::Contravariant));
        let ty_v = Ty::var(NReason::none(), tv);
        let ty_arraykey = Ty::arraykey(NReason::none());

        let mut subtyper = Subtyper::new(
            &mut env.inf_env,
            &mut env.tp_env,
            env.decl_env.clone(),
            None,
        );

        // #1 <: arraykey
        let r1 = subtyper.subtype(&ty_v, &ty_arraykey);
        assert!(r1.unwrap().is_none());

        let r2 = try_bind_variance(&mut env, tv, &NReason::none());
        assert!(matches!(r2.unwrap(), TryBindResult::Bound(_)));
        assert!(env.inf_env.binding(&tv).unwrap() == &ty_arraykey);
    }

    #[test]
    fn test_solve_lower_bound() {
        let mut env = default_normalize_env();
        let tv = env.inf_env.fresh_var(V::Covariant, NPos::none());
        assert!(matches!(env.inf_env.variance(&tv), V::Covariant));
        let ty_v = Ty::var(NReason::none(), tv);
        let ty_arraykey = Ty::arraykey(NReason::none());
        let cty_arraykey = CstrTy::Locl(ty_arraykey.clone());

        let mut subtyper = Subtyper::new(
            &mut env.inf_env,
            &mut env.tp_env,
            env.decl_env.clone(),
            None,
        );

        // arraykey <: #1
        let r1 = subtyper.subtype(&ty_arraykey, &ty_v);
        assert!(r1.unwrap().is_none());

        assert!(
            env.inf_env
                .lower_bounds(&tv)
                .unwrap_or_default()
                .contains(&cty_arraykey)
        );

        let r2 = try_bind_variance(&mut env, tv, &NReason::none());
        assert!(matches!(r2.unwrap(), TryBindResult::Bound(_)));
        assert!(env.inf_env.binding(&tv).unwrap() == &ty_arraykey);
    }
}
