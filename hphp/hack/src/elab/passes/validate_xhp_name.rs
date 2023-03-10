// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::naming_error::NamingError;
use oxidized::nast::Hint_;
use oxidized::nast::Id;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateXhpNamePass;

impl Pass for ValidateXhpNamePass {
    fn on_ty_hint__top_down(&mut self, hint_: &mut Hint_, env: &Env) -> ControlFlow<()> {
        match hint_ {
            // "some common Xhp screw ups"
            Hint_::Happly(Id(pos, name), _) if ["Xhp", ":Xhp", "XHP"].contains(&name.as_str()) => {
                env.emit_error(NamingError::DisallowedXhpType {
                    pos: pos.clone(),
                    ty_name: name.clone(),
                })
            }
            _ => (),
        }
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::nast::Pos;

    use super::*;
    use crate::transform::Transform;

    #[test]
    fn test_bad_xhp_name() {
        let env = Env::default();
        let mut pass = ValidateXhpNamePass;

        let mut hint_ = Hint_::Happly(Id(Pos::NONE, "Xhp".to_string()), vec![]);

        hint_.transform(&env, &mut pass);
        assert!(matches!(
            env.into_errors().as_slice(),
            &[NamingPhaseError::Naming(
                NamingError::DisallowedXhpType { .. }
            )]
        ))
    }
}
