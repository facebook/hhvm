// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Class_;
use nast::Method_;

use crate::prelude::*;

#[derive(Clone, Default)]
pub struct ValidateMethodPrivateFinalPass {
    in_trait: bool,
}

impl Pass for ValidateMethodPrivateFinalPass {
    fn on_ty_class__top_down(&mut self, _env: &Env, elem: &mut Class_) -> ControlFlow<()> {
        self.in_trait = elem.kind.is_ctrait();
        ControlFlow::Continue(())
    }
    fn on_ty_method__bottom_up(&mut self, env: &Env, elem: &mut Method_) -> ControlFlow<()> {
        if !self.in_trait && elem.final_ && elem.visibility.is_private() {
            env.emit_error(NastCheckError::PrivateAndFinal(elem.name.pos().clone()))
        }
        ControlFlow::Continue(())
    }
}
