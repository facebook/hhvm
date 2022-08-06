// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod normalize;
pub mod oracle;
mod solve;
mod visited_goals;

use std::ops::Deref;
use std::rc::Rc;

use im::HashSet;
use oracle::Oracle;
use pos::Symbol;
use pos::TypeName;
pub use solve::force_solve;
pub use solve::solve;
use ty::local::Ty;
use ty::local::Ty_;
use ty::local::Tyvar;
use ty::local_error::TypingError;
use ty::prop::Cstr;
use ty::prop::Prop;
use ty::prop::PropF;
use ty::reason::Reason;

use crate::inference_env::InferenceEnv;
use crate::subtyping::normalize::NormalizeEnv;
use crate::typaram_env::TyparamEnv;
use crate::typing::typing_error::Result;

/// A structure that can handle and normalize subtyping constraints, and
/// propagate them to the inference environment.
pub struct Subtyper<'a, R: Reason> {
    inf_env: &'a mut InferenceEnv<R>,

    tp_env: &'a mut TyparamEnv<R>,
    decl_env: Rc<dyn Oracle<R>>,
    this_ty: Option<Ty<R>>,
}

impl<'a, R: Reason> Subtyper<'a, R> {
    pub fn new(
        inf_env: &'a mut InferenceEnv<R>,
        tp_env: &'a mut TyparamEnv<R>,
        decl_env: Rc<dyn Oracle<R>>,
        this_ty: Option<Ty<R>>,
    ) -> Self {
        Self {
            inf_env,
            tp_env,
            decl_env,
            this_ty,
        }
    }

    /// Create a new normalization environment, with
    /// copies of the inference environment and the
    /// type parameter inference.
    fn normalize_env(&self) -> NormalizeEnv<R> {
        NormalizeEnv::new(
            self.this_ty.clone(),
            self.inf_env.clone(),
            self.tp_env.clone(),
            self.decl_env.clone(),
        )
    }

    /// Commit the normalization environment.
    fn commit(&mut self, mut env: NormalizeEnv<R>) {
        std::mem::swap(self.inf_env, &mut env.inf_env);
        std::mem::swap(self.tp_env, &mut env.tp_env);
    }

    pub fn subtype(
        &mut self,
        ty_sub: &Ty<R>,
        ty_sup: &Ty<R>,
    ) -> Result<Option<Vec<TypingError<R>>>> {
        let mut env = self.normalize_env();
        let prop = env.simp_ty(ty_sub, ty_sup)?;
        match Self::props_to_env(&mut env, prop) {
            (props, ty_errs) if ty_errs.is_empty() => {
                std::mem::swap(self.inf_env, &mut env.inf_env);
                self.inf_env.add_subtype_prop(Prop::conjs(props));
                std::mem::swap(self.tp_env, &mut env.tp_env);
                Ok(None)
            }
            (_, ty_errs) => Ok(Some(ty_errs)),
        }
    }

    pub fn has_member(
        &mut self,
        _ty_sub: &Ty<R>,
        _class_id: TypeName,
        _ty_args: Vec<Ty<R>>,
        _member_name: Symbol,
        _member_ty: Ty<R>,
    ) -> Result<Option<TypingError<R>>> {
        unimplemented!("Inference for `has_member` propositions is not implemented")
    }

    pub fn force_solve(&mut self, tvar: Tyvar, r: &R) -> Result<Option<Vec<TypingError<R>>>> {
        if self.inf_env.is_unsolved(&tvar) {
            let mut normalize_env = self.normalize_env();
            solve::force_solve(&mut normalize_env, tvar, r, false)?;
            self.commit(normalize_env);
        }
        Ok(None)
    }

    pub fn force_solve_ty(&mut self, ty: &Ty<R>) -> Result<Option<Vec<TypingError<R>>>> {
        match &**ty {
            Ty_::Tvar(var) => self.force_solve(var.clone(), ty.reason()),
            Ty_::Tunion(..) => todo!(),
            _ => Ok(None),
        }
    }

