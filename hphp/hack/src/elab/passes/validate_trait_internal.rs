// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Class_;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateTraitInternalPass;

impl Pass for ValidateTraitInternalPass {
    fn on_ty_class__bottom_up(&mut self, env: &Env, class_: &mut Class_) -> ControlFlow<()> {
        if class_.module.is_some() && class_.kind.is_ctrait() && !class_.internal {
            class_
                .methods
                .iter()
                .filter(|m| m.visibility.is_internal())
                .for_each(|m| {
                    env.emit_error(NastCheckError::InternalMemberInsidePublicTrait {
                        member_pos: m.span.clone(),
                        trait_pos: class_.name.pos().clone(),
                    })
                });

            class_
                .vars
                .iter()
                .filter(|cv| cv.visibility.is_internal())
                .for_each(|cv| {
                    env.emit_error(NastCheckError::InternalMemberInsidePublicTrait {
                        member_pos: cv.span.clone(),
                        trait_pos: class_.name.pos().clone(),
                    })
                });
        }
        ControlFlow::Continue(())
    }
}
