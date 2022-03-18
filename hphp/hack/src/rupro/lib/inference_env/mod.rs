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

#[derive(Debug, Clone, PartialEq, Eq)]
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

#[derive(Debug, Clone, PartialEq, Eq)]
enum ResolveStep<T> {
    Bound(T),
    Unbound(Tyvar),
    CircularRef(Tyvar),
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

    /// Get the type bound to [tv], if any
    pub fn binding(&self, tv: &Tyvar) -> Option<&Ty<R>> {
        self.tyvar_info.get(tv).and_then(|info| info.binding())
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

    pub fn resolve(&mut self, reason: R, tv: Tyvar) -> Result<Ty<R>, Tyvar> {
        let (aliases, step) = self.resolve_help(tv);
        use ResolveStep::*;
        let ty_res = match step {
            Unbound(tv) => Ok(Ty::var(reason, tv)),
            Bound(ty) => Ok(ty.clone()),
            CircularRef(tv) => Err(tv),
        };
        if let Ok(ref ty) = ty_res {
            for tv in aliases.into_iter() {
                self.add(None, tv, ty.clone())
            }
        }
        ty_res
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

    fn test_resolve_bound() {
        let mut env = InferenceEnv::default();
        let gen = IdentGen::new();
        let tv1: Tyvar = gen.make().into();
        let tv2: Tyvar = gen.make().into();
        let tv3: Tyvar = gen.make().into();
        let ty_v2 = Ty::var(NReason::none(), tv2);
        let ty_v3 = Ty::var(NReason::none(), tv2);
        let ty_int = Ty::int(NReason::none());

        // #1 -> #2 -> #3 -> int
        env.add(None, tv1, ty_v2);
        env.add(None, tv2, ty_v3);
        env.add(None, tv3, ty_int);

        let (aliases, res) = env.resolve_help(tv1);
        assert!(aliases.contains(&tv1));
        assert!(aliases.contains(&tv2));
        assert!(!aliases.contains(&tv3));
        assert!(matches!(res, ResolveStep::Bound(_)));
    }

    fn test_resolve_unbound() {
        let mut env = InferenceEnv::default();
        let gen = IdentGen::new();
        let tv1: Tyvar = gen.make().into();
        let tv2: Tyvar = gen.make().into();
        let tv3: Tyvar = gen.make().into();
        let ty_v2 = Ty::var(NReason::none(), tv2);
        let ty_v3 = Ty::var(NReason::none(), tv2);

        // #1 -> #2 -> #3
        env.add(None, tv1, ty_v2);
        env.add(None, tv2, ty_v3);

        let (aliases, res) = env.resolve_help(tv1);
        assert!(aliases.contains(&tv1));
        assert!(aliases.contains(&tv2));
        assert!(!aliases.contains(&tv3));
        assert!(matches!(res, ResolveStep::Unbound(_)));
    }

    fn test_resolve_ciricular_ref() {
        let mut env = InferenceEnv::default();
        let gen = IdentGen::new();
        let tv1: Tyvar = gen.make().into();
        let tv2: Tyvar = gen.make().into();
        let tv3: Tyvar = gen.make().into();
        let ty_v1 = Ty::var(NReason::none(), tv1);
        let ty_v2 = Ty::var(NReason::none(), tv2);
        let ty_v3 = Ty::var(NReason::none(), tv2);

        // #1 -> #2 -> #3 -> #1
        env.add(None, tv1, ty_v2);
        env.add(None, tv2, ty_v3);
        env.add(None, tv3, ty_v1);

        let (aliases, res) = env.resolve_help(tv1);
        assert!(aliases.contains(&tv1));
        assert!(aliases.contains(&tv2));
        assert!(!aliases.contains(&tv3));
        assert!(matches!(res, ResolveStep::CircularRef(_)));
    }
}
