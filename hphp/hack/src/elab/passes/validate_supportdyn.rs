// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust as sn;
use nast::Hint_;
use nast::Id;
use oxidized::naming_phase_error::ExperimentalFeature;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateSupportDynPass;

impl Pass for ValidateSupportDynPass {
    fn on_ty_hint__bottom_up(&mut self, env: &Env, hint_: &mut Hint_) -> ControlFlow<()> {
        if env.is_hhi() || env.is_systemlib() || env.supportdynamic_type_hint_enabled() {
            return Continue(());
        }
        match hint_ {
            Hint_::Happly(Id(pos, ty_name), _) if ty_name == sn::classes::SUPPORT_DYN => {
                env.emit_error(NamingPhaseError::ExperimentalFeature(
                    ExperimentalFeature::Supportdyn(pos.clone()),
                ));
            }
            _ => (),
        }

        Continue(())
    }
}
