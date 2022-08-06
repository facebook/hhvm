// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

mod tyvar_info;
mod tyvar_occurrences;

use std::ops::Deref;

use im::HashMap;
use im::HashSet;
use pos::Pos;
use pos::ToOxidized;
use pos::TypeName;
use ty::local::Ty;
use ty::local::Ty_;
use ty::local::Tyvar;
use ty::local::Variance;
use ty::local_error::Primary;
use ty::local_error::TypingError;
use ty::prop::Prop;
use ty::reason::Reason;
use ty::visitor::Visitor;
use ty::visitor::Walkable;
use tyvar_info::TyvarInfo;
use tyvar_occurrences::TyvarOccurrences;
use utils::core::IdentGen;

#[derive(Debug, Clone)]
pub struct InferenceEnv<R: Reason> {
    gen: IdentGen,
    tyvar_info: HashMap<Tyvar, TyvarInfo<R>>,
    tyvar_stack: Vec<(R::Pos, Vec<Tyvar>)>,
    occurrences: TyvarOccurrences,
    subtype_prop: Prop<R>,
}

impl<R: Reason> InferenceEnv<R> {
    fn add_unsolved_info(&mut self, tv: Tyvar, variance: Variance, pos: R::Pos) {
        self.tyvar_info.insert(tv, TyvarInfo::new(variance, pos));
    }

    fn push_tyvar(&mut self, tv: Tyvar) {
        self.tyvar_stack.last_mut().iter_mut().for_each(|(_, tvs)| {
            tvs.push(tv);
        })
    }

    pub fn fresh_var(&mut self, variance: Variance, pos: R::Pos) -> Tyvar {
        let tv = self.gen.make().into();
        self.add_unsolved_info(tv, variance, pos);
        self.push_tyvar(tv);
        tv
    }

    pub fn fresh(&mut self, variance: Variance, pos: R::Pos, reason_opt: Option<R>) -> Ty<R> {
        let reason = reason_opt.unwrap_or_else(|| R::tyvar(pos.clone()));
        let tv = self.fresh_var(variance, pos);
        Ty::var(reason, tv)
    }

    pub fn open_tyvars(&mut self, pos: R::Pos) {
        self.tyvar_stack.push((pos, vec![]))
    }

    pub fn get_all_tyvars(&self) -> Vec<Tyvar> {
        self.tyvar_info.keys().copied().collect()
    }

    pub fn close_tyvars(&mut self) {
        let popped = self.tyvar_stack.pop();
        if popped.is_none() {
            panic!("close_tyvars: empty stack")
        }
    }

    pub fn unsolved(&self) -> Vec<Tyvar> {
        self.tyvar_info
            .iter()
            .filter_map(|(tv, info)| if info.is_solved() { None } else { Some(*tv) })
            .collect()
    }

    pub fn is_solved(&self, tv: &Tyvar) -> bool {
        self.tyvar_info
            .get(tv)
            .map_or(false, |info| info.is_solved())
    }

    pub fn is_unsolved(&self, tv: &Tyvar) -> bool {
        !self.is_solved(tv)
    }

    fn is_error(&self, tv: &Tyvar) -> bool {
        self.tyvar_info
            .get(tv)
            .map_or(false, |info| info.is_error())
    }

    fn contains_unsolved(&self, tv: &Tyvar) -> bool {
        self.occurrences.contains_any(tv)
    }

    fn collect_unsolved(&self, t: &Ty<R>) -> HashSet<Tyvar> {
        CollectUnsolved::new(self, t).acc
    }

    /// Get the type bound to [tv], if any
    pub fn binding(&self, tv: &Tyvar) -> Option<&Ty<R>> {
        self.tyvar_info.get(tv).and_then(|info| info.binding())
    }

    fn resolve_step(
        &self,
        aliases: &HashSet<Tyvar>,
        tv: Tyvar,
    ) -> Result<ResolveStep<&Ty<R>>, (Tyvar, Tyvar)> {
        use ResolveStep::*;
        if let Some(ty) = self.binding(&tv) {
            match *ty.deref() {
                Ty_::Tvar(tv) if aliases.contains(&tv) => Ok(CircularRef(tv)),
                Ty_::Tvar(tv_out) => Err((tv, tv_out)),
                _ => Ok(Bound(ty)),
            }
        } else {
            Ok(Unbound(tv))
        }
    }