    /// Traverse a proposition and add any bounds appearing in sub-propositions of
    /// the form `#1 <: t`, `t <: #1` or `#1 <: #2` to the inference environment
    /// then eagerly normalize any proposition implied by transitivity of `<:`
    fn props_to_env(
        env: &mut NormalizeEnv<R>,
        prop: Prop<R>,
    ) -> (Vec<Prop<R>>, Vec<TypingError<R>>) {
        let mut remain = vec![];
        let mut errs = vec![];
        let mut props = vec![prop];
        Self::props_to_env_help(env, &mut props, &mut remain, &mut errs);
        (remain, errs)
    }

    fn props_to_env_help(
        env: &mut NormalizeEnv<R>,
        props: &mut Vec<Prop<R>>,
        remain: &mut Vec<Prop<R>>,
        errs: &mut Vec<TypingError<R>>,
    ) {
        while let Some(prop) = props.pop() {
            Self::prop_to_env(env, prop, props, remain, errs);
        }
    }

    fn prop_to_env(
        env: &mut NormalizeEnv<R>,
        prop: Prop<R>,
        props: &mut Vec<Prop<R>>,
        remain: &mut Vec<Prop<R>>,
        errs: &mut Vec<TypingError<R>>,
    ) {
        match prop.deref() {
            PropF::Conj(conjs) => props.extend(conjs.clone()),
            PropF::Disj(err, disjs) => {
                // TODO[mjt] simplify the disjuncton
                Self::disjs_to_env(env, disjs, err, remain, errs)
            }
            PropF::Atom(cstr) => match cstr {
                Cstr::Subtype(cty_sub, cty_sup) => {
                    Self::subtype_to_env(env, cty_sub, cty_sup, props, remain);
                }
                _ => {}
            },
        }
    }

