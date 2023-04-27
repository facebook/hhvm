// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Block;
use nast::Expr;
use nast::Expr_;
use nast::Id;
use nast::ParamKind;
use nast::Pos;
use nast::Stmt;
use nast::Stmt_;
use nast::Uop;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ElabExprCallHhInvariantPass;

impl Pass for ElabExprCallHhInvariantPass {
    // We are elaborating a `Call` `Expr` into a `Stmt` so the transformation
    // is defined on `Stmt`
    fn on_ty_stmt__bottom_up(&mut self, env: &Env, elem: &mut Stmt_) -> ControlFlow<()> {
        match check_call(env, elem) {
            Check::Ignore => Continue(()),
            Check::Invalidate => {
                if let Stmt_::Expr(box expr) = elem {
                    let inner_expr = std::mem::replace(expr, elab_utils::expr::null());
                    *expr = elab_utils::expr::invalid(inner_expr);
                }
                Break(())
            }
            Check::Elaborate => {
                let old_stmt_ = std::mem::replace(elem, Stmt_::Noop);
                if let Stmt_::Expr(box Expr(
                    annot,
                    expr_pos,
                    Expr_::Call(box (
                        Expr(fn_expr_annot, fn_expr_pos, Expr_::Id(box Id(fn_name_pos, _))),
                        targs,
                        mut exprs,
                        unpacked_element,
                    )),
                )) = old_stmt_
                {
                    let (pk, expr) = exprs.remove(0);
                    // Raise error if this is an inout param
                    if let ParamKind::Pinout(ref pk_pos) = pk {
                        env.emit_error(NamingPhaseError::NastCheck(
                            NastCheckError::InoutInTransformedPseudofunction {
                                pos: Pos::merge(pk_pos, &fn_expr_pos).unwrap(),
                                fn_name: "invariant".to_string(),
                            },
                        ))
                    }
                    // Construct a call to `invariant_violation` for the
                    // false case
                    let id_expr = Expr_::Id(Box::new(Id(
                        fn_name_pos,
                        sn::autoimported_functions::INVARIANT_VIOLATION.into(),
                    )));
                    let fn_expr = Expr(fn_expr_annot, fn_expr_pos, id_expr);
                    let violation_expr = Expr(
                        annot,
                        expr_pos.clone(),
                        Expr_::Call(Box::new((fn_expr, targs, exprs, unpacked_element))),
                    );
                    // See if we have a bool constant as our condition; use the
                    // call to `invariant_violation` directly if we do and put
                    // into an `If` statement otherwise. Note that we put it
                    // on the true branch so the condition is negated.
                    match expr {
                        Expr(_, _, Expr_::False) => *elem = Stmt_::Expr(Box::new(violation_expr)),
                        Expr(cond_annot, cond_pos, cond_expr) => {
                            let true_block =
                                Block(vec![Stmt(expr_pos, Stmt_::Expr(Box::new(violation_expr)))]);
                            let false_block =
                                Block(vec![Stmt(elab_utils::pos::null(), Stmt_::Noop)]);
                            let cond_expr = Expr(
                                (),
                                cond_pos.clone(),
                                Expr_::Unop(Box::new((
                                    Uop::Unot,
                                    Expr(cond_annot, cond_pos, cond_expr),
                                ))),
                            );
                            *elem = Stmt_::If(Box::new((cond_expr, true_block, false_block)))
                        }
                    }
                }
                Continue(())
            }
        }
    }
}

enum Check {
    Ignore,
    Invalidate,
    Elaborate,
}