    /// Follow the bindings of a tyvar until we reach any of
    /// i) A dead-end i.e. an unbound tyvar
    /// ii) A ciricular ref i.e. we have already seen the tyvar earlier in the chain
    /// iii) A concrete type
    fn resolve_help(&self, tv: Tyvar) -> (HashSet<Tyvar>, ResolveStep<&Ty<R>>) {
        let mut aliases = HashSet::default();
        let mut next = self.resolve_step(&aliases, tv);
        while let Err((alias, tv)) = next {
            aliases.insert(alias);
            next = self.resolve_step(&aliases, tv);
        }
        // We should now have one of the above cases, if not we should panic
        if let Ok(step) = next {
            (aliases, step)
        } else {
            panic!("Encountered unfollowed alias during resolve")
        }
    }

    /// Try and resolve the tyvar `tv` to a concrete type. If the tyvar is
    /// involved in a cycle return `Err`
    pub fn resolve(&mut self, reason: &R, tv: Tyvar) -> Result<Ty<R>, Tyvar> {
        let (aliases, step) = self.resolve_help(tv);
        use ResolveStep::*;
        let ty_res = match step {
            Unbound(tv) => Ok(Ty::var(reason.clone(), tv)),
            Bound(ty) => Ok(ty.clone()),
            CircularRef(tv) => Err(tv),
        };
        if let Ok(ref ty) = ty_res {
            for tv in aliases.into_iter() {
                self.bind(None, tv, ty.clone());
            }
        }
        ty_res
    }

    pub fn resolve_ty(&mut self, ty: &Ty<R>) -> Ty<R> {
        if let Ty_::Tvar(tv) = ty.deref() {
            self.resolve(ty.reason(), *tv).unwrap()
        } else {
            ty.clone()
        }
    }

    pub fn is_mixed(&mut self, ty: &Ty<R>) -> bool {
        if let Ty_::Toption(ty_inner) = ty.deref() {
            let ety = self.resolve_ty(ty_inner);
            matches!(ety.deref(), Ty_::Tnonnull)
        } else {
            false
        }
    }

    pub fn upper_bounds(&self, tv: &Tyvar) -> Option<HashSet<Ty<R>>> {
        self.tyvar_info.get(tv).and_then(|info| info.upper_bounds())
    }

    pub fn lower_bounds(&self, tv: &Tyvar) -> Option<HashSet<Ty<R>>> {
        self.tyvar_info.get(tv).and_then(|info| info.lower_bounds())
    }

    pub fn add_upper_bound(&mut self, tv: Tyvar, bound: Ty<R>) {
        self.tyvar_info
            .entry(tv)
            .or_insert(TyvarInfo::default())
            .add_upper_bound(bound);
    }

    pub fn add_lower_bound(&mut self, tv: Tyvar, bound: Ty<R>) {
        self.tyvar_info
            .entry(tv)
            .or_insert(TyvarInfo::default())
            .add_lower_bound(bound);
    }

    pub fn remove_upper_bound(&mut self, tv: Tyvar, bound: &Ty<R>) {
        self.tyvar_info
            .entry(tv)
            .or_insert(TyvarInfo::default())
            .remove_upper_bound(bound);
    }

    pub fn remove_lower_bound(&mut self, tv: Tyvar, bound: &Ty<R>) {
        self.tyvar_info
            .entry(tv)
            .or_insert(TyvarInfo::default())
            .remove_lower_bound(bound);
    }

    /// Remove type variables `remove_tvs` from the upper bounds on `tv`, if it exists
    pub fn remove_tyvar_upper_bound(&mut self, tv: Tyvar, remove_tvs: &HashSet<Tyvar>) {
        self.tyvar_info.entry(tv).and_modify(|info| {
            info.remove_tyvar_upper_bound(remove_tvs);
        });
    }

    /// Remove type variables `remove_tvs` from the lower bounds on `tv`, if it exists
    pub fn remove_tyvar_lower_bound(&mut self, tv: Tyvar, remove_tvs: &HashSet<Tyvar>) {
        self.tyvar_info.entry(tv).and_modify(|info| {
            info.remove_tyvar_lower_bound(remove_tvs);
        });
    }