    /// Find the first proposition in the disjunction which has no subtyping
    /// errors. If there is no such proposition, add the supplied typing error
    fn disjs_to_env(
        env: &mut NormalizeEnv<R>,
        disjs: &Vec<Prop<R>>,
        err: &TypingError<R>,
        remain: &mut Vec<Prop<R>>,
        errs: &mut Vec<TypingError<R>>,
    ) {
        // take a copy of the env so we can restore it after trying each
        // disjunct
        let env_in = env.clone();
        let mut found = false;
        // try each disjunction until we have one which gives us no errors
        for disj in disjs {
            match Self::props_to_env(env, disj.clone()) {
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
    fn subtype_to_env(
        env: &mut NormalizeEnv<R>,
        ty_sub: &Ty<R>,
        ty_sup: &Ty<R>,
        props: &mut Vec<Prop<R>>,
        remain: &mut Vec<Prop<R>>,
    ) {
        let tv_sub_opt = ty_sub.tyvar_opt();
        let tv_sup_opt = ty_sup.tyvar_opt();

        if tv_sub_opt.is_none() && tv_sup_opt.is_none() {
            // TODO[mjt] clarify with Andrew what is going on here
            remain.push(Prop::subtype(ty_sub.clone(), ty_sup.clone()))
        } else {
            // At least one is a tyvar, add bounds and apply transitivity
            tv_sub_opt.into_iter().for_each(|tv_sub| {
                let upper_bounds_delta = Self::add_upper_bound(env, tv_sub, ty_sup);
                let lower_bounds = env.inf_env.lower_bounds(tv_sub).unwrap_or_default();
                let prop = Self::close_upper_bounds(env, &lower_bounds, &upper_bounds_delta);
                props.push(prop);
            });

            // tyvar in supertype position
            tv_sup_opt.into_iter().for_each(|tv_sup| {
                // at our type as a lower bound and return the difference
                let lower_bounds_delta = Self::add_lower_bound(env, tv_sup, ty_sub);
                // get all existing upper bounds
                let upper_bounds = env.inf_env.upper_bounds(tv_sup).unwrap_or_default();
                // for each new lower bound & each upper bound, simplify the
                // proposition lb <: ub obtain by transitivity of <:
                let prop = Self::close_lower_bounds(env, &lower_bounds_delta, &upper_bounds);
                props.push(prop);
            });
        }
    }

    fn add_lower_bound(
        env: &mut NormalizeEnv<R>,
        tv_sup: &Tyvar,
        ty_sub: &Ty<R>,
    ) -> HashSet<Ty<R>> {
        let lower_bounds_pre = env.inf_env.lower_bounds(tv_sup).unwrap_or_default();
        let get_tparam_variance = |nm| env.decl_env.get_variance(nm).unwrap();
        env.inf_env
            .add_lower_bound_update_variances(*tv_sup, ty_sub, &get_tparam_variance);
        let lower_bounds_post = env.inf_env.lower_bounds(tv_sup).unwrap_or_default();
        // TODO[mjt] We are using set difference since when we start
        // simplifying lower bounds (as a union) when adding a new bound,
        // we won't simply be able to the sole lower bound we take as an argument
        lower_bounds_post.difference(lower_bounds_pre)
    }

    fn add_upper_bound(
        env: &mut NormalizeEnv<R>,
        tv_sub: &Tyvar,
        ty_sup: &Ty<R>,
    ) -> HashSet<Ty<R>> {
        let upper_bounds_pre = env.inf_env.upper_bounds(tv_sub).unwrap_or_default();

        let get_tparam_variance = |nm| env.decl_env.get_variance(nm).unwrap();

        env.inf_env
            .add_upper_bound_update_variances(*tv_sub, ty_sup, &get_tparam_variance);

        let upper_bounds_post = env.inf_env.upper_bounds(tv_sub).unwrap();

        // TODO[mjt] as with the lower bound, we use set difference since at some
        // point we want to simplify the upper bound (as an intersection) meaning
        // this difference isn't just equal to the new bound
        upper_bounds_post.difference(upper_bounds_pre)
    }

    fn close_lower_bounds(
        env: &mut NormalizeEnv<R>,
        lower_bounds_delta: &HashSet<Ty<R>>,
        upper_bounds: &HashSet<Ty<R>>,
    ) -> Prop<R> {
        lower_bounds_delta
            .iter()
            .try_fold(Prop::valid(), |acc, ty_sub| {
                // TODO[mjt] type constants
                upper_bounds.iter().try_fold(acc, |acc, ty_sup| {
                    // TODO[mjt] capture the failure in return type
                    let next = env.simp_ty(ty_sub, ty_sup).unwrap();
                    if next.is_unsat() {
                        Err(next)
                    } else {
                        Ok(acc.conj(next))
                    }
                })
            })
            .unwrap_or_else(std::convert::identity)
    }

    fn close_upper_bounds(
        env: &mut NormalizeEnv<R>,
        lower_bounds: &HashSet<Ty<R>>,
        upper_bounds_delta: &HashSet<Ty<R>>,
    ) -> Prop<R> {
        upper_bounds_delta
            .iter()
            .try_fold(Prop::valid(), |acc, ty_sup| {
                lower_bounds.iter().try_fold(acc, |acc, ty_sub| {
                    let next = env.simp_ty(ty_sub, ty_sup).unwrap();
                    if next.is_unsat() {
                        Err(next)
                    } else {
                        Ok(acc.conj(next))
                    }
                })
            })
            .unwrap_or_else(std::convert::identity)
    }
}

#[cfg(test)]
mod tests {
    use ty::reason::NReason;
    use utils::core::IdentGen;

    use super::*;

    #[test]
    fn test_prim() {
        let mut inf_env = InferenceEnv::default();
        let mut tp_env = TyparamEnv::default();
        let oracle = Rc::new(oracle::NoClasses);

        let mut subtyper = Subtyper::new(&mut inf_env, &mut tp_env, oracle, None);

        // Subtypes of arraykey
        let ty_arraykey = Ty::arraykey(NReason::none());
        // int <: arraykey
        let ty_int = Ty::int(NReason::none());
        assert!(subtyper.subtype(&ty_int, &ty_arraykey,).unwrap().is_none());

        // arraykey </: int
        assert!(subtyper.subtype(&ty_arraykey, &ty_int,).unwrap().is_some());

        // string <: arraykey
        let ty_string = Ty::string(NReason::none());
        assert!(
            subtyper
                .subtype(&ty_string, &ty_arraykey,)
                .unwrap()
                .is_none()
        );

        // arraykey </: string
        assert!(
            subtyper
                .subtype(&ty_arraykey, &ty_string,)
                .unwrap()
                .is_some()
        );

        // Subtypes of num
        let ty_num = Ty::num(NReason::none());
        // int <: num
        assert!(subtyper.subtype(&ty_int, &ty_num,).unwrap().is_none());

        // float <: num
        let ty_float = Ty::float(NReason::none());
        assert!(subtyper.subtype(&ty_float, &ty_num,).unwrap().is_none());

        // num === (int | float)
        let ty_num_as_union = Ty::union(NReason::none(), vec![ty_int.clone(), ty_float.clone()]);
        assert!(
            subtyper
                .subtype(&ty_num, &ty_num_as_union)
                .unwrap()
                .is_none()
        );

        assert!(
            subtyper
                .subtype(&ty_num_as_union, &ty_num,)
                .unwrap()
                .is_none()
        );

        // string </: num
        assert!(subtyper.subtype(&ty_string, &ty_num,).unwrap().is_some());
    }

    #[test]
    fn test_add_upper_bound() {
        let decl_env = Rc::new(oracle::NoClasses);
        let mut inf_env = InferenceEnv::default();
        let mut tp_env = TyparamEnv::default();

        let gen = IdentGen::new();

        let subtyper = Subtyper::new(&mut inf_env, &mut tp_env, decl_env, None);
        let mut env = subtyper.normalize_env();

        let tv_sub: Tyvar = gen.make().into();
        let ty_sup = Ty::num(NReason::none());

        let delta = Subtyper::add_upper_bound(&mut env, &tv_sub, &ty_sup);
        assert!(delta.contains(&ty_sup));
    }

    #[test]
    fn test_add_lower_bound() {
        let decl_env = Rc::new(oracle::NoClasses);
        let mut inf_env = InferenceEnv::default();
        let mut tp_env = TyparamEnv::default();

        let gen = IdentGen::new();

        let subtyper = Subtyper::new(&mut inf_env, &mut tp_env, decl_env, None);
        let mut env = subtyper.normalize_env();

        let tv_sup: Tyvar = gen.make().into();
        let ty_sub = Ty::num(NReason::none());

        let delta = Subtyper::add_lower_bound(&mut env, &tv_sup, &ty_sub);
        assert!(delta.contains(&ty_sub));
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

        let mut subtyper = Subtyper::new(&mut inf_env, &mut tp_env, oracle, None);

        // Assert that #0 <: num
        let res1 = subtyper.subtype(&ty_v0, &ty_num);
        assert!(res1.unwrap().is_none());
        assert!(
            subtyper
                .inf_env
                .upper_bounds(&tv_0)
                .unwrap()
                .contains(&ty_num)
        );

        // Now assert that int <: #1;
        let res2 = subtyper.subtype(&ty_int, &ty_v1);
        assert!(res2.unwrap().is_none());
        assert!(
            subtyper
                .inf_env
                .lower_bounds(&tv_1)
                .unwrap()
                .contains(&ty_int)
        );

        // Now assert that  #1 <: #0; we should then have num as an upper bound
        // on #1 and int as a lower bound on #0
        let res3 = subtyper.subtype(&ty_v1, &ty_v0);
        assert!(res3.unwrap().is_none());
        assert!(inf_env.upper_bounds(&tv_1).unwrap().contains(&ty_num));
        assert!(inf_env.lower_bounds(&tv_0).unwrap().contains(&ty_int));
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

        let mut subtyper = Subtyper::new(&mut inf_env, &mut tp_env, oracle, None);

        // Assert that #0 <: num
        let res1 = subtyper.subtype(&ty_v0, &ty_num);
        assert!(res1.unwrap().is_none());
        assert!(
            subtyper
                .inf_env
                .upper_bounds(&tv_0)
                .unwrap()
                .contains(&ty_num)
        );

        // Now assert that string <: #1;
        let res2 = subtyper.subtype(&ty_string, &ty_v1);
        assert!(res2.unwrap().is_none());
        assert!(
            subtyper
                .inf_env
                .lower_bounds(&tv_1)
                .unwrap()
                .contains(&ty_string)
        );

        // Now assert that #1 <: #0; this should result in an error
        // since we now have string <: num by transitivity; we didn't end up
        // adding any new bound, the lower bound of #0 should still be empty,
        // and the upper bound of #1 should be empty
        let res3 = subtyper.subtype(&ty_v1, &ty_v0);
        assert!(res3.unwrap().is_some());
        assert!(inf_env.lower_bounds(&tv_0).unwrap().is_empty());
        assert!(inf_env.upper_bounds(&tv_1).unwrap().is_empty());
    }
}
