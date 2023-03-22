// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Class_;

use crate::prelude::*;

#[derive(Clone, Default)]
pub struct ValidateEnumSupertypingPass;

impl Pass for ValidateEnumSupertypingPass {
    fn on_ty_class__bottom_up(&mut self, env: &Env, class: &mut Class_) -> ControlFlow<()> {
        if !env.enable_enum_supertyping() {
            if matches!(&class.enum_, Some(e) if !e.includes.is_empty() && !class.kind.is_cenum_class())
            {
                env.emit_error(NastCheckError::EnumSupertypingReservedSyntax {
                    pos: class.name.pos().clone(),
                });
            }
        }
        Continue(())
    }
}
