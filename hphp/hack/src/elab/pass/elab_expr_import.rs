// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::tast::Pos;
use transform::Pass;

use crate::context::Context;

pub struct ElabExprImportPass;

impl Pass for ElabExprImportPass {
    type Ctx = Context;
    type Err = NamingPhaseError;

    // Wrap all occurrence of `Import` in an `Invalid` marker
    // TODO[mjt] Oddly there is no error reporting about the occurrence of these
    // expressions and no invariant is assumed. I would expect one of these
    fn on_ty_expr<Ex: Default, En>(
        &self,
        elem: &mut Expr<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        let Expr(_, _, expr_) = elem;
        match expr_ {
            Expr_::Import(_) => {
                let inner_expr =
                    std::mem::replace(elem, Expr(Ex::default(), Pos::make_none(), Expr_::Null));
                *elem = Expr(
                    Ex::default(),
                    inner_expr.1.clone(),
                    Expr_::Invalid(Box::new(Some(inner_expr))),
                );
                ControlFlow::Break(())
            }
            _ => ControlFlow::Continue(()),
        }
    }
}

#[cfg(test)]
mod tests {
    use oxidized::aast_defs::ImportFlavor;
    use transform::Transform;

    use super::*;

    pub struct Identity;
    impl Pass for Identity {
        type Err = NamingPhaseError;
        type Ctx = Context;
    }

    // -- ValCollection --------------------------------------------------------

    #[test]
    fn test_val_collection_empty() {
        let mut ctx = Context::default();
        let mut errs = Vec::default();
        let top_down = ElabExprImportPass;
        let bottom_up = Identity;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::make_none(),
            Expr_::Import(Box::new((
                ImportFlavor::Include,
                Expr((), Pos::make_none(), Expr_::Null),
            ))),
        );

        elem.transform(&mut ctx, &mut errs, &top_down, &bottom_up);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(expr_, Expr_::Invalid(_)));
    }
}