    #[inline]
    pub fn variance_opt(&self, tv: &Tyvar) -> Option<Variance> {
        self.tyvar_info.get(tv)?.variance()
    }
    pub fn variance(&self, tv: &Tyvar) -> Variance {
        self.variance_opt(tv).unwrap()
    }

    fn appears_covariantly(&self, tv: &Tyvar) -> bool {
        self.variance_opt(tv)
            .map_or(false, |info| info.appears_covariantly())
    }

    fn appears_contravariantly(&self, tv: &Tyvar) -> bool {
        self.variance_opt(tv)
            .map_or(false, |info| info.appears_contravariantly())
    }

    fn with_appearance(&mut self, tv: Tyvar, appearing: &Variance) {
        self.tyvar_info
            .entry(tv)
            .or_insert(TyvarInfo::default())
            .with_appearance(appearing);
    }

    /// Bind the type [ty] to the type variable [tv] and update the occurrences
    /// of each type variable contained in [ty]
    /// If the occurs check fails, mark as an error
    pub fn bind(&mut self, pos: Option<R::Pos>, tv: Tyvar, ty: Ty<R>) -> Option<TypingError<R>> {
        if ty.occurs(tv) {
            // occurs check failed and we have a unification cycle
            // bind to
            self.tyvar_info
                .entry(tv)
                .and_modify(|info| info.mark_error());
            Some(TypingError::Primary(Primary::OccursCheck))
        } else {
            let tvs = self.collect_unsolved(&ty);
            self.tyvar_info
                .entry(tv)
                .and_modify(|info| info.bind(pos.unwrap_or_else(R::Pos::none), ty));
            self.occurrences.unbind(tv);
            tvs.into_iter()
                .for_each(|tv_occ| self.occurrences.add_occurrence(tv_occ, tv));
            None
        }
    }

    pub fn bind_update_variance<F>(
        &mut self,
        pos: Option<R::Pos>,
        tv_bind: Tyvar,
        ty: Ty<R>,
        get_tparam_variance: &F,
    ) -> Option<TypingError<R>>
    where
        F: Fn(TypeName) -> Option<Vec<oxidized::ast_defs::Variance>>,
    {
        // get the variance of the tyvar wrt to its appearances
        let variance = self.variance(&tv_bind);

        // get the type variables appearing co- and contravariantly in the type
        // we are binding to
        let (covs, contravs) = ty.tyvars(get_tparam_variance);

        // actually bind it
        let err_opt = self.bind(pos, tv_bind, ty);

        // depending on what variance we have for `tv`, update the variance
        // of all type variable in `ty`
        match variance {
            Variance::Covariant => {
                // tv appears cvariantly so all covariant tyvars in
                // ty appear covariantly in that position
                covs.iter().for_each(|tv| {
                    self.with_appearance(*tv, &Variance::Covariant);
                });
                // tv appears cvariantly so all contravariant tyvars in
                // ty appear contravariantly in that position
                contravs
                    .iter()
                    .for_each(|tv| self.with_appearance(*tv, &Variance::Contravariant))
            }
            Variance::Contravariant => {
                // tv appears contravariantly so all covariant tyvars in
                // ty appear contravariantly in that position
                covs.iter().for_each(|tv| {
                    self.with_appearance(*tv, &Variance::Contravariant);
                });

                // tv appears contravariantly so all contravariant tyvars in
                // ty appear covariantly in that position
                contravs
                    .iter()
                    .for_each(|tv| self.with_appearance(*tv, &Variance::Covariant))
            }
            Variance::Invariant => {
                covs.iter().for_each(|tv| {
                    self.with_appearance(*tv, &Variance::Invariant);
                });
                contravs
                    .iter()
                    .for_each(|tv| self.with_appearance(*tv, &Variance::Invariant))
            }
            Variance::Bivariant => {}
        }
        err_opt
    }

