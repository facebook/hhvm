// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use im::HashMap;
use im::HashSet;
use pos::TypeName;
use ty::local::Ty;
use ty::reason::Reason;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct VisitedGoals<R: Reason>(HashMap<TypeName, (HashSet<Ty<R>>, HashSet<Ty<R>>)>);

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum GoalResult {
    AlreadyVisited,
    NewGoal,
}

impl<R: Reason> Default for VisitedGoals<R> {
    fn default() -> Self {
        VisitedGoals(HashMap::new())
    }
}

impl<R: Reason> VisitedGoals<R> {
    /// Add `ty` to the set of visited subtype constraint returning the added bound if
    /// if has not already been visited
    pub fn try_add_visited_generic_sup(&mut self, tp_name: TypeName, ty: &Ty<R>) -> GoalResult {
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
    pub fn try_add_visited_generic_sub(&mut self, tp_name: TypeName, ty: &Ty<R>) -> GoalResult {
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
    use ty::reason::NReason;

    use super::*;

    #[test]
    fn test_visited_goals() {
        let mut goals = VisitedGoals::default();
        let tp_name = TypeName::new("T");
        let ty_int = Ty::int(NReason::none());
        let added_super = goals.try_add_visited_generic_sub(tp_name, &ty_int);
        assert!(matches!(added_super, GoalResult::NewGoal));
    }
}
