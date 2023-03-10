// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::naming_error::NamingError;
use oxidized::nast::Expr;
use oxidized::nast::Expr_;
use oxidized::nast::Id;
use oxidized::nast::ParamKind;
use oxidized::nast::Pos;
use oxidized::nast_check_error::NastCheckError;

use crate::elab_utils;
use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabExprCallCallUserFuncPass;

impl Pass for ElabExprCallCallUserFuncPass {
    fn on_ty_expr__bottom_up(&mut self, env: &Env, elem: &mut Expr_) -> ControlFlow<()> {
        match elem {
            Expr_::Call(box (
                Expr(_, fn_expr_pos, Expr_::Id(box Id(_, fn_name))),
                _,
                fn_param_exprs,
                _,
            )) if fn_name == sn::std_lib_functions::CALL_USER_FUNC && fn_param_exprs.is_empty() => {
                // We're cloning here since we need to preserve the entire expression
                env.emit_error(NamingError::DeprecatedUse {
                    pos: fn_expr_pos.clone(),
                    fn_name: fn_name.clone(),
                });
                env.emit_error(NamingError::TooFewArguments(fn_expr_pos.clone()));
                let inner_expr_ = std::mem::replace(elem, Expr_::Null);
                let inner_expr = elab_utils::expr::from_expr_(inner_expr_);
                *elem = Expr_::Invalid(Box::new(Some(inner_expr)));
                ControlFlow::Break(())
            }

            Expr_::Call(box (fn_expr, _, fn_param_exprs, fn_variadic_param_opt))
                if is_expr_call_user_func(env, fn_expr) && !fn_param_exprs.is_empty() =>
            {
                // remove the first element of `fn_param_exprs`
                let (param_kind, head_expr) = fn_param_exprs.remove(0);
                // raise an error if this is an inout param
                if let ParamKind::Pinout(pk_pos) = &param_kind {
                    let pos = Pos::merge(pk_pos, &fn_expr.1).unwrap();
                    env.emit_error(NastCheckError::InoutInTransformedPseudofunction {
                        pos,
                        fn_name: sn::std_lib_functions::CALL_USER_FUNC.to_string(),
                    })
                }
                // use the first argument as the function expression
                *fn_expr = head_expr;
                // TODO[mjt] why are we dropping the unpacked variadic arg here?
                *fn_variadic_param_opt = None;
                ControlFlow::Continue(())
            }
            _ => ControlFlow::Continue(()),
        }
    }
}

fn is_expr_call_user_func(env: &Env, expr: &Expr) -> bool {
    match expr {
        Expr(_, pos, Expr_::Id(box id)) if id.name() == sn::std_lib_functions::CALL_USER_FUNC => {
            env.emit_error(NamingError::DeprecatedUse {
                pos: pos.clone(),
                fn_name: id.name().to_string(),
            });
            true
        }
        _ => false,
    }
}

#[cfg(test)]
mod tests {

    use oxidized::naming_phase_error::NamingPhaseError;

    use super::*;
    use crate::elab_utils;
    use crate::Transform;

    #[test]
    fn test_valid() {
        let env = Env::default();

        let mut pass = ElabExprCallCallUserFuncPass;
        let mut elem = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::std_lib_functions::CALL_USER_FUNC.to_string(),
                ))),
            ),
            vec![],
            vec![(ParamKind::Pnormal, elab_utils::expr::null())],
            None,
        )));
        elem.transform(&env, &mut pass);

        // Expect one deprecation error in the valid case
        let depr_err_opt = env.into_errors().pop();
        assert!(matches!(
            depr_err_opt,
            Some(NamingPhaseError::Naming(NamingError::DeprecatedUse { .. }))
        ));
        // Expect our parameter to be the call expression and the args to now
        // be empty
        assert!(match elem {
            Expr_::Call(cc) => {
                let (Expr(_, _, expr_), _, args, _) = *cc;
                matches!(expr_, Expr_::Null) && args.is_empty()
            }
            _ => false,
        })
    }

    #[test]
    fn test_no_args() {
        let env = Env::default();

        let mut pass = ElabExprCallCallUserFuncPass;
        let mut elem = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::std_lib_functions::CALL_USER_FUNC.to_string(),
                ))),
            ),
            vec![],
            vec![],
            None,
        )));
        elem.transform(&env, &mut pass);
        let mut errs = env.into_errors();

        // Expect errors for too few args and deprecation
        let too_few_args_err_opt = errs.pop();
        assert!(matches!(
            too_few_args_err_opt,
            Some(NamingPhaseError::Naming(NamingError::TooFewArguments(_)))
        ));

        let depr_err_opt = errs.pop();
        assert!(matches!(
            depr_err_opt,
            Some(NamingPhaseError::Naming(NamingError::DeprecatedUse { .. }))
        ));

        // Expect our original expression to be wrapped in `Invalid`
        assert!(match elem {
            Expr_::Invalid(expr) => {
                if let Some(Expr(_, _, Expr_::Call(box (Expr(_, _, Expr_::Id(id)), _, _, _)))) =
                    *expr
                {
                    id.name() == sn::std_lib_functions::CALL_USER_FUNC
                } else {
                    false
                }
            }
            _ => false,
        })
    }
}
