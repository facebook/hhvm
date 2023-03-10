// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::naming_phase_error::ExperimentalFeature;
use oxidized::nast::ClassVar;
use oxidized::nast::Pos;
use oxidized::nast::UserAttributes;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateClassVarUserAttributeConstPass;

impl Pass for ValidateClassVarUserAttributeConstPass {
    fn on_ty_class_var_bottom_up(&mut self, elem: &mut ClassVar, env: &Env) -> ControlFlow<()> {
        if !env.const_static_props() && elem.is_static {
            check_const(env, elem.id.pos(), &elem.user_attributes)
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
        env.emit_error(ExperimentalFeature::ConstStaticProp(pos.clone()))
    }
}