    // TODO[mjt] add with intersection
    pub fn add_upper_bound_update_variances<F>(
        &mut self,
        tv: Tyvar,
        bound: &Ty<R>,
        get_tparam_variance: &F,
    ) where
        F: Fn(TypeName) -> Option<Vec<oxidized::ast_defs::Variance>>,
    {
        self.add_upper_bound(tv, bound.clone());
        if self.appears_contravariantly(&tv) {
            let ety = self.resolve_ty(bound);
            if !ety.is_var() {
                let (covs, contravs) = ety.tyvars(get_tparam_variance);
                covs.iter()
                    .for_each(|tv| self.with_appearance(*tv, &Variance::Contravariant));
                contravs
                    .iter()
                    .for_each(|tv| self.with_appearance(*tv, &Variance::Covariant))
            }
        }
    }

    // TODO[mjt] add with union
    pub fn add_lower_bound_update_variances<F>(
        &mut self,
        tv: Tyvar,
        bound: &Ty<R>,
        get_tparam_variance: &F,
    ) where
        F: Fn(TypeName) -> Option<Vec<oxidized::ast_defs::Variance>>,
    {
        self.add_lower_bound(tv, bound.clone());
        if self.appears_covariantly(&tv) {
            let ety = self.resolve_ty(bound);
            if !ety.is_var() {
                let (covs, contravs) = ety.tyvars(get_tparam_variance);
                covs.iter()
                    .for_each(|tv| self.with_appearance(*tv, &Variance::Covariant));
                contravs
                    .iter()
                    .for_each(|tv| self.with_appearance(*tv, &Variance::Contravariant))
            }
        }
    }

    pub fn add_subtype_prop(&mut self, prop: Prop<R>) {
        self.subtype_prop = self.subtype_prop.clone().conj(prop)
    }
}

