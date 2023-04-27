// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Expr;
use nast::Expr_;
use nast::Id;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateExprCallEchoPass;

impl Pass for ValidateExprCallEchoPass {
    fn on_ty_expr__bottom_up(&mut self, env: &Env, elem: &mut Expr_) -> ControlFlow<()> {
        match elem {
            Expr_::Call(box (
                Expr(_, _, Expr_::Id(box Id(_, fn_name))),
                _,
                _,
                Some(Expr(_, pos, _)),
            )) if fn_name == sn::special_functions::ECHO => {
                env.emit_error(NamingError::TooFewTypeArguments(pos.clone()))
            }
            _ => (),
        }
        Continue(())
    }
}

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_valid() {
        let env = Env::default();

        let mut pass = ValidateExprCallEchoPass;
        let mut elem = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::special_functions::ECHO.to_string(),
                ))),
            ),
            vec![],
            vec![],
            None,
        )));
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(match elem {
            Expr_::Call(cc) => {
                let (Expr(_, _, expr_), _, _, _) = *cc;
                match expr_ {
                    Expr_::Id(id) => {
                        let Id(_, nm) = *id;
                        nm == sn::special_functions::ECHO
                    }
                    _ => false,
                }
            }
            _ => false,
        })
    }

    #[test]
    fn test_invalid() {
        let env = Env::default();

        let mut pass = ValidateExprCallEchoPass;
        let mut elem = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::special_functions::ECHO.to_string(),
                ))),
            ),
            vec![],
            vec![],
            Some(elab_utils::expr::null()),
        )));
        elem.transform(&env, &mut pass);

        assert_eq!(env.into_errors().len(), 1);
        assert!(match elem {
            Expr_::Call(cc) => {
                let (Expr(_, _, expr_), _, _, _) = *cc;
                match expr_ {
                    Expr_::Id(id) => {
                        let Id(_, nm) = *id;
                        nm == sn::special_functions::ECHO
                    }
                    _ => false,
                }
            }
            _ => false,
        })
    }
}
