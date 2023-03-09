// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Utilities for elaboration passes.

/// Factory functions constructing positions.
pub(crate) mod pos {
    use oxidized::aast_defs::Pos;

    /// Construct a "null" position.
    #[inline(always)]
    pub(crate) fn null() -> Pos {
        Pos::NONE
    }
}

/// Factory functions constructing hints.
pub(crate) mod hint {
    use oxidized::aast::Hint;
    use oxidized::aast::Hint_;

    /// Construct a "null" hint.
    #[allow(dead_code)] // Only called from #[test] functions.
    #[inline(always)]
    pub(crate) fn null() -> Hint {
        Hint(super::pos::null(), Box::new(Hint_::Hnothing))
    }
}

/// Factory functions constructing expressions.
pub(crate) mod expr {
    use oxidized::aast_defs::Expr;
    use oxidized::aast_defs::Expr_;
    use oxidized::aast_defs::Pos;

    /// Construct a "null" expression.
    #[inline(always)]
    pub(crate) fn null<Ex: Default, En>() -> Expr<Ex, En> {
        from_expr_(Expr_::Null)
    }

    /// Construct an "invalid" expression.
    #[inline(always)]
    pub(crate) fn invalid<Ex: Default, En>(expr: Expr<Ex, En>) -> Expr<Ex, En> {
        let Expr(_, pos, _) = &expr;
        from_expr__with_pos_(pos.clone(), Expr_::Invalid(Box::new(Some(expr))))
    }

    /// Construct an expression (with a null position) from an `Expr_`.
    #[inline(always)]
    pub(crate) fn from_expr_<Ex: Default, En>(expr_: Expr_<Ex, En>) -> Expr<Ex, En> {
        from_expr__with_pos_(super::pos::null(), expr_)
    }

    /// Construct an expression from a `Pos` and an `Expr_`.
    #[inline(always)]
    #[allow(non_snake_case)]
    pub(crate) fn from_expr__with_pos_<Ex: Default, En>(
        pos: Pos,
        expr_: Expr_<Ex, En>,
    ) -> Expr<Ex, En> {
        Expr(Ex::default(), pos, expr_)
    }
}
