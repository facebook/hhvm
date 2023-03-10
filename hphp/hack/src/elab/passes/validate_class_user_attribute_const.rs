// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::naming_phase_error::ExperimentalFeature;
use oxidized::nast::Class_;
use oxidized::nast::Pos;
use oxidized::nast::UserAttributes;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateClassUserAttributeConstPass;

impl Pass for ValidateClassUserAttributeConstPass {
    fn on_ty_class__bottom_up(&mut self, env: &Env, elem: &mut Class_) -> ControlFlow<()> {
        if !env.const_attribute() {
            // Disallow `__Const` attribute unless typechecker option is enabled
            check_const(env, elem.name.pos(), &elem.user_attributes);
            elem.vars
                .iter()
                .for_each(|cv| check_const(env, elem.name.pos(), &cv.user_attributes));
        }
        ControlFlow::Continue(())
    }
}

fn check_const(env: &Env, pos: &Pos, attrs: &UserAttributes) {
    if attrs
        .0
        .iter()
        .any(|ua| ua.name.name() == sn::user_attributes::CONST)
    {
        env.emit_error(ExperimentalFeature::ConstAttr(pos.clone()))
    }
}