impl<R: Reason> Default for InferenceEnv<R> {
    fn default() -> Self {
        InferenceEnv {
            gen: IdentGen::new(),
            tyvar_info: HashMap::default(),
            tyvar_stack: vec![],
            occurrences: TyvarOccurrences::default(),
            subtype_prop: Prop::valid(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
enum ResolveStep<T> {
    Bound(T),
    Unbound(Tyvar),
    CircularRef(Tyvar),
}

struct CollectUnsolved<'a, R: Reason> {
    pub acc: HashSet<Tyvar>,
    pub env: &'a InferenceEnv<R>,
}

impl<'a, R: Reason> CollectUnsolved<'a, R> {
    fn new(env: &'a InferenceEnv<R>, ty: &Ty<R>) -> CollectUnsolved<'a, R> {
        let mut tvs = CollectUnsolved {
            acc: HashSet::default(),
            env,
        };
        ty.accept(&mut tvs);
        tvs
    }
}

impl<'a, R: Reason> Visitor<R> for CollectUnsolved<'a, R> {
    fn object(&mut self) -> &mut dyn Visitor<R> {
        self
    }

    fn visit_local_ty(&mut self, ty: &Ty<R>) {
        match ty.deref() {
            Ty_::Tvar(tv) => {
                if self.env.is_unsolved(tv) || self.env.contains_unsolved(tv) {
                    self.acc.insert(*tv);
                }
            }
            _ => {}
        }
        ty.recurse(self.object())
    }
}

impl<'a, R: Reason> ToOxidized<'a> for InferenceEnv<R> {
    type Output = oxidized_by_ref::typing_inference_env::TypingInferenceEnv<'a>;

    fn to_oxidized(&self, bump: &'a bumpalo::Bump) -> Self::Output {
        let InferenceEnv {
            gen: _,
            tyvar_info,
            tyvar_stack,
            occurrences,
            subtype_prop,
        } = self;
        rupro_todo_assert!(tyvar_stack.is_empty(), AST);

        oxidized_by_ref::typing_inference_env::TypingInferenceEnv {
            tvenv: tyvar_info.to_oxidized(bump),
            tyvars_stack: &[],
            subtype_prop: bump.alloc(subtype_prop.to_oxidized(bump)),
            tyvar_occurrences: occurrences.to_oxidized(bump),
            allow_solve_globals: false,
        }
    }
}

#[cfg(test)]
mod tests {
    use oxidized::typing_defs_flags::FunTypeFlags;
    use pos::NPos;
    use ty::local::FunParam;
    use ty::local::FunType;
    use ty::reason::NReason;

    use super::*;

    #[test]
    fn test_unsolved_empty_env() {
        // in the empty env, all ty vars are unsolved
        let mut env = InferenceEnv::default();

        // and any never contains a tyvar..
        let t_any = Ty::any(NReason::none());
        assert!(env.collect_unsolved(&t_any).is_empty());

        // or void
        let t_void = Ty::void(NReason::none());
        assert!(env.collect_unsolved(&t_void).is_empty());

        // or a prim
        let t_int = Ty::int(NReason::none());
        assert!(env.collect_unsolved(&t_int).is_empty());

        // a var contains a tyvar
        let tv: Tyvar = env.fresh_var(Variance::Covariant, Pos::none());
        let t_var = Ty::var(NReason::none(), tv);
        assert!(!env.collect_unsolved(&t_var).is_empty());

        // a union containing a tyvar contains a tyvar
        let t_var = Ty::var(NReason::none(), tv);
        let t_union = Ty::union(NReason::none(), vec![t_var]);
        assert!(!env.collect_unsolved(&t_union).is_empty());

        // a function type with a tyvar return type contains a tyvar
        let t_var = Ty::var(NReason::none(), tv);
        let t_fun_ret = Ty::fun(
            NReason::none(),
            FunType {
                tparams: vec![].into_boxed_slice(),
                flags: FunTypeFlags::empty(),
                params: vec![],
                ret: t_var,
            },
        );
        assert!(!env.collect_unsolved(&t_fun_ret).is_empty());

        // a function with a tyvar param contains a tyvar
        let t_var = Ty::var(NReason::none(), tv);
        let t_void = Ty::void(NReason::none());
        let fp = FunParam {
            pos: NPos,
            name: None,
            ty: t_var,
        };
        let t_fun_fp = Ty::fun(
            NReason::none(),
            FunType {
                tparams: vec![].into_boxed_slice(),
                flags: FunTypeFlags::empty(),
                params: vec![fp],
                ret: t_void,
            },
        );
        assert!(!env.collect_unsolved(&t_fun_fp).is_empty());
    }

    #[test]
    fn test_bind() {
        let mut env = InferenceEnv::default();
        let tv1: Tyvar = env.fresh_var(Variance::Covariant, Pos::none());
        let tv2: Tyvar = env.fresh_var(Variance::Covariant, Pos::none());
        let ty_v2 = Ty::var(NReason::none(), tv2);
        let ty_int = Ty::int(NReason::none());
        let ty_union = Ty::union(NReason::none(), vec![ty_v2, ty_int]);

        // bind #1 => ( #2 | int )
        let err_opt1 = env.bind(None, tv1, ty_union);
        assert!(err_opt1.is_none());
        assert!(env.occurrences.occurs_in(&tv2, &tv1));

        // rebind #1 => int, should unbind occurrence of #2 in #1
        let ty_int = Ty::int(NReason::none());
        let err_opt2 = env.bind(None, tv1, ty_int);
        assert!(err_opt2.is_none());
        assert!(!env.occurrences.occurs_in(&tv2, &tv1));
    }
    #[test]
    fn test_with_appearance() {
        let mut env = InferenceEnv::<NReason>::default();
        let tv = env.fresh_var(Variance::Contravariant, Pos::none());
        assert!(matches!(env.variance(&tv), Variance::Contravariant));
        env.with_appearance(tv, &Variance::Covariant);
        assert!(matches!(env.variance(&tv), Variance::Invariant));
    }

    #[test]
    fn test_bind_update_variance() {
        let mut env = InferenceEnv::default();

        // initially we have
        // #1 contravariant
        // #2 covariant
        let tv1 = env.fresh_var(Variance::Contravariant, Pos::none());
        let tv2 = env.fresh_var(Variance::Covariant, Pos::none());
        assert!(matches!(env.variance(&tv1), Variance::Contravariant));
        assert!(matches!(env.variance(&tv2), Variance::Covariant));

        // now make a union in which #2 appears covariantly
        let ty_v2 = Ty::var(NReason::none(), tv2);
        let ty_int = Ty::int(NReason::none());
        let ty_union = Ty::union(NReason::none(), vec![ty_v2, ty_int]);
        let (covs, cons) = ty_union.tyvars(|_| None);
        assert!(covs.contains(&tv2));
        assert!(cons.is_empty());

        // bind #1 => ( #2 | int )
        // #2 must therefore appear in a contravriant position so, after updating
        // we should have + /\ - = o
        let get_tparam_variance = |_| None;
        let err_opt1 = env.bind_update_variance(None, tv1, ty_union, &get_tparam_variance);
        assert!(err_opt1.is_none());
        assert!(env.occurrences.occurs_in(&tv2, &tv1));

        assert!(matches!(env.variance(&tv2), Variance::Invariant));
    }

    #[test]
    fn test_bind_bad() {
        let mut env = InferenceEnv::default();
        let tv1 = env.fresh_var(Variance::Contravariant, Pos::none());
        let ty_v1 = Ty::var(NReason::none(), tv1);
        let ty_int = Ty::int(NReason::none());
        let ty_union = Ty::union(NReason::none(), vec![ty_v1, ty_int]);

        // bind #1 => ( #1 | int )
        let err_opt1 = env.bind(None, tv1, ty_union);
        assert!(err_opt1.is_some());
        assert!(!env.occurrences.occurs_in(&tv1, &tv1));
        assert!(env.is_error(&tv1));
    }

    fn test_resolve_bound() {
        let mut env = InferenceEnv::default();
        let tv1 = env.fresh_var(Variance::Contravariant, Pos::none());
        let tv2 = env.fresh_var(Variance::Contravariant, Pos::none());
        let tv3 = env.fresh_var(Variance::Contravariant, Pos::none());
        let ty_v2 = Ty::var(NReason::none(), tv2);
        let ty_v3 = Ty::var(NReason::none(), tv2);
        let ty_int = Ty::int(NReason::none());

        // #1 -> #2 -> #3 -> int
        env.bind(None, tv1, ty_v2);
        env.bind(None, tv2, ty_v3);
        env.bind(None, tv3, ty_int);

        let (aliases, res) = env.resolve_help(tv1);
        assert!(aliases.contains(&tv1));
        assert!(aliases.contains(&tv2));
        assert!(!aliases.contains(&tv3));
        assert!(matches!(res, ResolveStep::Bound(_)));
    }

    fn test_resolve_unbound() {
        let mut env = InferenceEnv::default();
        let tv1 = env.fresh_var(Variance::Contravariant, Pos::none());
        let tv2 = env.fresh_var(Variance::Contravariant, Pos::none());
        let tv3 = env.fresh_var(Variance::Contravariant, Pos::none());
        let ty_v2 = Ty::var(NReason::none(), tv2);
        let ty_v3 = Ty::var(NReason::none(), tv2);

        // #1 -> #2 -> #3
        env.bind(None, tv1, ty_v2);
        env.bind(None, tv2, ty_v3);

        let (aliases, res) = env.resolve_help(tv1);
        assert!(aliases.contains(&tv1));
        assert!(aliases.contains(&tv2));
        assert!(!aliases.contains(&tv3));
        assert!(matches!(res, ResolveStep::Unbound(_)));
    }

    fn test_resolve_ciricular_ref() {
        let mut env = InferenceEnv::default();
        let tv1 = env.fresh_var(Variance::Contravariant, Pos::none());
        let tv2 = env.fresh_var(Variance::Contravariant, Pos::none());
        let tv3 = env.fresh_var(Variance::Contravariant, Pos::none());
        let ty_v1 = Ty::var(NReason::none(), tv1);
        let ty_v2 = Ty::var(NReason::none(), tv2);
        let ty_v3 = Ty::var(NReason::none(), tv2);

        // #1 -> #2 -> #3 -> #1
        env.bind(None, tv1, ty_v2);
        env.bind(None, tv2, ty_v3);
        env.bind(None, tv3, ty_v1);

        let (aliases, res) = env.resolve_help(tv1);
        assert!(aliases.contains(&tv1));
        assert!(aliases.contains(&tv2));
        assert!(!aliases.contains(&tv3));
        assert!(matches!(res, ResolveStep::CircularRef(_)));
    }
}
