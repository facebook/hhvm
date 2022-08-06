// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use im::HashMap;
use im::HashSet;
use pos::ToOxidized;
use ty::local::Tyvar;

#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct TyvarOccurrences {
    /// map from a tyvar to the tyvars which occur in it's binding
    /// e.g. if #1 is bound to (#2 | int) this map contains the entry
    /// #1 => { #2 }
    tyvs_in: HashMap<Tyvar, HashSet<Tyvar>>,

    /// map from a tyvar to the tyvars for which it occurs as part of the
    /// binding e.g. if #1 is bound to (#2 | int) this map contains the entry
    /// #2 => { #1 }
    occs_of: HashMap<Tyvar, HashSet<Tyvar>>,
}

impl TyvarOccurrences {
    pub fn is_empty(&self) -> bool {
        self.tyvs_in.is_empty()
    }

    /// Check if [tv_occ] occurs inside [tv_in], ensuring the occurrences
    /// invariant is met
    pub fn occurs_in(&self, tv_occ: &Tyvar, tv_in: &Tyvar) -> bool {
        let fwd = self
            .tyvs_in
            .get(tv_in)
            .map_or(false, |tvs| tvs.contains(tv_occ));
        let bwd = self
            .occs_of
            .get(tv_occ)
            .map_or(false, |tvs| tvs.contains(tv_in));
        assert_eq!(fwd, bwd);
        fwd
    }

    /// Check if [tv] has any type variables in its binding
    pub fn contains_any(&self, tv: &Tyvar) -> bool {
        self.tyvs_in.get(tv).map_or(false, |m| !m.is_empty())
    }

    /// Construct a new occurrences map by adding the occurrence of the
    /// type variable [tv_occ] within [ty_in]
    pub fn add_occurrence(&mut self, tv_occ: Tyvar, tv_in: Tyvar) {
        self.tyvs_in.entry(tv_in).or_default().insert(tv_occ);
        self.occs_of.entry(tv_occ).or_default().insert(tv_in);
    }

    /// Construct a new occurrences map by removing the occurrence of the
    /// type variable [tv_occ] within [ty_in]
    pub fn remove_occurrence(&mut self, tv_occ: Tyvar, tv_in: Tyvar) {
        self.tyvs_in.entry(tv_in).and_modify(|m| {
            m.remove(&tv_occ);
        });
        self.occs_of.entry(tv_occ).and_modify(|m| {
            m.remove(&tv_in);
        });
    }

    /// Remove occurrences after unbinding a type variable
    pub fn unbind(&mut self, tv: Tyvar) {
        if let Some(vs) = self.tyvs_in.remove(&tv) {
            vs.into_iter().for_each(|tv_occ| {
                self.occs_of.entry(tv_occ).and_modify(|m| {
                    m.remove(&tv);
                });
            })
        }
    }

    /// Remove a type variable from the binding structure i.e. remove any
    /// occurrence of the type variable from any binding in which it appears
    /// and remove the occurrence of any type variables which appear in its
    /// own binding
    pub fn remove(&mut self, tv: Tyvar) {
        if let Some(vs) = self.occs_of.remove(&tv) {
            vs.into_iter().for_each(|tv_in| {
                self.tyvs_in.entry(tv_in).and_modify(|m| {
                    m.remove(&tv);
                });
            })
        }
        self.unbind(tv);
    }
}

impl<'a> ToOxidized<'a> for TyvarOccurrences {
    type Output = &'a oxidized_by_ref::typing_tyvar_occurrences::TypingTyvarOccurrences<'a>;

    fn to_oxidized(&self, bump: &'a bumpalo::Bump) -> Self::Output {
        use oxidized_by_ref::i_map::IMap;
        use oxidized_by_ref::i_set::ISet;
        use oxidized_by_ref::typing_tyvar_occurrences::TypingTyvarOccurrences;

        let Self { tyvs_in, occs_of } = self;
        let conv = |x: &HashMap<Tyvar, HashSet<Tyvar>>| {
            IMap::from(
                bump,
                x.iter().map(|(k, v)| {
                    (
                        k.to_oxidized(bump),
                        ISet::from(bump, v.iter().map(|v| v.to_oxidized(bump))),
                    )
                }),
            )
        };
        let occ = TypingTyvarOccurrences {
            tyvar_occurrences: conv(occs_of),
            tyvars_in_tyvar: conv(tyvs_in),
        };
        bump.alloc(occ)
    }
}

#[cfg(test)]
mod tests {
    use utils::core::IdentGen;

    use super::*;

