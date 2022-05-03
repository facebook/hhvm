// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod config;
mod env;
mod normalize;
mod oracle;
mod solve;
mod visited_goals;

use crate::{inference_env::InferenceEnv, typaram_env::TyparamEnv, typing::typing_error::Result};
use config::Config;
use env::Env;
use im::HashSet;
use oracle::Oracle;
use pos::{Symbol, TypeName};
use std::{ops::Deref, rc::Rc};
use ty::{
    local::{Ty, Tyvar},
    local_error::TypingError,
    prop::{CstrTy, Prop, PropF},
    reason::Reason,
};

pub fn subtype<R>(
    inf_env: &mut InferenceEnv<R>,
    tp_env: &mut TyparamEnv<R>,
    decl_env: Rc<dyn Oracle<R>>,
    this_ty: Option<Ty<R>>,
    ty_sub: &Ty<R>,
    ty_sup: &Ty<R>,
) -> Result<Option<Vec<TypingError<R>>>>
where
    R: Reason,
{
    let mut env = Env {
        this_ty,
        inf_env: inf_env.clone(),
        tp_env: tp_env.clone(),
        decl_env,
        ..Env::default()
    };
    let cfg = Config::default();
    let prop = normalize::simp_ty(&cfg, &mut env, ty_sub, ty_sup)?;
    match props_to_env(&cfg, &mut env, prop) {
        (props, ty_errs) if ty_errs.is_empty() => {
            std::mem::swap(inf_env, &mut env.inf_env);
            inf_env.add_subtype_prop(Prop::conjs(props));
            std::mem::swap(tp_env, &mut env.tp_env);
            Ok(None)
        }
        (_, ty_errs) => Ok(Some(ty_errs)),
    }
}

pub fn has_member<R>(
    _inf_env: &mut InferenceEnv<R>,
    _tp_env: &mut TyparamEnv<R>,
    _decl_env: Box<dyn Oracle<R>>,
    _this_ty: Option<Ty<R>>,
    _ty_sub: &Ty<R>,
    _class_id: TypeName,
    _ty_args: Vec<Ty<R>>,
    _member_name: Symbol,
    _member_ty: Ty<R>,
) -> Result<Option<TypingError<R>>>
where
    R: Reason,
{
    unimplemented!("Inference for `has_member` propositions is not implemented")
}

/// Traverse a proposition and add any bounds appearing in sub-propositions of
/// the form `#1 <: t`, `t <: #1` or `#1 <: #2` to the inference environment
/// then eagerly normalize any proposition implied by transitivity of `<:`
fn props_to_env<R>(
    cfg: &Config,
    env: &mut Env<R>,
    prop: Prop<R>,
) -> (Vec<Prop<R>>, Vec<TypingError<R>>)
where
    R: Reason,
{
    let mut remain = vec![];
    let mut errs = vec![];
    let mut props = vec![prop];
    props_to_env_help(cfg, env, &mut props, &mut remain, &mut errs);
    (remain, errs)
}

fn props_to_env_help<R>(
    cfg: &Config,
    env: &mut Env<R>,
    props: &mut Vec<Prop<R>>,
    remain: &mut Vec<Prop<R>>,
    errs: &mut Vec<TypingError<R>>,
) where
    R: Reason,
{
    while let Some(prop) = props.pop() {
        prop_to_env(cfg, env, prop, props, remain, errs);
    }
}

fn prop_to_env<R>(
    cfg: &Config,
    env: &mut Env<R>,
    prop: Prop<R>,
    props: &mut Vec<Prop<R>>,
    remain: &mut Vec<Prop<R>>,
    errs: &mut Vec<TypingError<R>>,
) where
    R: Reason,
{
    match prop.deref() {
        PropF::Conj(conjs) => props.extend(conjs.clone()),
        PropF::Disj(err, disjs) => {
            // TODO[mjt] simplify the disjuncton
            disjs_to_env(cfg, env, disjs, err, remain, errs)
        }
        PropF::Subtype(cty_sub, cty_sup) => {
            subtype_to_env(cfg, env, cty_sub, cty_sup, props, remain);
        }
    }
}

/// Find the first proposition in the disjunction which has no subtyping
/// errors. If there is no such proposition, add the supplied typing error
fn disjs_to_env<R>(
    cfg: &Config,
    env: &mut Env<R>,
    disjs: &Vec<Prop<R>>,
    err: &TypingError<R>,
    remain: &mut Vec<Prop<R>>,
    errs: &mut Vec<TypingError<R>>,
) where
    R: Reason,
{
    // take a copy of the env so we can restore it after trying each
    // disjunct
    let env_in = env.clone();
    let mut found = false;
    // try each disjunction until we have one which gives us no errors
    for disj in disjs {
        match props_to_env(cfg, env, disj.clone()) {
            (disj_remain, disj_errs) if disj_errs.is_empty() => {
                remain.extend(disj_remain);
                found = true;
                break;
            }
            _ => {
                *env = env_in.clone();
            }
        }
    }
    // If there was no successful disjunct, add the associated error
    if !found {
        errs.push(err.clone())
    }
}

