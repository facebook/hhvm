// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::tast::Pos;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabHintHsoftPass;

impl Pass for ElabHintHsoftPass {
    type Cfg = Config;
    type Err = NamingPhaseError;

    fn on_ty_hint_top_down(
        &mut self,
        elem: &mut Hint,
        cfg: &Self::Cfg,
        _errs: &mut Vec<Self::Err>,
    ) -> std::ops::ControlFlow<(), ()> {
        let Hint(_, hint_) = elem;
        if let Hint_::Hsoft(inner) = hint_ as &mut Hint_ {
            if cfg.soft_as_like() {
                // Replace `Hsoft` with `Hlike` retaining the original position
                // (pos, Hsoft(hint)) ==> (pos, Hlike(hint))
                let herr = Hint(Pos::make_none(), Box::new(Hint_::Herr));
                let inner_hint = std::mem::replace(inner, herr);
                **hint_ = Hint_::Hlike(inner_hint)
            } else {
                // Drop the surrounding `Hsoft` and use the inner Hint_
                // whilst maintaining positions
                // (pos, Hsoft(_, hint_)) ==> (pos, hint_)
                let Hint(_, inner_hint_) = inner;
                let herr_ = Hint_::Herr;
                let inner_hint_ = std::mem::replace(inner_hint_ as &mut Hint_, herr_);
                **hint_ = inner_hint_
            }
        }
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use oxidized::aast_defs::Hint;
    use oxidized::aast_defs::Hint_;
    use oxidized::tast::Pos;

    use super::*;
    use crate::Transform;

    #[test]
    fn test() {
        let mut errs = Vec::default();
        let mut pass = ElabHintHsoftPass;

        let mut elem1: Hint = Hint(
            Pos::make_none(),
            Box::new(Hint_::Hsoft(Hint(
                Pos::make_none(),
                Box::new(Hint_::Hdynamic),
            ))),
        );

        let mut elem2 = elem1.clone();

        // Transform elem1 without `soft_as_like` set
        // expect Hdynamic
        let mut cfg = Config::default();
        elem1.transform(&cfg, &mut errs, &mut pass);
        assert!(matches!(*elem1.1, Hint_::Hdynamic));

        // Transform elem2 with `soft_as_like` set
        // expect Hlike(_,Hdynamic)
        cfg = Config::SOFT_AS_LIKE;
        elem2.transform(&cfg, &mut errs, &mut pass);
        assert!(matches!(&*elem2.1, Hint_::Hlike(_)));
        assert!(match &*elem2.1 {
            Hint_::Hlike(inner) => matches!(*inner.1, Hint_::Hdynamic),
            _ => false,
        })
    }
}
