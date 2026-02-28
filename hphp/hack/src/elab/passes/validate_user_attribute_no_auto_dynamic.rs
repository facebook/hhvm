// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use nast::Class_;
use nast::Fun_;
use nast::Method_;
use nast::UserAttribute;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateUserAttributeNoAutoDynamic;

impl Pass for ValidateUserAttributeNoAutoDynamic {
    fn on_ty_fun__bottom_up(&mut self, env: &Env, elem: &mut Fun_) -> ControlFlow<()> {
        if !env.is_hhi() && !env.no_auto_dynamic_enabled() {
            check_no_auto_dynamic(&elem.user_attributes, env);
        }
        Continue(())
    }

    fn on_ty_method__bottom_up(&mut self, env: &Env, elem: &mut Method_) -> ControlFlow<()> {
        if !env.is_hhi() && !env.no_auto_dynamic_enabled() {
            check_no_auto_dynamic(&elem.user_attributes, env);
        }
        Continue(())
    }

    fn on_ty_class__bottom_up(&mut self, env: &Env, elem: &mut Class_) -> ControlFlow<()> {
        if !env.is_hhi() && !env.no_auto_dynamic_enabled() {
            check_no_auto_dynamic(&elem.user_attributes, env);
        }
        Continue(())
    }
}

fn check_no_auto_dynamic(user_attrs: &[UserAttribute], env: &Env) {
    user_attrs
        .iter()
        .find(|ua| ua.name.name() == sn::user_attributes::NO_AUTO_DYNAMIC)
        .iter()
        .for_each(|ua| {
            env.emit_error(NastCheckError::AttributeNoAutoDynamic(
                ua.name.pos().clone(),
            ))
        })
}
