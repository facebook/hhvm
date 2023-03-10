// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::tast::Pos;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabExprImportPass;

impl Pass for ElabExprImportPass {
    // Wrap all occurrence of `Import` in an `Invalid` marker
    // TODO[mjt] Oddly there is no error reporting about the occurrence of these
    // expressions and no invariant is assumed. I would expect one of these
    fn on_ty_expr_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut Expr<Ex, En>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()> {
        let Expr(_, _, expr_) = elem;
        match expr_ {
            Expr_::Import(_) => {
                let inner_expr =
                    std::mem::replace(elem, Expr(Ex::default(), Pos::NONE, Expr_::Null));
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

    use super::*;
    use crate::Transform;

    // -- ValCollection --------------------------------------------------------

    #[test]
    fn test_val_collection_empty() {
        let cfg = Config::default();

        let mut pass = ElabExprImportPass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Import(Box::new((
                ImportFlavor::Include,
                Expr((), Pos::NONE, Expr_::Null),
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(expr_, Expr_::Invalid(_)));
    }
}
