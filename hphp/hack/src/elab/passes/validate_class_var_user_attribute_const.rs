// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::ClassVar;
use nast::Pos;
use nast::UserAttributes;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateClassVarUserAttributeConstPass;

impl Pass for ValidateClassVarUserAttributeConstPass {
    fn on_ty_class_var_bottom_up(&mut self, env: &Env, elem: &mut ClassVar) -> ControlFlow<()> {
        if !env.const_static_props() && elem.is_static {
            check_const(env, elem.id.pos(), &elem.user_attributes)
        }
        Continue(())
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
