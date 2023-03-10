// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::aast_defs::Hint_;
use oxidized::ast::Id;
use oxidized::naming_error::NamingError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateXhpNamePass;

impl Pass for ValidateXhpNamePass {
    fn on_ty_hint__top_down(&mut self, hint_: &mut Hint_, cfg: &Config) -> ControlFlow<(), ()> {
        match hint_ {
            // "some common Xhp screw ups"
            Hint_::Happly(Id(pos, name), _) if ["Xhp", ":Xhp", "XHP"].contains(&name.as_str()) => {
                cfg.emit_error(NamingError::DisallowedXhpType {
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

    use oxidized::aast_defs::Pos;
    use oxidized::naming_phase_error::NamingPhaseError;

    use super::*;
    use crate::transform::Transform;

    #[test]
    fn test_bad_xhp_name() {
        let cfg = Config::default();
        let mut pass = ValidateXhpNamePass;

        let mut hint_ = Hint_::Happly(Id(Pos::NONE, "Xhp".to_string()), vec![]);

        hint_.transform(&cfg, &mut pass);
        assert!(matches!(
            cfg.into_errors().as_slice(),
            &[NamingPhaseError::Naming(
                NamingError::DisallowedXhpType { .. }
            )]
        ))
    }
}