fn check_call(env: &Env, stmt: &Stmt_) -> Check {
    match stmt {
        Stmt_::Expr(box Expr(
            _,
            _,
            Expr_::Call(box (Expr(_, fn_expr_pos, Expr_::Id(box Id(_, fn_name))), _, exprs, _)),
        )) if fn_name == sn::autoimported_functions::INVARIANT => match exprs.get(0..1) {
            None | Some(&[]) => {
                env.emit_error(NamingPhaseError::Naming(NamingError::TooFewArguments(
                    fn_expr_pos.clone(),
                )));
                Check::Invalidate
            }
            Some(&[(ParamKind::Pnormal, _), ..]) => Check::Elaborate,
            Some(&[(ParamKind::Pinout(ref pk_pos), _), ..]) => {
                env.emit_error(NamingPhaseError::NastCheck(
                    NastCheckError::InoutInTransformedPseudofunction {
                        pos: Pos::merge(fn_expr_pos, pk_pos).unwrap(),
                        fn_name: "invariant".to_string(),
                    },
                ));
                Check::Elaborate
            }
        },

        _ => Check::Ignore,
    }
}

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_valid() {
        let env = Env::default();

        let mut pass = ElabExprCallHhInvariantPass;
        let mut elem = Stmt_::Expr(Box::new(Expr(
            (),
            elab_utils::pos::null(),
            Expr_::Call(Box::new((
                Expr(
                    (),
                    elab_utils::pos::null(),
                    Expr_::Id(Box::new(Id(
                        elab_utils::pos::null(),
                        sn::autoimported_functions::INVARIANT.to_string(),
                    ))),
                ),
                vec![],
                vec![(ParamKind::Pnormal, elab_utils::expr::null())],
                None,
            ))),
        )));
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());

        assert!(match elem {
            Stmt_::If(box (
                Expr(_, _, Expr_::Unop(box (Uop::Unot, Expr(_, _, Expr_::Null)))),
                Block(mut ts),
                Block(mut fs),
            )) => {
                match ts.pop().zip(fs.pop()) {
                    Some((
                        Stmt(
                            _,
                            Stmt_::Expr(box Expr(
                                _,
                                _,
                                Expr_::Call(box (
                                    Expr(_, _, Expr_::Id(box Id(_, fn_name))),
                                    _,
                                    exprs,
                                    _,
                                )),
                            )),
                        ),
                        Stmt(_, Stmt_::Noop),
                    )) => {
                        fn_name == sn::autoimported_functions::INVARIANT_VIOLATION
                            && exprs.is_empty()
                    }

                    _ => false,
                }
            }
            _ => false,
        })
    }

    #[test]
    fn test_valid_false() {
        let env = Env::default();

        let mut pass = ElabExprCallHhInvariantPass;
        let mut elem = Stmt_::Expr(Box::new(Expr(
            (),
            elab_utils::pos::null(),
            Expr_::Call(Box::new((
                Expr(
                    (),
                    elab_utils::pos::null(),
                    Expr_::Id(Box::new(Id(
                        elab_utils::pos::null(),
                        sn::autoimported_functions::INVARIANT.to_string(),
                    ))),
                ),
                vec![],
                vec![(
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::False),
                )],
                None,
            ))),
        )));
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());

        assert!(match elem {
            Stmt_::Expr(box Expr(
                _,
                _,
                Expr_::Call(box (Expr(_, _, Expr_::Id(box Id(_, fn_name))), _, exprs, _)),
            )) => fn_name == sn::autoimported_functions::INVARIANT_VIOLATION && exprs.is_empty(),
            _ => false,
        })
    }

    #[test]
    fn test_too_few_args() {
        let env = Env::default();

        let mut pass = ElabExprCallHhInvariantPass;
        let mut elem = Stmt_::Expr(Box::new(Expr(
            (),
            elab_utils::pos::null(),
            Expr_::Call(Box::new((
                Expr(
                    (),
                    elab_utils::pos::null(),
                    Expr_::Id(Box::new(Id(
                        elab_utils::pos::null(),
                        sn::autoimported_functions::INVARIANT.to_string(),
                    ))),
                ),
                vec![],
                vec![],
                None,
            ))),
        )));
        elem.transform(&env, &mut pass);

        let too_few_args_err_opt = env.into_errors().pop();
        assert!(matches!(
            too_few_args_err_opt,
            Some(NamingPhaseError::Naming(NamingError::TooFewArguments(_)))
        ));

        // Expect our original expression to be wrapped in `Invalid`
        assert!(matches!(
            elem,
            Stmt_::Expr(box Expr(_, _, Expr_::Invalid(_)))
        ))
    }
}
