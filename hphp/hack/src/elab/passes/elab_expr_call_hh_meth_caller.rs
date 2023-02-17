// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::ClassId;
use oxidized::aast_defs::ClassId_;
use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::Pstring;
use oxidized::ast_defs::Id;
use oxidized::ast_defs::ParamKind;
use oxidized::naming_error::NamingError;
use oxidized::naming_phase_error::NamingPhaseError;

use crate::config::Config;
use crate::elab_utils;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabExprCallHhMethCallerPass;

impl Pass for ElabExprCallHhMethCallerPass {
    #[allow(non_snake_case)]
    fn on_ty_expr__bottom_up<Ex: Default, En>(
        &mut self,
        elem: &mut Expr_<Ex, En>,
        _cfg: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()> {
        match check_call(elem, errs) {
            Check::Ignore => ControlFlow::Continue(()),
            Check::Invalidate => {
                let expr_ = std::mem::replace(elem, Expr_::Null);
                let inner_expr = elab_utils::expr::from_expr_(expr_);
                *elem = Expr_::Invalid(Box::new(Some(inner_expr)));
                ControlFlow::Break(())
            }
            Check::Elaborate => {
                let expr_ = std::mem::replace(elem, Expr_::Null);
                if let Expr_::Call(box (_, _, mut fn_param_exprs, _)) = expr_ {
                    let meth_arg = fn_param_exprs.pop();
                    let rcvr_arg = fn_param_exprs.pop();
                    match rcvr_arg.zip(meth_arg) {
                        Some((
                            (_, Expr(_, pc, Expr_::String(rcvr))),
                            (_, Expr(_, pm, Expr_::String(meth))),
                        )) => {
                            *elem = Expr_::MethodCaller(Box::new((
                                Id(pc, rcvr.to_string()),
                                (pm, meth.to_string()),
                            )))
                        }
                        Some((
                            (
                                _,
                                Expr(
                                    _,
                                    _,
                                    Expr_::ClassConst(box (ClassId(_, _, ClassId_::CI(id)), _)),
                                ),
                            ),
                            (_, Expr(_, pm, Expr_::String(meth))),
                        )) => *elem = Expr_::MethodCaller(Box::new((id, (pm, meth.to_string())))),
                        _ => (),
                    }
                }
                ControlFlow::Continue(())
            }
        }
    }
}

enum Check {
    Ignore,
    Invalidate,
    Elaborate,
}

fn check_call<Ex, En>(expr_: &Expr_<Ex, En>, errs: &mut Vec<NamingPhaseError>) -> Check {
    match expr_ {
        Expr_::Call(box (
            Expr(_, fn_expr_pos, Expr_::Id(box Id(_, fn_name))),
            _,
            fn_param_exprs,
            fn_variadic_param_opt,
        )) if fn_name == sn::autoimported_functions::METH_CALLER => {
            // Raise an error if we have a variadic arg
            if let Some(Expr(_, pos, _)) = fn_variadic_param_opt {
                errs.push(NamingPhaseError::Naming(NamingError::TooFewArguments(
                    pos.clone(),
                )))
            }

            if fn_param_exprs.len() > 2 {
                errs.push(NamingPhaseError::Naming(NamingError::TooManyArguments(
                    fn_expr_pos.clone(),
                )));
                return Check::Invalidate;
            }

            match fn_param_exprs.get(0..2) {
                Some(
                    &[
                        (ParamKind::Pnormal, Expr(_, ref pos, ref expr_1)),
                        (ParamKind::Pnormal, Expr(_, _, ref expr_2)),
                    ],
                ) => {
                    // Exactly two non-inout args; ensure the second is a string literal
                    // and the first is either a string literal or a reference to `class`
                    match (expr_1, expr_2) {
                        (Expr_::String(_), Expr_::String(_)) => Check::Elaborate,
                        (Expr_::ClassConst(cc), Expr_::String(_)) => {
                            let (ClassId(_, _, class_id_), (_, mem)) =
                                cc as &(ClassId<_, _>, Pstring);
                            match class_id_ {
                                ClassId_::CI(_) if mem == sn::members::M_CLASS => Check::Elaborate,
                                _ => {
                                    errs.push(NamingPhaseError::Naming(
                                        NamingError::IllegalMethCaller(pos.clone()),
                                    ));
                                    Check::Invalidate
                                }
                            }
                        }
                        _ => {
                            errs.push(NamingPhaseError::Naming(NamingError::IllegalMethCaller(
                                pos.clone(),
                            )));
                            Check::Invalidate
                        }
                    }
                }
                Some(&[_, _]) => {
                    // We expect a string literal as the second argument and neither param
                    // can be inout; raise an error and invalidate
                    errs.push(NamingPhaseError::Naming(NamingError::IllegalMethCaller(
                        fn_expr_pos.clone(),
                    )));
                    Check::Invalidate
                }

                // We are expecting exactly two args
                Some(&[]) | Some(&[_]) => {
                    errs.push(NamingPhaseError::Naming(NamingError::TooFewArguments(
                        fn_expr_pos.clone(),
                    )));
                    Check::Invalidate
                }
                _ => {
                    errs.push(NamingPhaseError::Naming(NamingError::TooFewArguments(
                        fn_expr_pos.clone(),
                    )));
                    Check::Invalidate
                }
            }
        }
        _ => Check::Ignore,
    }
}

#[cfg(test)]
mod tests {

