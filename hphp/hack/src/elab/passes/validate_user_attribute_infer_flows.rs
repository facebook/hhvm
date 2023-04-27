// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use nast::Fun_;
use nast::Method_;
use nast::UserAttribute;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateUserAttributeInferFlowsPass;

impl Pass for ValidateUserAttributeInferFlowsPass {
    fn on_ty_fun__bottom_up(&mut self, env: &Env, elem: &mut Fun_) -> ControlFlow<(), ()> {
        if !env.infer_flows() {
            check_ifc_infer_flows(&elem.user_attributes, env)
        }
        ControlFlow::Continue(())
    }

    fn on_ty_method__bottom_up(&mut self, env: &Env, elem: &mut Method_) -> ControlFlow<(), ()> {
        if !env.infer_flows() {
            check_ifc_infer_flows(&elem.user_attributes, env)
        }
        ControlFlow::Continue(())
    }
}

fn check_ifc_infer_flows(user_attrs: &[UserAttribute], env: &Env) {
    if let Some(ua) = user_attrs
        .iter()
        .find(|ua| ua.name.name() == sn::user_attributes::INFERFLOWS)
    {
        env.emit_error(ExperimentalFeature::IFCInferFlows(ua.name.pos().clone()))
    }
}
