// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use nast::FunDef;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateUserAttributeEntryPointPass;

impl Pass for ValidateUserAttributeEntryPointPass {
    // Ban arguments on functions with the __EntryPoint attribute.
    fn on_ty_fun_def_bottom_up(&mut self, env: &Env, elem: &mut FunDef) -> ControlFlow<()> {
        if elem
            .fun
            .user_attributes
            .iter()
            .any(|ua| ua.name.name() == sn::user_attributes::ENTRY_POINT)
        {
            match elem.fun.params.as_slice() {
                [] => (),
                [param, ..] => {
                    env.emit_error(NastCheckError::EntrypointArguments(param.pos.clone()))
                }
            }
            if let Some(param) = elem.fun.params.iter().find(|p| p.is_variadic) {
                env.emit_error(NastCheckError::EntrypointArguments(param.pos.clone()))
            }
            match elem.tparams.as_slice() {
                [] => (),
                [tparam, ..] => env.emit_error(NastCheckError::EntrypointArguments(
                    tparam.name.pos().clone(),
                )),
            }
        }
        ControlFlow::Continue(())
    }
}
