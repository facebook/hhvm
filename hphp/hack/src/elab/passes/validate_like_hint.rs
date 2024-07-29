// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Hint;
use nast::Hint_;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateLikeHintPass {}

impl Pass for ValidateLikeHintPass {
    fn on_ty_hint_top_down(&mut self, env: &Env, hint: &mut Hint) -> ControlFlow<()> {
        match hint {
            Hint(pos, box Hint_::Hlike(_)) if !env.is_hhi() && !env.like_type_hints_enabled() => {
                env.emit_error(ExperimentalFeature::LikeType(pos.clone()));
            }
            _ => (),
        }
        Continue(())
    }
}