/// Add any lower or upper bound for a type variable appearing in a `_ <: _`
/// proposition and eagerly process any propositions implied by the transitivity
/// of subtyping
fn subtype_to_env<R>(
    cfg: &Config,
    env: &mut Env<R>,
    cty_sub: &CstrTy<R>,
    cty_sup: &CstrTy<R>,
    props: &mut Vec<Prop<R>>,
    remain: &mut Vec<Prop<R>>,
) where
    R: Reason,
{
    let tv_sub_opt = cty_sub.tyvar_opt();
    let tv_sup_opt = cty_sup.tyvar_opt();

    if tv_sub_opt.is_none() && tv_sup_opt.is_none() {
        // TODO[mjt] clarify with Andrew what is going on here
        remain.push(Prop::subtype(cty_sub.clone(), cty_sup.clone()))
    } else {
        // At least one is a tyvar, add bounds and apply transitivity
        tv_sub_opt.into_iter().for_each(|tv_sub| {
            let upper_bounds_delta = add_upper_bound(env, tv_sub, cty_sup);
            let lower_bounds = env.inf_env.lower_bounds(tv_sub).unwrap_or_default();
            let prop = close_upper_bounds(cfg, env, &lower_bounds, &upper_bounds_delta);
            props.push(prop);
        });

        // tyvar in supertype position
        tv_sup_opt.into_iter().for_each(|tv_sup| {
            // at our type as a lower bound and return the difference
            let lower_bounds_delta = add_lower_bound(env, tv_sup, cty_sub);
            // get all existing upper bounds
            let upper_bounds = env.inf_env.upper_bounds(tv_sup).unwrap_or_default();
            // for each new lower bound & each upper bound, simplify the
            // proposition lb <: ub obtain by transitivity of <:
            let prop = close_lower_bounds(cfg, env, &lower_bounds_delta, &upper_bounds);
            props.push(prop);
        });
    }
}

fn add_lower_bound<R>(env: &mut Env<R>, tv_sup: &Tyvar, cty_sub: &CstrTy<R>) -> HashSet<CstrTy<R>>
where
    R: Reason,
{
    let lower_bounds_pre = env.inf_env.lower_bounds(tv_sup).unwrap_or_default();
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
        .add_lower_bound_update_variances(*tv_sup, cty_sub, &get_tparam_variance);
    let lower_bounds_post = env.inf_env.lower_bounds(tv_sup).unwrap_or_default();
    // TODO[mjt] We are using set difference since when we start
    // simplifying lower bounds (as a union) when adding a new bound,
    // we won't simply be able to the sole lower bound we take as an argument
    lower_bounds_post.difference(lower_bounds_pre)
}

fn add_upper_bound<R>(env: &mut Env<R>, tv_sub: &Tyvar, cty_sup: &CstrTy<R>) -> HashSet<CstrTy<R>>
where
    R: Reason,
{
    let upper_bounds_pre = env.inf_env.upper_bounds(tv_sub).unwrap_or_default();

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
        .add_upper_bound_update_variances(*tv_sub, cty_sup, &get_tparam_variance);

    let upper_bounds_post = env.inf_env.upper_bounds(tv_sub).unwrap();

    // TODO[mjt] as with the lower bound, we use set difference since at some
    // point we want to simplify the upper bound (as an intersection) meaning
    // this difference isn't just equal to the new bound
    upper_bounds_post.difference(upper_bounds_pre)
}

fn close_lower_bounds<R>(
    cfg: &Config,
    env: &mut Env<R>,
    lower_bounds_delta: &HashSet<CstrTy<R>>,
    upper_bounds: &HashSet<CstrTy<R>>,
) -> Prop<R>
where
    R: Reason,
{
    lower_bounds_delta
        .iter()
        .try_fold(Prop::valid(), |acc, cty_sub| {
            // TODO[mjt] type constants
            upper_bounds.iter().try_fold(acc, |acc, cty_sup| {
                // TODO[mjt] capture the failure in return type
                match (cty_sub, cty_sup) {
                    (CstrTy::Locl(ty_sub), CstrTy::Locl(ty_sup)) => {
                        let next = normalize::simp_ty(cfg, env, ty_sub, ty_sup).unwrap();
                        if next.is_unsat() {
                            Err(next)
                        } else {
                            Ok(acc.conj(next))
                        }
                    }
                    _ => unimplemented!("Inference is not implemented for constraint types"),
                }
            })
        })
        .unwrap_or_else(std::convert::identity)
}

