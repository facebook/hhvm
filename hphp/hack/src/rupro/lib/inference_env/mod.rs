// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

mod tyvar_info;
mod tyvar_occurrences;

use crate::reason::Reason;
use crate::typing_defs::{Ty, Ty_, Tyvar};
use crate::typing_prop::Prop;
use crate::visitor::{Visitor, Walkable};
use im::{HashMap, HashSet};
use std::ops::Deref;
use tyvar_info::TyvarInfo;
use tyvar_occurrences::TyvarOccurrences;

pub struct InferenceEnv<R: Reason> {
    tyvar_info: HashMap<Tyvar, TyvarInfo<R>>,
    occurrences: TyvarOccurrences,
    subtype_prop: Prop<R>,
}

impl<R: Reason> Default for InferenceEnv<R> {
    fn default() -> Self {
        InferenceEnv {
            tyvar_info: HashMap::default(),
            occurrences: TyvarOccurrences::default(),
            subtype_prop: Prop::valid(),
        }
    }
}

impl<R: Reason> InferenceEnv<R> {
    fn is_solved(&self, tv: &Tyvar) -> bool {
        self.tyvar_info
            .get(tv)
            .map_or(false, |info| info.is_solved())
    }

    fn is_unsolved(&self, tv: &Tyvar) -> bool {
        !self.is_solved(tv)
    }

    fn contains_unsolved(&self, tv: &Tyvar) -> bool {
        self.occurrences.contains_any(tv)
    }

    fn collect_unsolved(&self, t: &Ty<R>) -> HashSet<Tyvar> {
        CollectUnsolved::new(self, t).acc
    }

    /// Bind the type [Ty<R>] to the type variable [tv]
    pub fn bind(&mut self, pos: Option<R::Pos>, tv: Tyvar, ty: Ty<R>) {
        self.tyvar_info.entry(tv).or_default().bind(pos, ty)
    }


    /// Bind the type [ty] to the type variable [tv] and update the occurrences
    /// of each type variable contained in [ty]
    pub fn add(&mut self, pos: Option<R::Pos>, tv: Tyvar, ty: Ty<R>) {
        let tvs = self.collect_unsolved(&ty);
        self.bind(pos, tv, ty);
        self.occurrences.unbind(tv);
        tvs.into_iter()
            .for_each(|tv_occ| self.occurrences.add_occurrence(tv_occ, tv));
    }
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

    fn visit_ty(&mut self, ty: &Ty<R>) {
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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::reason::NReason;
    use crate::typing_defs::{FunParam, FunType};
    use crate::utils::core::IdentGen;
    use pos::NPos;

    #[test]
    fn test_unsolved_empty_env() {
        // in the empty env, all ty vars are unsolved
        let env = InferenceEnv::default();

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
        let gen = IdentGen::new();
        let tv: Tyvar = gen.make().into();
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
                params: vec![fp],
                ret: t_void,
            },
        );
        assert!(!env.collect_unsolved(&t_fun_fp).is_empty());
    }

    #[test]
    fn test_add() {
        let mut env = InferenceEnv::default();

        let gen = IdentGen::new();
        let tv1: Tyvar = gen.make().into();

        let tv2 = gen.make().into();
        let ty_v2 = Ty::var(NReason::none(), tv2);
        let ty_int = Ty::int(NReason::none());
        let ty_union = Ty::union(NReason::none(), vec![ty_v2, ty_int]);

        // bind #1 => ( #2 | int )
        env.add(None, tv1, ty_union);
        assert!(env.occurrences.occurs_in(&tv2, &tv1));

        // rebind #1 => int, should unbind occurrence of #2 in #1
        let ty_int = Ty::int(NReason::none());
        env.add(None, tv1, ty_int);
        assert!(!env.occurrences.occurs_in(&tv2, &tv1));
    }
}
