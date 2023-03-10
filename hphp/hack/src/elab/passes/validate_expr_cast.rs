// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::aast_defs::Tprim::*;
use oxidized::ast::Id;
use oxidized::naming_error::NamingError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateExprCastPass;

impl Pass for ValidateExprCastPass {
    fn on_ty_expr__bottom_up<Ex: Default, En>(
        &mut self,
        expr: &mut Expr_<Ex, En>,
        cfg: &Config,
    ) -> ControlFlow<(), ()> {
        match &*expr {
            Expr_::Cast(box (Hint(_, box Hint_::Hprim(Tint | Tbool | Tfloat | Tstring)), _)) => {
                ControlFlow::Continue(())
            }
            Expr_::Cast(box (Hint(_, box Hint_::Happly(Id(_, tycon_nm), _)), _))
                if tycon_nm == sn::collections::DICT || tycon_nm == sn::collections::VEC =>
            {
                ControlFlow::Continue(())
            }
            Expr_::Cast(box (Hint(_, box Hint_::HvecOrDict(_, _)), _)) => ControlFlow::Continue(()),
            // We end up with a `Hany` when we have an arity error for
            // `dict`/`vec`--we don't error on this case to preserve behaviour.
            Expr_::Cast(box (Hint(_, box Hint_::Hany), _)) => ControlFlow::Continue(()),
            Expr_::Cast(box (Hint(p, _), _)) => {
                cfg.emit_error(NamingError::ObjectCast(p.clone()));
                ControlFlow::Break(())
            }
            _ => ControlFlow::Continue(()),
        }
    }
}

#[cfg(test)]
mod tests {
    use oxidized::ast::Expr;
    use oxidized::tast::Pos;

    use super::*;
    use crate::elab_utils;
    use crate::Transform;

    #[test]
    fn test_invalid_cast() {
        let mut expr: Expr = elab_utils::expr::from_expr_(Expr_::Cast(Box::new((
            Hint(Pos::NONE, Box::new(Hint_::Hthis)),
            elab_utils::expr::null(),
        ))));
        let cfg = Config::default();
        expr.transform(&cfg, &mut ValidateExprCastPass);
        assert_eq!(cfg.into_errors().len(), 1);
    }
}