fn close_upper_bounds<R>(
    cfg: &Config,
    env: &mut Env<R>,
    lower_bounds: &HashSet<CstrTy<R>>,
    upper_bounds_delta: &HashSet<CstrTy<R>>,
) -> Prop<R>
where
    R: Reason,
{
    upper_bounds_delta
        .iter()
        .try_fold(Prop::valid(), |acc, cty_sup| {
            lower_bounds
                .iter()
                .try_fold(acc, |acc, cty_sub| match (cty_sub, cty_sup) {
                    (CstrTy::Locl(ty_sub), CstrTy::Locl(ty_sup)) => {
                        let next = normalize::simp_ty(cfg, env, ty_sub, ty_sup).unwrap();
                        if next.is_unsat() {
                            Err(next)
                        } else {
                            Ok(acc.conj(next))
                        }
                    }
                    _ => unimplemented!("Inference is not implemented for constraint types"),
                })
        })
        .unwrap_or_else(std::convert::identity)
}

#[cfg(test)]
mod tests {
    use super::*;
    use ty::reason::NReason;
    use utils::core::IdentGen;

    #[test]
    fn test_prim() {
        let mut inf_env = InferenceEnv::default();
        let mut tp_env = TyparamEnv::default();
        let oracle = Rc::new(oracle::NoClasses);

        // Subtypes of arraykey
        let ty_arraykey = Ty::arraykey(NReason::none());
        // int <: arraykey
        let ty_int = Ty::int(NReason::none());
        assert!(
            subtype(
                &mut inf_env,
                &mut tp_env,
                oracle.clone(),
                None,
                &ty_int,
                &ty_arraykey,
            )
            .unwrap()
            .is_none()
        );

        // arraykey </: int
        assert!(
            subtype(
                &mut inf_env,
                &mut tp_env,
                oracle.clone(),
                None,
                &ty_arraykey,
                &ty_int,
            )
            .unwrap()
            .is_some()
        );

        // string <: arraykey
        let ty_string = Ty::string(NReason::none());
        assert!(
            subtype(
                &mut inf_env,
                &mut tp_env,
                oracle.clone(),
                None,
                &ty_string,
                &ty_arraykey,
            )
            .unwrap()
            .is_none()
        );

        // arraykey </: string
        assert!(
            subtype(
                &mut inf_env,
                &mut tp_env,
                oracle.clone(),
                None,
                &ty_arraykey,
                &ty_string,
            )
            .unwrap()
            .is_some()
        );

        // Subtypes of num
        let ty_num = Ty::num(NReason::none());
        // int <: num
        assert!(
            subtype(
                &mut inf_env,
                &mut tp_env,
                oracle.clone(),
                None,
                &ty_int,
                &ty_num,
            )
            .unwrap()
            .is_none()
        );

        // float <: num
        let ty_float = Ty::float(NReason::none());
        assert!(
            subtype(
                &mut inf_env,
                &mut tp_env,
                oracle.clone(),
                None,
                &ty_float,
                &ty_num,
            )
            .unwrap()
            .is_none()
        );

        // num === (int | float)
        let ty_num_as_union = Ty::union(NReason::none(), vec![ty_int.clone(), ty_float.clone()]);
        assert!(
            subtype(
                &mut inf_env,
                &mut tp_env,
                oracle.clone(),
                None,
                &ty_num,
                &ty_num_as_union
            )
            .unwrap()
            .is_none()
        );

        assert!(
            subtype(
                &mut inf_env,
                &mut tp_env,
                oracle.clone(),
                None,
                &ty_num_as_union,
                &ty_num,
            )
            .unwrap()
            .is_none()
        );

        // string </: num
        assert!(
            subtype(&mut inf_env, &mut tp_env, oracle, None, &ty_string, &ty_num,)
                .unwrap()
                .is_some()
        );
    }

    #[test]
    fn test_add_upper_bound() {
        let decl_env = Rc::new(oracle::NoClasses);
        let mut env = Env {
            this_ty: None,
            inf_env: InferenceEnv::default(),
            tp_env: TyparamEnv::default(),
            decl_env,
            ..Env::default()
        };

        let gen = IdentGen::new();

        let tv_sub: Tyvar = gen.make().into();
        let ty_sup = Ty::num(NReason::none());
        let cty_sup = CstrTy::Locl(ty_sup);

        let delta = add_upper_bound(&mut env, &tv_sub, &cty_sup);
        assert!(delta.contains(&cty_sup));
    }

