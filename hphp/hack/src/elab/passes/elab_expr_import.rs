// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Expr;
use nast::Expr_;
use nast::Pos;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ElabExprImportPass;

impl Pass for ElabExprImportPass {
    // Wrap all occurrence of `Import` in an `Invalid` marker
    // TODO[mjt] Oddly there is no error reporting about the occurrence of these
    // expressions and no invariant is assumed. I would expect one of these
    fn on_ty_expr_top_down(&mut self, _: &Env, elem: &mut Expr) -> ControlFlow<()> {
        let Expr(_, _, expr_) = elem;
        match expr_ {
            Expr_::Import(_) => {
                let inner_expr = std::mem::replace(elem, Expr((), Pos::NONE, Expr_::Null));
                *elem = Expr(
                    (),
                    inner_expr.1.clone(),
                    Expr_::Invalid(Box::new(Some(inner_expr))),
                );
                Break(())
            }
            _ => Continue(()),
        }
    }
}

#[cfg(test)]
mod tests {
    use nast::ImportFlavor;

    use super::*;

    // -- ValCollection --------------------------------------------------------

    #[test]
    fn test_val_collection_empty() {
        let env = Env::default();

        let mut pass = ElabExprImportPass;
        let mut elem: Expr = Expr(
            (),
            Pos::NONE,
            Expr_::Import(Box::new((
                ImportFlavor::Include,
                Expr((), Pos::NONE, Expr_::Null),
            ))),
        );

        elem.transform(&env, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(expr_, Expr_::Invalid(_)));
    }
}
