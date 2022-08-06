// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::Deref;

use im::HashMap;
use im::HashSet;
use pos::Pos;
use pos::TypeName;
use ty::local::Kind;
use ty::local::Ty;
use ty::local::Ty_;
use ty::reason::Reason;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct TyparamEnv<R: Reason> {
    typarams: HashMap<TypeName, (R::Pos, Kind<R>)>,
    is_consistent: bool,
}

impl<R: Reason> TyparamEnv<R> {
    pub fn contains(&self, tp_name: &TypeName) -> bool {
        self.typarams.contains_key(tp_name)
    }

    pub fn is_consistent(&self) -> bool {
        self.is_consistent
    }

    pub fn get(&self, tp_name: &TypeName) -> Option<&(R::Pos, Kind<R>)> {
        self.typarams.get(tp_name)
    }

    pub fn add(&mut self, tp_name: TypeName, def_pos: R::Pos, kind: Kind<R>) {
        self.typarams.insert(tp_name, (def_pos, kind));
    }

    pub fn append(&mut self, other: Self) {
        self.typarams = std::mem::take(&mut self.typarams).union(other.typarams);
        self.is_consistent = self.is_consistent && other.is_consistent;
    }

    pub fn mark_inconsistent(&mut self) {
        self.is_consistent = true;
    }

    fn is_generic_param(ty: &Ty<R>, tp_name: &TypeName, elide_nullable: bool) -> bool {
        if let Some(ty_name) = ty.generic_name() {
            ty_name == tp_name
        } else if elide_nullable {
            if let Ty_::Toption(ty_inner) = ty.deref() {
                Self::is_generic_param(ty_inner, tp_name, elide_nullable)
            } else {
                false
            }
        } else {
            false
        }
    }

    pub fn upper_bounds(&self, tp_name: &TypeName) -> Option<&HashSet<Ty<R>>> {
        self.typarams
            .get(tp_name)
            .map(|(_, kind)| kind.upper_bounds())
    }

    pub fn lower_bounds(&self, tp_name: &TypeName) -> Option<&HashSet<Ty<R>>> {
        self.typarams
            .get(tp_name)
            .map(|(_, kind)| kind.lower_bounds())
    }

    fn remove_lower_bound(&mut self, tp_name: &TypeName, ty: &Ty<R>) {
        self.typarams.entry(*tp_name).and_modify(|(_, k)| {
            k.remove_lower_bound(ty);
        });
    }

    fn remove_upper_bound(&mut self, tp_name: &TypeName, ty: &Ty<R>) {
        self.typarams.entry(*tp_name).and_modify(|(_, k)| {
            k.remove_upper_bound(ty);
        });
    }

    fn add_upper_bound_help(&mut self, tp_name: &TypeName, ty: Ty<R>) {
        if !Self::is_generic_param(&ty, tp_name, true) {
            self.typarams
                .entry(*tp_name)
                .or_insert((R::Pos::none(), Kind::default()))
                .1
                .add_upper_bound(ty);
        }
    }

    fn add_lower_bound_help(&mut self, tp_name: &TypeName, ty: Ty<R>) {
        if !Self::is_generic_param(&ty, tp_name, true) {
            self.typarams
                .entry(*tp_name)
                .or_insert((R::Pos::none(), Kind::default()))
                .1
                .add_lower_bound(ty);
        }
    }

    /// Add a single new upper bound [ty] to generic parameter [name].
    pub fn add_upper_bound(&mut self, tp_name: TypeName, ty: Ty<R>) {
        // If we're adding U as the upper bound of T, also add T as the lower
        // bound of U
        if let Ty_::Tgeneric(formal_super, ps) = ty.deref() {
            if ps.is_empty() {
                self.add_lower_bound_help(
                    formal_super,
                    Ty::generic(ty.reason().clone(), tp_name, vec![]),
                );
            }
        }

        self.add_upper_bound_help(&tp_name, ty);
    }

    /// If the optional [intersect] operation is supplied, then use this to avoid
    /// adding redundant bounds by merging the type with existing bounds. This makes
    /// sense because a conjunction of upper bounds
    ///  (T <: t1) /\ ... /\ (T <: tn)
    /// is equivalent to a single upper bound
    ///  T <: (t1 & ... & tn)
    pub fn add_upper_bound_as_intersect<F>(&mut self, tp_name: TypeName, ty: Ty<R>, intersect: F)
    where
        F: FnOnce(&Ty<R>, Option<&HashSet<Ty<R>>>) -> Vec<Ty<R>>,
    {
        // If we're adding U as the upper bound of T, also add T as the lower
        // bound of U
        if let Ty_::Tgeneric(formal_super, ps) = ty.deref() {
            if ps.is_empty() {
                self.add_lower_bound_help(
                    formal_super,
                    Ty::generic(ty.reason().clone(), tp_name, vec![]),
                );
            }
        }
        // We have a function which lets so replace the conjunction of a
        // upper bounds with the intersection of those bounds
        let upper_bounds: HashSet<Ty<R>> = HashSet::from_iter(
            intersect(&ty, self.upper_bounds(&tp_name))
                .into_iter()
                .filter(|ty| !Self::is_generic_param(ty, &tp_name, true)),
        );
        self.typarams
            .entry(tp_name)
            .or_insert((R::Pos::none(), Kind::default()))
            .1
            .set_upper_bounds(upper_bounds);
    }