    #[test]
    fn test_add_lower_bound() {
        let decl_env = Rc::new(oracle::NoClasses);
        let mut env = Env {
            this_ty: None,
            inf_env: InferenceEnv::default(),
            tp_env: TyparamEnv::default(),
            decl_env,
            ..Env::default()
        };

        let gen = IdentGen::new();

        let tv_sup: Tyvar = gen.make().into();
        let ty_sub = Ty::num(NReason::none());
        let cty_sub = CstrTy::Locl(ty_sub);

        let delta = add_lower_bound(&mut env, &tv_sup, &cty_sub);
        assert!(delta.contains(&cty_sub));
    }

    #[test]
    fn test_tc_good() {
        let mut inf_env = InferenceEnv::default();
        let mut tp_env = TyparamEnv::default();
        let oracle = Rc::new(oracle::NoClasses);

        let gen = IdentGen::new();

        let tv_0: Tyvar = gen.make().into();
        let ty_v0 = Ty::var(NReason::none(), tv_0);
        let tv_1: Tyvar = gen.make().into();
        let ty_v1 = Ty::var(NReason::none(), tv_1);
        let ty_num = Ty::num(NReason::none());
        let ty_int = Ty::int(NReason::none());
        let cty_num = CstrTy::Locl(ty_num.clone());
        let cty_int = CstrTy::Locl(ty_int.clone());

        // Assert that #0 <: num
        let res1 = subtype(
            &mut inf_env,
            &mut tp_env,
            oracle.clone(),
            None,
            &ty_v0,
            &ty_num,
        );
        assert!(res1.unwrap().is_none());
        assert!(inf_env.upper_bounds(&tv_0).unwrap().contains(&cty_num));

        // Now assert that int <: #1;
        let res2 = subtype(
            &mut inf_env,
            &mut tp_env,
            oracle.clone(),
            None,
            &ty_int,
            &ty_v1,
        );
        assert!(res2.unwrap().is_none());
        assert!(inf_env.lower_bounds(&tv_1).unwrap().contains(&cty_int));

        // Now assert that  #1 <: #0; we should then have num as an upper bound
        // on #1 and int as a lower bound on #0
        let res3 = subtype(&mut inf_env, &mut tp_env, oracle, None, &ty_v1, &ty_v0);
        assert!(res3.unwrap().is_none());
        assert!(inf_env.upper_bounds(&tv_1).unwrap().contains(&cty_num));
        assert!(inf_env.lower_bounds(&tv_0).unwrap().contains(&cty_int));
    }

    #[test]
    fn test_tc_bad() {
        let mut inf_env = InferenceEnv::default();
        let mut tp_env = TyparamEnv::default();
        let oracle = Rc::new(oracle::NoClasses);

        let gen = IdentGen::new();

        let tv_0: Tyvar = gen.make().into();
        let ty_v0 = Ty::var(NReason::none(), tv_0);
        let tv_1: Tyvar = gen.make().into();
        let ty_v1 = Ty::var(NReason::none(), tv_1);
        let ty_num = Ty::num(NReason::none());
        let ty_string = Ty::string(NReason::none());
        let cty_num = CstrTy::Locl(ty_num.clone());
        let cty_string = CstrTy::Locl(ty_string.clone());

        // Assert that #0 <: num
        let res1 = subtype(
            &mut inf_env,
            &mut tp_env,
            oracle.clone(),
            None,
            &ty_v0,
            &ty_num,
        );
        assert!(res1.unwrap().is_none());
        assert!(inf_env.upper_bounds(&tv_0).unwrap().contains(&cty_num));

        // Now assert that string <: #1;
        let res2 = subtype(
            &mut inf_env,
            &mut tp_env,
            oracle.clone(),
            None,
            &ty_string,
            &ty_v1,
        );
        assert!(res2.unwrap().is_none());
        assert!(inf_env.lower_bounds(&tv_1).unwrap().contains(&cty_string));

        // Now assert that #1 <: #0; this should result in an error
        // since we now have string <: num by transitivity; we didn't end up
        // adding any new bound, the lower bound of #0 should still be empty,
        // and the upper bound of #1 should be empty
        let res3 = subtype(&mut inf_env, &mut tp_env, oracle, None, &ty_v1, &ty_v0);
        assert!(res3.unwrap().is_some());
        assert!(inf_env.lower_bounds(&tv_0).unwrap().is_empty());
        assert!(inf_env.upper_bounds(&tv_1).unwrap().is_empty());
    }
}
