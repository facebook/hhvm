// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::naming_error::NamingError;

use crate::config::Config;
use crate::elab_utils;
use crate::Pass;

/// Replace empty tuples with invalid expressions and record errors.
#[derive(Clone, Copy, Default)]
pub struct ElabExprTuplePass;

impl Pass for ElabExprTuplePass {
    fn on_ty_expr_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut Expr<Ex, En>,
        cfg: &Config,
    ) -> ControlFlow<(), ()> {
        if let Expr(_annot, pos, Expr_::Tuple(es)) = elem {
            if es.is_empty() {
                // Loc. of the empty tuple.
                let pos = pos.clone();
                // Steal the contents of `elem`.
                let expr = std::mem::replace(elem, elab_utils::expr::null());
                // Wrap them in `Invalid` and install the result back into
                // `elem`.
                *elem = elab_utils::expr::invalid(expr);
                // Record the error and break.
                cfg.emit_error(NamingError::TooFewArguments(pos));
                return ControlFlow::Break(());
            }
        }
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use super::*;
    use crate::elab_utils;
    use crate::Transform;

    #[test]
    fn test_empty_tuple() {
        let cfg = Config::default();

        let mut pass = ElabExprTuplePass;

        let mut elem: Expr<(), ()> = elab_utils::expr::from_expr_(Expr_::Tuple(vec![]));
        elem.transform(&cfg, &mut pass);
        assert!(matches!(elem, Expr(_, _, Expr_::Invalid(_))));
    }
}