    /// Add a single new upper lower [ty] to generic parameter [name].
    /// If the optional [union] operation is supplied, then use this to avoid
    /// adding redundant bounds by merging the type with existing bounds. This makes
    /// sense because a conjunction of lower bounds
    ///  (t1 <: T) /\ ... /\ (tn <: T)
    /// is equivalent to a single lower bound
    ///  (t1 | ... | tn) <: T
    pub fn add_lower_bound(&mut self, tp_name: TypeName, bound: Ty<R>) {
        // If we're adding U as the lower bound of T, also add T as the upper
        // bound of U
        if let Ty_::Tgeneric(formal_sub, ps) = bound.deref() {
            if ps.is_empty() {
                self.add_upper_bound_help(
                    formal_sub,
                    Ty::generic(bound.reason().clone(), tp_name, vec![]),
                );
            }
        }

        self.add_lower_bound_help(&tp_name, bound);
    }

    pub fn add_lower_bound_as_union<F>(&mut self, tp_name: TypeName, bound: Ty<R>, union: F)
    where
        F: FnOnce(&Ty<R>, Option<&HashSet<Ty<R>>>) -> Vec<Ty<R>>,
    {
        // If we're adding U as the lower bound of T, also add T as the upper
        // bound of U
        if let Ty_::Tgeneric(formal_sub, ps) = bound.deref() {
            if ps.is_empty() {
                self.add_upper_bound_help(
                    formal_sub,
                    Ty::generic(bound.reason().clone(), tp_name, vec![]),
                );
            }
        }
        // We have a function which lets so replace the conjunction of
        // lower bounds with the union of those bounds
        let lower_bounds: HashSet<Ty<R>> = HashSet::from_iter(
            union(&bound, self.lower_bounds(&tp_name))
                .into_iter()
                .filter(|ty| !Self::is_generic_param(ty, &tp_name, true)),
        );
        self.typarams
            .entry(tp_name)
            .or_insert((R::Pos::none(), Kind::default()))
            .1
            .set_lower_bounds(lower_bounds);
    }

    pub fn remove(&mut self, tp_name: TypeName) {
        let bound = Ty::generic(R::none(), tp_name, vec![]);

        // remove as upper bound from each of `tp_name`s lower bounds
        if let Some(lower_bounds) = self.lower_bounds(&tp_name) {
            for ty in lower_bounds.clone() {
                if let Ty_::Tgeneric(name, _) = ty.deref() {
                    self.remove_upper_bound(name, &bound)
                }
            }
        }

        // remove as lower bound from each of `tp_name`s upper bounds
        if let Some(upper_bounds) = self.upper_bounds(&tp_name) {
            for ty in upper_bounds.clone() {
                if let Ty_::Tgeneric(name, _) = ty.deref() {
                    self.remove_lower_bound(name, &bound)
                }
            }
        }

        // finally, remove the entry itself
        self.typarams.remove(&tp_name);
    }
}

impl<R: Reason> Default for TyparamEnv<R> {
    fn default() -> Self {
        Self {
            typarams: HashMap::default(),
            is_consistent: true,
        }
    }
}

#[cfg(test)]
mod tests {
    use pos::NPos;
    use ty::reason::NReason;

    use super::*;

    #[test]
    fn test_add() {
        let mut env = TyparamEnv::<NReason>::default();
        let tp_name = pos::TypeName::new("t");
        env.add(tp_name, NPos::none(), Kind::default());
        assert!(env.contains(&tp_name));
    }

    #[test]
    fn test_add_generic_upper_bound() {
        let mut env = TyparamEnv::<NReason>::default();

        let tp_t = pos::TypeName::new("t");
        let tp_u = pos::TypeName::new("u");
        let ty_t = Ty::generic(NReason::none(), tp_t, vec![]);
        let ty_u = Ty::generic(NReason::none(), tp_u, vec![]);

        env.add(tp_t, NPos::none(), Kind::default());
        env.add(tp_u, NPos::none(), Kind::default());
        env.add_upper_bound(tp_t.clone(), ty_u.clone());

        let t_upper = env.upper_bounds(&tp_t);
        let u_lower = env.lower_bounds(&tp_u);

        assert!(t_upper.map_or(false, |tys| tys.contains(&ty_u)));
        assert!(u_lower.map_or(false, |tys| tys.contains(&ty_t)));
    }
}
