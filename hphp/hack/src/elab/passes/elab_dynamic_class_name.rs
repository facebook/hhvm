// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast::ClassGetExpr;
use oxidized::aast_defs::ClassId;
use oxidized::aast_defs::ClassId_;
use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::FunctionPtrId;
use oxidized::aast_defs::PropOrMethod;
use oxidized::ast_defs::Id;
use oxidized::naming_error::NamingError;

use crate::config::Config;
use crate::elab_utils;
use crate::Pass;

#[derive(Copy, Clone, Default)]
pub struct ElabDynamicClassNamePass;

impl Pass for ElabDynamicClassNamePass {
    fn on_ty_expr_bottom_up<Ex, En>(
        &mut self,
        elem: &mut Expr<Ex, En>,
        cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        let invalid = |expr_: &mut Expr_<_, _>| {
            let inner_expr_ = std::mem::replace(expr_, Expr_::Null);
            let inner_expr = elab_utils::expr::from_expr__with_pos_(elem.1.clone(), inner_expr_);
            *expr_ = Expr_::Invalid(Box::new(Some(inner_expr)));
            ControlFlow::Break(())
        };

        match &mut elem.2 {
            Expr_::New(box (class_id, _, _, _, _)) if is_dynamic(class_id) => {
                cfg.emit_error(NamingError::DynamicNewInStrictMode(class_id.1.clone()));
                class_id.2 = ClassId_::CI(Id(class_id.1.clone(), sn::classes::UNKNOWN.to_string()));
                ControlFlow::Continue(())
            }
            Expr_::ClassGet(box (class_id, ClassGetExpr::CGstring(..), _))
                if !is_dynamic(class_id) =>
            {
                ControlFlow::Continue(())
            }
            Expr_::ClassGet(box (
                class_id,
                ClassGetExpr::CGexpr(cg_expr),
                PropOrMethod::IsMethod,
            )) if !is_dynamic(class_id) => {
                cfg.emit_error(NamingError::DynamicMethodAccess(cg_expr.1.clone()));
                ControlFlow::Continue(())
            }
            Expr_::ClassGet(box (
                ClassId(_, _, ClassId_::CIexpr(ci_expr)),
                ClassGetExpr::CGstring(..)
                | ClassGetExpr::CGexpr(Expr(_, _, Expr_::Lvar(..) | Expr_::This)),
                _,
            )) => {
                cfg.emit_error(NamingError::DynamicClassNameInStrictMode(ci_expr.1.clone()));
                invalid(&mut elem.2)
            }
            Expr_::ClassGet(box (
                ClassId(_, _, ClassId_::CIexpr(ci_expr)),
                ClassGetExpr::CGexpr(cg_expr),
                _,
            )) => {
                cfg.emit_error(NamingError::DynamicClassNameInStrictMode(ci_expr.1.clone()));
                cfg.emit_error(NamingError::DynamicClassNameInStrictMode(cg_expr.1.clone()));
                invalid(&mut elem.2)
            }
            Expr_::ClassGet(box (_, ClassGetExpr::CGexpr(cg_expr), _)) => {
                cfg.emit_error(NamingError::DynamicClassNameInStrictMode(cg_expr.1.clone()));
                invalid(&mut elem.2)
            }
            Expr_::FunctionPointer(box (FunctionPtrId::FPClassConst(class_id, _), _))
                if is_dynamic(class_id) =>
            {
                invalid(&mut elem.2)
            }
            Expr_::ClassConst(box (class_id, _)) if is_dynamic(class_id) => invalid(&mut elem.2),
            _ => ControlFlow::Continue(()),
        }
    }
}

fn is_dynamic<Ex, En>(class_id: &ClassId<Ex, En>) -> bool {
    match &class_id.2 {
        ClassId_::CIparent
        | ClassId_::CIself
        | ClassId_::CIstatic
        | ClassId_::CI(..)
        | ClassId_::CIexpr(Expr(_, _, Expr_::Lvar(..) | Expr_::This | Expr_::Dollardollar(..))) => {
            false
        }
        ClassId_::CIexpr(_) => true,
    }
}

#[cfg(test)]
mod tests {

    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::tast::Pos;

    use super::*;
    use crate::Transform;

    fn mk_dynamic_class_id() -> ClassId<(), ()> {
        ClassId(
            (),
            Pos::default(),
            ClassId_::CIexpr(elab_utils::expr::null()),
        )
    }

    fn mk_non_dynamic_class_id() -> ClassId<(), ()> {
        ClassId((), Pos::default(), ClassId_::CIself)
    }

    // -- in `New` expressions -------------------------------------------------

