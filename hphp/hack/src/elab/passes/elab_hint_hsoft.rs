// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Hint;
use nast::Hint_;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ElabHintHsoftPass;

impl Pass for ElabHintHsoftPass {
    fn on_ty_hint_top_down(&mut self, env: &Env, elem: &mut Hint) -> std::ops::ControlFlow<()> {
        let Hint(_, hint_) = elem;
        if let Hint_::Hsoft(inner) = hint_ as &mut Hint_ {
            if env.soft_as_like() {
                // Replace `Hsoft` with `Hlike` retaining the original position
                // (pos, Hsoft(hint)) ==> (pos, Hlike(hint))
                **hint_ = Hint_::Hlike(inner.clone())
            } else {
                // Drop the surrounding `Hsoft` and use the inner Hint_
                // whilst maintaining positions
                // (pos, Hsoft(_, hint_)) ==> (pos, hint_)
                let Hint(_, inner_hint_) = inner;
                let herr_ = Hint_::Hmixed;
                let inner_hint_ = std::mem::replace(inner_hint_ as &mut Hint_, herr_);
                **hint_ = inner_hint_
            }
        }
        Continue(())
    }
}

#[cfg(test)]
mod tests {

    use nast::Hint;
    use nast::Hint_;
    use nast::Pos;
    use oxidized::typechecker_options::TypecheckerOptions;

    use super::*;
    use crate::env::ProgramSpecificOptions;

    #[test]
    fn test() {
        let mut pass = ElabHintHsoftPass;

        let mut elem1: Hint = Hint(
            Pos::NONE,
            Box::new(Hint_::Hsoft(Hint(Pos::NONE, Box::new(Hint_::Hdynamic)))),
        );
        let mut elem2 = elem1.clone();

        // Transform `elem1` without flag `SOFT_AS_LIKE` set and expect
        // `Hdynamic`.
        let tco = TypecheckerOptions {
            ..Default::default()
        };
        let pso = ProgramSpecificOptions {
            ..Default::default()
        };
        let env = Env::new(&tco, &pso);
        elem1.transform(&env, &mut pass);
        assert!(matches!(*elem1.1, Hint_::Hdynamic));

        // Transform `elem2` with flag `SOFT_AS_LIKE` set & expect `Hlike(_,
        // Hdynamic)`.
        let tco = TypecheckerOptions {
            po_interpret_soft_types_as_like_types: true,
            ..Default::default()
        };
        let pso = ProgramSpecificOptions {
            ..Default::default()
        };
        let env = Env::new(&tco, &pso);
        elem2.transform(&env, &mut pass);
        assert!(matches!(&*elem2.1, Hint_::Hlike(_)));
        assert!(match &*elem2.1 {
            Hint_::Hlike(inner) => matches!(*inner.1, Hint_::Hdynamic),
            _ => false,
        })
    }
}
