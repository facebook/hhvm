// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::aast_defs::Tprim;
use oxidized::ast::Id;
use oxidized::naming_error::NamingError;
use oxidized::naming_phase_error::NamingPhaseError;
use pos;
use special_names;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateExprCastPass;

impl Pass for ValidateExprCastPass {
    fn on_ty_expr_bottom_up<Ex: Default, En>(
        &mut self,
        elem: &mut Expr<Ex, En>,
        _cfg: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()> {
        if let Expr(_, _, Expr_::Cast(target)) = elem {
            let (Hint(pos, hint_), _) = target as &_;
            match hint_ as &_ {
                Hint_::Hprim(Tprim::Tint | Tprim::Tbool | Tprim::Tfloat | Tprim::Tstring) => {
                    return ControlFlow::Continue(());
                }
                Hint_::Happly(Id(_, tycon_nm), _)
                    if (pos::TypeName(tycon_nm.into()) == *special_names::collections::cDict)
                        || (pos::TypeName(tycon_nm.into())
                            == *special_names::collections::cVec) =>
                {
                    return ControlFlow::Continue(());
                }
                Hint_::HvecOrDict(_, _) => return ControlFlow::Continue(()),
                Hint_::Hany =>
                // We end up with a `Hany` when we have an arity error for
                // `dict`/`vec` we don't error on this case to preserve
                // behaviour.
                {
                    return ControlFlow::Continue(());
                }
                _ => {
                    let pos = pos.clone();
                    // Record the error and break.
                    errs.push(NamingPhaseError::Naming(NamingError::ObjectCast(pos)));
                    return ControlFlow::Break(());
                }
            }
        }
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use super::*;
    use crate::elab_utils;
    use crate::Transform;

    #[test]
    fn test_invalid_cast() {
        let cfg = Config::default();
        let mut errs = Vec::default();
        let mut pass = ValidateExprCastPass;
        let bad_hint_for_cast = Hint(elab_utils::pos::null(), Box::new(Hint_::Hthis));
        let mut elem: Expr<(), ()> = elab_utils::expr::from_expr_(Expr_::Cast(Box::new((
            bad_hint_for_cast,
            elab_utils::expr::null(),
        ))));
        elem.transform(&cfg, &mut errs, &mut pass);
        assert_eq!(errs.len(), 1);
    }
}
