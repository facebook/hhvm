// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Variance of a type wrt to a given type parameter.
///
/// Standard variance lattice.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Variance {
    /// R is bivariant (or constant) in X when [S/X]R <: [T/X]R for every S and T.
    Bivariant,
    /// R is covariant in X when Γ ⊢ [S/X]R <: [T/X]R iff Γ ⊢ S <: T.
    Covariant,
    /// R is contravariant in X when Γ ⊢ [T/X]R <: [S/X]R iff Γ ⊢ S <: T.
    Contravariant,
    /// R is invariant in X when Γ ⊢ [S/X]R <: [T/X]R iff both Γ ⊢ S <: T and Γ ⊢ T <: S
    Invariant,
}

impl Variance {
    pub fn appears_covariantly(&self) -> bool {
        match self {
            Variance::Covariant | Variance::Invariant => true,
            Variance::Contravariant | Variance::Bivariant => false,
        }
    }

    pub fn appears_contravariantly(&self) -> bool {
        match self {
            Variance::Contravariant | Variance::Invariant => true,
            Variance::Covariant | Variance::Bivariant => false,
        }
    }

    pub fn is_bivariant(&self) -> bool {
        matches!(self, Variance::Bivariant)
    }

    pub const TOP: Self = Variance::Bivariant;

    pub const BOTTOM: Self = Variance::Invariant;

    /// The least upper bound of two variances
    pub fn join(&self, other: &Self) -> Self {
        match (*self, *other) {
            (t, u) if t == u => t,
            (t, Variance::Invariant) | (Variance::Invariant, t) => t,
            _ => Variance::Bivariant,
        }
    }

    /// Compute the greatest lower bound of two variances
    pub fn meet(&self, other: &Self) -> Self {
        match (*self, *other) {
            (t, u) if t == u => t,
            (Variance::Bivariant, t) | (t, Variance::Bivariant) => t,
            _ => Variance::Invariant,
        }
    }
}

impl Default for Variance {
    fn default() -> Self {
        Variance::Bivariant
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_join_bottom() {
        let cov = Variance::Covariant;
        let contrav = Variance::Contravariant;
        let biv = Variance::Bivariant;
        let inv = Variance::Invariant;
        let bot = Variance::BOTTOM;
        assert_eq!(bot.join(&cov), cov);
        assert_eq!(bot.join(&contrav), contrav);
        assert_eq!(bot.join(&biv), biv);
        assert_eq!(bot.join(&inv), inv);
    }

    #[test]
    fn test_meet_bottom() {
        let cov = Variance::Covariant;
        let contrav = Variance::Contravariant;
        let biv = Variance::Bivariant;
        let inv = Variance::Invariant;
        let bot = Variance::BOTTOM;
        assert_eq!(bot.meet(&cov), bot);
        assert_eq!(bot.meet(&contrav), bot);
        assert_eq!(bot.meet(&biv), bot);
        assert_eq!(bot.meet(&inv), bot);
    }

    #[test]
    fn test_meet_top() {
        let cov = Variance::Covariant;
        let contrav = Variance::Contravariant;
        let biv = Variance::Bivariant;
        let inv = Variance::Invariant;
        let top = Variance::TOP;
        assert_eq!(top.meet(&cov), cov);
        assert_eq!(top.meet(&contrav), contrav);
        assert_eq!(top.meet(&biv), biv);
        assert_eq!(top.meet(&inv), inv);
    }

    #[test]
    fn test_join_top() {
        let cov = Variance::Covariant;
        let contrav = Variance::Contravariant;
        let biv = Variance::Bivariant;
        let inv = Variance::Invariant;
        let top = Variance::TOP;
        assert_eq!(top.join(&cov), top);
        assert_eq!(top.join(&contrav), top);
        assert_eq!(top.join(&biv), top);
        assert_eq!(top.join(&inv), top);
    }

    #[test]
    fn test_cov_contra() {
        let cov = Variance::Covariant;
        let contrav = Variance::Contravariant;
        let top = Variance::TOP;
        let bot = Variance::BOTTOM;
        assert_eq!(cov.join(&contrav), top);
        assert_eq!(contrav.join(&cov), top);
        assert_eq!(cov.meet(&contrav), bot);
        assert_eq!(contrav.meet(&cov), bot);
    }
}