    #[test]
    fn test_new_valid() {
        let cfg = Config::default();

        let mut pass = ElabDynamicClassNamePass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::default(),
            Expr_::New(Box::new((
                mk_non_dynamic_class_id(),
                Default::default(),
                Default::default(),
                Default::default(),
                Default::default(),
            ))),
        );
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            expr_,
            Expr_::New(box (ClassId(_, _, ClassId_::CIself), _, _, _, _))
        ));
    }

    #[test]
    fn test_new_invalid() {
        let cfg = Config::default();

        let mut pass = ElabDynamicClassNamePass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::default(),
            Expr_::New(Box::new((
                mk_dynamic_class_id(),
                Default::default(),
                Default::default(),
                Default::default(),
                Default::default(),
            ))),
        );
        elem.transform(&cfg, &mut pass);

        let err_opt = cfg.into_errors().pop();
        assert!(matches!(
            err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::DynamicNewInStrictMode(..)
            ))
        ));
        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            expr_,
            Expr_::New(box (ClassId(_, _, ClassId_::CI(..)), _, _, _, _))
        ))
    }

    // -- in `FunctionPointer` expressions -------------------------------------
    #[test]
    fn test_function_pointer_valid() {
        let cfg = Config::default();

        let mut pass = ElabDynamicClassNamePass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::default(),
            Expr_::FunctionPointer(Box::new((
                FunctionPtrId::FPClassConst(mk_non_dynamic_class_id(), Default::default()),
                Default::default(),
            ))),
        );
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            expr_,
            Expr_::FunctionPointer(box (
                FunctionPtrId::FPClassConst(ClassId(_, _, ClassId_::CIself), _),
                _
            ))
        ));
    }

    #[test]
    fn test_function_pointer_invalid() {
        let cfg = Config::default();

        let mut pass = ElabDynamicClassNamePass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::default(),
            Expr_::FunctionPointer(Box::new((
                FunctionPtrId::FPClassConst(mk_dynamic_class_id(), Default::default()),
                Default::default(),
            ))),
        );
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        let Expr(_, _, expr_) = elem;
        assert!(matches!(expr_, Expr_::Invalid(_)))
    }

    // -- in `ClassConst` expressions ------------------------------------------

    #[test]
    fn test_class_const_valid() {
        let cfg = Config::default();

        let mut pass = ElabDynamicClassNamePass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::default(),
            Expr_::ClassConst(Box::new((mk_non_dynamic_class_id(), Default::default()))),
        );
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        assert!(matches!(
            elem,
            Expr(
                _,
                _,
                Expr_::ClassConst(box (ClassId(_, _, ClassId_::CIself), _))
            )
        ))
    }

    #[test]
    fn test_class_const_invalid() {
        let cfg = Config::default();

        let mut pass = ElabDynamicClassNamePass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::default(),
            Expr_::ClassConst(Box::new((mk_dynamic_class_id(), Default::default()))),
        );
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        assert!(matches!(
            elem,
            Expr(
                _,
                _,
                Expr_::Invalid(box Some(Expr(
                    _,
                    _,
                    Expr_::ClassConst(box (ClassId(_, _, ClassId_::CIexpr(..)), _))
                )))
            )
        ))
    }

    // -- in `ClassGet` expressions --------------------------------------------
    #[test]
    fn test_class_get_cg_string_valid() {
        let cfg = Config::default();

        let mut pass = ElabDynamicClassNamePass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::default(),
            Expr_::ClassGet(Box::new((
                mk_non_dynamic_class_id(),
                ClassGetExpr::CGstring(Default::default()),
                PropOrMethod::IsProp,
            ))),
        );
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        assert!(matches!(
            elem,
            Expr(
                _,
                _,
                Expr_::ClassGet(box (ClassId(_, _, ClassId_::CIself), _, _))
            )
        ))
    }

    #[test]
    fn test_class_get_cg_string_invalid() {
        let cfg = Config::default();

        let mut pass = ElabDynamicClassNamePass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::default(),
            Expr_::ClassGet(Box::new((
                mk_dynamic_class_id(),
                ClassGetExpr::CGstring(Default::default()),
                PropOrMethod::IsProp,
            ))),
        );
        elem.transform(&cfg, &mut pass);

        let err_opt = cfg.into_errors().pop();
        assert!(matches!(
            err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::DynamicClassNameInStrictMode(..)
            ))
        ));
        assert!(matches!(elem, Expr(_, _, Expr_::Invalid(_))))
    }

    #[test]
    fn test_class_get_cg_expr_invalid() {
        let cfg = Config::default();

        let mut pass = ElabDynamicClassNamePass;
        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::default(),
            Expr_::ClassGet(Box::new((
                mk_dynamic_class_id(),
                ClassGetExpr::CGexpr(elab_utils::expr::null()),
                PropOrMethod::IsProp,
            ))),
        );
        elem.transform(&cfg, &mut pass);

        assert!(matches!(
            cfg.into_errors().as_slice(),
            [
                NamingPhaseError::Naming(NamingError::DynamicClassNameInStrictMode(..)),
                NamingPhaseError::Naming(NamingError::DynamicClassNameInStrictMode(..))
            ]
        ));
        assert!(matches!(elem, Expr(_, _, Expr_::Invalid(_))))
    }
}
