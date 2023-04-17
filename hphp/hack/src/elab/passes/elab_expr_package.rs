// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::pseudo_functions;
use nast::Expr;
use nast::Expr_;
use nast::Id;
use nast::ParamKind;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ElabExprPackagePass;

impl Pass for ElabExprPackagePass {
    fn on_ty_expr_top_down(&mut self, _: &Env, elem: &mut Expr) -> ControlFlow<()> {
        let Expr(_, pos, expr) = elem;
        match expr {
            Expr_::Package(box pkg) => {
                *expr = Expr_::Call(Box::new((
                    Expr::new(
                        (),
                        pos.clone(),
                        Expr_::mk_id(Id(pos.clone(), pseudo_functions::PACKAGE_EXISTS.into())),
                    ),
                    vec![],
                    vec![(
                        ParamKind::Pnormal,
                        Expr::new((), pkg.pos().clone(), Expr_::mk_string(pkg.name().into())),
                    )],
                    None,
                )));
                Break(())
            }
            _ => Continue(()),
        }
    }
}

#[cfg(test)]
mod tests {
    use nast::Id;
    use nast::Pos;

    use super::*;

    #[test]
    fn test() {
        let env = Env::default();

        let mut pass = ElabExprPackagePass;
        let mut elem: Expr = Expr(
            (),
            Pos::NONE,
            Expr_::Package(Box::new(Id(Pos::NONE, "foo".into()))),
        );

        elem.transform(&env, &mut pass);

        let Expr(_, _, expr) = elem;
        assert!(match expr {
            Expr_::Call(box (Expr(_, _, Expr_::Id(box Id(_, fn_name))), _, fn_param_exprs, _))
                if fn_name == pseudo_functions::PACKAGE_EXISTS =>
            {
                if let [(ParamKind::Pnormal, Expr(_, _, Expr_::String(fn_param_name)))] =
                    &fn_param_exprs[..]
                {
                    fn_param_name == "foo"
                } else {
                    false
                }
            }
            _ => false,
        });
    }
}
