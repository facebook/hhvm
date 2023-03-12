// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::nast::Expr_;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct GuardInvalidPass;

impl Pass for GuardInvalidPass {
    fn on_ty_expr__top_down(&mut self, _: &Env, elem: &mut Expr_) -> ControlFlow<()> {
        if matches!(elem, Expr_::Invalid(..)) {
            ControlFlow::Break(())
        } else {
            ControlFlow::Continue(())
        }
    }
}

#[cfg(test)]
mod tests {

    use oxidized::nast::Bop;
    use oxidized::nast::Expr;
    use oxidized::nast::Expr_;
    use oxidized::nast::Pos;

    use super::*;
    use crate::Pass;
    use crate::Transform;

    #[derive(Clone)]
    pub struct RewriteZero;
    impl Pass for RewriteZero {
        fn on_ty_expr__bottom_up(&mut self, _: &Env, elem: &mut Expr_) -> ControlFlow<()> {
            match elem {
                Expr_::Int(..) => *elem = Expr_::Int("0".to_string()),
                _ => (),
            }
            ControlFlow::Continue(())
        }
    }

    #[test]
    fn test() {
        let env = Env::default();

        let mut pass = passes![GuardInvalidPass, RewriteZero];

        let mut elem = Expr_::Binop(Box::new((
            Bop::Lt,
            Expr(
                (),
                Pos::NONE,
                Expr_::Invalid(Box::new(Some(Expr(
                    (),
                    Pos::NONE,
                    Expr_::Int("42".to_string()),
                )))),
            ),
            Expr((), Pos::NONE, Expr_::Int("43".to_string())),
        )));

        elem.transform(&env, &mut pass);

        assert!(matches!(
            elem,
            Expr_::Binop(box (
                _,
                Expr(_, _, Expr_::Invalid(box Some(Expr(_, _, Expr_::Int(n))))),
                Expr(_, _, Expr_::Int(m)),
            ))
            if n == "42" && m == "0"
        ));
    }
}