    use super::*;
    use crate::elab_utils;
    use crate::Transform;

    // -- Valid cases resulting in elaboration to `MethodCaller` ---------------

    #[test]
    fn test_valid_two_string_args() {
        let cfg = Config::default();
        let mut errs = Vec::default();
        let mut pass = ElabExprCallHhMethCallerPass;
        let rcvr_name = "wut";
        let meth_name = "foo";
        let mut elem: Expr_<(), ()> = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::autoimported_functions::METH_CALLER.to_string(),
                ))),
            ),
            vec![],
            vec![
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::String(rcvr_name.into())),
                ),
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::String(meth_name.into())),
                ),
            ],
            None,
        )));
        elem.transform(&cfg, &mut errs, &mut pass);

        // Expect no errors
        assert!(errs.is_empty());

        // Expect our `Expr_` to elaborate to a `MethodCaller`
        assert!(match elem {
            Expr_::MethodCaller(meth_caller) => {
                let (Id(_, x), (_, y)) = *meth_caller;
                x == rcvr_name && y == meth_name
            }
            _ => false,
        })
    }

    #[test]
    fn test_valid_class_const_string_args() {
        let cfg = Config::default();
        let mut errs = Vec::default();
        let mut pass = ElabExprCallHhMethCallerPass;
        let rcvr_name = "wut";
        let meth_name = "foo";
        let mut elem: Expr_<(), ()> = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::autoimported_functions::METH_CALLER.to_string(),
                ))),
            ),
            vec![],
            vec![
                (
                    ParamKind::Pnormal,
                    Expr(
                        (),
                        elab_utils::pos::null(),
                        Expr_::ClassConst(Box::new((
                            ClassId(
                                (),
                                elab_utils::pos::null(),
                                ClassId_::CI(Id(elab_utils::pos::null(), rcvr_name.into())),
                            ),
                            (elab_utils::pos::null(), sn::members::M_CLASS.to_string()),
                        ))),
                    ),
                ),
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::String(meth_name.into())),
                ),
            ],
            None,
        )));
        elem.transform(&cfg, &mut errs, &mut pass);

        // Expect no errors
        assert!(errs.is_empty());

        // Expect our `Expr_` to elaborate to a `MethodCaller`
        assert!(match elem {
            Expr_::MethodCaller(meth_caller) => {
                let (Id(_, x), (_, y)) = *meth_caller;
                x == rcvr_name && y == meth_name
            }
            _ => false,
        })
    }

    #[test]
    fn test_valid_with_variadic_arg() {
        let cfg = Config::default();
        let mut errs = Vec::default();
        let mut pass = ElabExprCallHhMethCallerPass;
        let rcvr_name = "wut";
        let meth_name = "foo";
        let mut elem: Expr_<(), ()> = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::autoimported_functions::METH_CALLER.to_string(),
                ))),
            ),
            vec![],
            vec![
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::String(rcvr_name.into())),
                ),
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::String(meth_name.into())),
                ),
            ],
            Some(elab_utils::expr::null()),
        )));
        elem.transform(&cfg, &mut errs, &mut pass);

        // Expect `TooFewArgs` error from variadic param
        let too_few_args_err_opt = errs.pop();
        assert!(matches!(
            too_few_args_err_opt,
            Some(NamingPhaseError::Naming(NamingError::TooFewArguments(_)))
        ));

        // Expect our `Expr_` to elaborate to a `MethodCaller`
        assert!(match elem {
            Expr_::MethodCaller(meth_caller) => {
                let (Id(_, x), (_, y)) = *meth_caller;
                x == rcvr_name && y == meth_name
            }
            _ => false,
        })
    }
    // -- Invalid cases resulting in elaboration to `Invalid(Some(Call(..))` ---

    #[test]
    fn test_invalid_arg_type() {
        let cfg = Config::default();
        let mut errs = Vec::default();
        let mut pass = ElabExprCallHhMethCallerPass;
        let meth_name = "foo";
        let mut elem: Expr_<(), ()> = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::autoimported_functions::METH_CALLER.to_string(),
                ))),
            ),
            vec![],
            vec![
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::Null),
                ),
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::String(meth_name.into())),
                ),
            ],
            None,
        )));
        elem.transform(&cfg, &mut errs, &mut pass);

        let illegal_err_opt = errs.pop();
        assert!(matches!(
            illegal_err_opt,
            Some(NamingPhaseError::Naming(NamingError::IllegalMethCaller(..)))
        ));

        // Expect our original expression to be wrapped in `Invalid`
        assert!(match elem {
            Expr_::Invalid(expr) => {
                if let Some(Expr(_, _, Expr_::Call(cc))) = *expr {
                    if let (Expr(_, _, Expr_::Id(id)), _, _, _) = *cc {
                        let Id(_, nm) = *id;
                        nm == sn::autoimported_functions::METH_CALLER
                    } else {
                        false
                    }
                } else {
                    false
                }
            }
            _ => false,
        })
    }

    #[test]
    fn test_invalid_param_kind() {
        let cfg = Config::default();
        let mut errs = Vec::default();
        let mut pass = ElabExprCallHhMethCallerPass;
        let rcvr_name = "wut";
        let meth_name = "foo";
        let mut elem: Expr_<(), ()> = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::autoimported_functions::METH_CALLER.to_string(),
                ))),
            ),
            vec![],
            vec![
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::String(rcvr_name.into())),
                ),
                (
                    ParamKind::Pinout(elab_utils::pos::null()),
                    Expr((), elab_utils::pos::null(), Expr_::String(meth_name.into())),
                ),
            ],
            None,
        )));
        elem.transform(&cfg, &mut errs, &mut pass);

        let illegal_err_opt = errs.pop();
        assert!(matches!(
            illegal_err_opt,
            Some(NamingPhaseError::Naming(NamingError::IllegalMethCaller(..)))
        ));

        // Expect our original expression to be wrapped in `Invalid`
        assert!(match elem {
            Expr_::Invalid(expr) => {
                if let Some(Expr(_, _, Expr_::Call(cc))) = *expr {
                    if let (Expr(_, _, Expr_::Id(id)), _, _, _) = *cc {
                        let Id(_, nm) = *id;
                        nm == sn::autoimported_functions::METH_CALLER
                    } else {
                        false
                    }
                } else {
                    false
                }
            }
            _ => false,
        })
    }

    #[test]
    fn test_too_few_args() {
        let cfg = Config::default();
        let mut errs = Vec::default();
        let mut pass = ElabExprCallHhMethCallerPass;
        let rcvr_name = "wut";
        let mut elem: Expr_<(), ()> = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::autoimported_functions::METH_CALLER.to_string(),
                ))),
            ),
            vec![],
            vec![(
                ParamKind::Pnormal,
                Expr((), elab_utils::pos::null(), Expr_::String(rcvr_name.into())),
            )],
            None,
        )));
        elem.transform(&cfg, &mut errs, &mut pass);

        let too_few_args_err_opt = errs.pop();
        assert!(matches!(
            too_few_args_err_opt,
            Some(NamingPhaseError::Naming(NamingError::TooFewArguments(_)))
        ));

        // Expect our original expression to be wrapped in `Invalid`
        assert!(match elem {
            Expr_::Invalid(expr) => {
                if let Some(Expr(_, _, Expr_::Call(cc))) = *expr {
                    if let (Expr(_, _, Expr_::Id(id)), _, _, _) = *cc {
                        let Id(_, nm) = *id;
                        nm == sn::autoimported_functions::METH_CALLER
                    } else {
                        false
                    }
                } else {
                    false
                }
            }
            _ => false,
        })
    }

    #[test]
    fn test_too_many_args() {
        let cfg = Config::default();
        let mut errs = Vec::default();
        let mut pass = ElabExprCallHhMethCallerPass;
        let rcvr_name = "wut";
        let meth_name = "foo";
        let mut elem: Expr_<(), ()> = Expr_::Call(Box::new((
            Expr(
                (),
                elab_utils::pos::null(),
                Expr_::Id(Box::new(Id(
                    elab_utils::pos::null(),
                    sn::autoimported_functions::METH_CALLER.to_string(),
                ))),
            ),
            vec![],
            vec![
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::String(rcvr_name.into())),
                ),
                (
                    ParamKind::Pnormal,
                    Expr((), elab_utils::pos::null(), Expr_::String(meth_name.into())),
                ),
                (ParamKind::Pnormal, elab_utils::expr::null()),
            ],
            None,
        )));
        elem.transform(&cfg, &mut errs, &mut pass);

        let too_many_args_err_opt = errs.pop();
        assert!(matches!(
            too_many_args_err_opt,
            Some(NamingPhaseError::Naming(NamingError::TooManyArguments(_)))
        ));

        // Expect our original expression to be wrapped in `Invalid`
        assert!(match elem {
            Expr_::Invalid(expr) => {
                if let Some(Expr(_, _, Expr_::Call(cc))) = *expr {
                    if let (Expr(_, _, Expr_::Id(id)), _, _, _) = *cc {
                        let Id(_, nm) = *id;
                        nm == sn::autoimported_functions::METH_CALLER
                    } else {
                        false
                    }
                } else {
                    false
                }
            }
            _ => false,
        })
    }
}