    #[test]
    fn test_add_remove_id() {
        let mut occs: TyvarOccurrences = Default::default();
        let gen = IdentGen::new();
        let tv_occ: Tyvar = gen.make().into();
        let tv_in: Tyvar = gen.make().into();

        occs.add_occurrence(tv_occ, tv_in);
        assert!(occs.occurs_in(&tv_occ, &tv_in));
        assert!(!occs.occurs_in(&tv_in, &tv_occ));

        occs.remove_occurrence(tv_occ, tv_in);
        assert!(!occs.occurs_in(&tv_occ, &tv_in));
    }

    #[test]
    fn test_unbind() {
        // Given the bindings
        // #1 = (#2 | #3)
        // #4 = (#1 | #3)
        // we have:
        // tyvs_in:
        //     #1 => { #2 , #3}
        //     #4 => { #1 , #3}
        // occs_of:
        //     #1 => { #4 }
        //     #2 => { #1 }
        //     #3 => { #1 , #4 }
        //
        // After unbinding #1, we have
        // tyvs_in:
        //     #4 => { #1 , #3 }
        // occs_of:
        //     #1 => { #4 }
        //     #2 => { }
        //     #3 => { #4 }
        let mut occs: TyvarOccurrences = Default::default();
        let gen = IdentGen::new();
        let tv1: Tyvar = gen.make().into();
        let tv2: Tyvar = gen.make().into();
        let tv3: Tyvar = gen.make().into();
        let tv4: Tyvar = gen.make().into();

        occs.add_occurrence(tv2, tv1);
        occs.add_occurrence(tv3, tv1);
        occs.add_occurrence(tv3, tv4);
        occs.add_occurrence(tv1, tv4);

        assert!(occs.occurs_in(&tv2, &tv1));
        assert!(occs.occurs_in(&tv3, &tv1));
        assert!(occs.occurs_in(&tv1, &tv4));
        assert!(occs.occurs_in(&tv3, &tv4));

        occs.unbind(tv1);
        assert!(!occs.occurs_in(&tv2, &tv1));
        assert!(!occs.occurs_in(&tv3, &tv1));
        assert!(occs.occurs_in(&tv1, &tv4));
        assert!(occs.occurs_in(&tv3, &tv4));
    }

    #[test]
    fn test_remove() {
        // Given the bindings
        // #1 = (#2 | #3)
        // #4 = (#1 | #3)
        // we have:
        // tyvs_in:
        //     #1 => { #2 , #3}
        //     #4 => { #1 , #3}
        // occs_of:
        //     #1 => { #4 }
        //     #2 => { #1 }
        //     #3 => { #1 , #4 }
        //
        // After removing #1, we have
        // tyvs_in:
        //     #4 => { #3 }
        // occs_of:
        //     #2 => { }
        //     #3 => { #4 }
        let mut occs: TyvarOccurrences = Default::default();
        let gen = IdentGen::new();
        let tv1: Tyvar = gen.make().into();
        let tv2: Tyvar = gen.make().into();
        let tv3: Tyvar = gen.make().into();
        let tv4: Tyvar = gen.make().into();

        occs.add_occurrence(tv2, tv1);
        occs.add_occurrence(tv3, tv1);
        occs.add_occurrence(tv3, tv4);
        occs.add_occurrence(tv1, tv4);

        assert!(occs.occurs_in(&tv2, &tv1));
        assert!(occs.occurs_in(&tv3, &tv1));
        assert!(occs.occurs_in(&tv1, &tv4));
        assert!(occs.occurs_in(&tv3, &tv4));

        occs.remove(tv1);
        assert!(!occs.occurs_in(&tv2, &tv1));
        assert!(!occs.occurs_in(&tv3, &tv1));
        assert!(!occs.occurs_in(&tv1, &tv4));
        assert!(occs.occurs_in(&tv3, &tv4));
    }

    #[test]
    fn test_contains_any() {
        let mut occs: TyvarOccurrences = Default::default();
        let gen = IdentGen::new();
        let tv1: Tyvar = gen.make().into();
        let tv2: Tyvar = gen.make().into();
        let tv3: Tyvar = gen.make().into();
        let tv4: Tyvar = gen.make().into();
        occs.add_occurrence(tv2, tv1);
        occs.add_occurrence(tv3, tv1);
        occs.add_occurrence(tv1, tv4);

        assert!(occs.contains_any(&tv1));
        assert!(!occs.contains_any(&tv2));
        assert!(!occs.contains_any(&tv3));
        assert!(occs.contains_any(&tv4));
    }
}
