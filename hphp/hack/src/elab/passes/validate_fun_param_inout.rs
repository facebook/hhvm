// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::FunDef;
use nast::FunParam;
use nast::Id;
use nast::Method_;
use nast::ParamKind;
use nast::UserAttributes;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateFunParamInoutPass;

impl Pass for ValidateFunParamInoutPass {
    fn on_ty_fun_def_bottom_up(&mut self, env: &Env, elem: &mut FunDef) -> ControlFlow<()> {
        check_params(env, &elem.name, &elem.fun.user_attributes, &elem.fun.params);
        Continue(())
    }
    fn on_ty_method__bottom_up(&mut self, env: &Env, elem: &mut Method_) -> ControlFlow<()> {
        check_params(env, &elem.name, &elem.user_attributes, &elem.params);
        Continue(())
    }
}

fn check_params(env: &Env, id: &Id, attrs: &UserAttributes, params: &[FunParam]) {
    let in_as_set_function = sn::members::AS_SET.contains(id.name());
    let has_memoize_user_attr = has_memoize_user_attr(attrs);
    // We can skip the check entirely if neither condition is true since we would never
    // raise an error
    if in_as_set_function || has_memoize_user_attr {
        let mut has_inout_param = None;

        // iterate through _all_ parameter; raise an error for `inout` params
        // if we are in a `as_set` function and record the position of the
        // first such parameter we encounter. If we are in a memoized function,
        // we also raise an error for the first parameter, if it exists
        params.iter().for_each(|fp| {
            if matches!(fp.callconv, ParamKind::Pinout(_)) {
                if has_inout_param.is_none() {
                    has_inout_param = Some(fp.pos.clone());
                }
                if in_as_set_function {
                    env.emit_error(NastCheckError::InoutParamsSpecial(fp.pos.clone()))
                }
            }
        });
        if let Some(param_pos) = has_inout_param {
            if has_memoize_user_attr {
                env.emit_error(NastCheckError::InoutParamsMemoize {
                    pos: id.pos().clone(),
                    param_pos,
                })
            }
        }
    }
}

fn has_memoize_user_attr(attrs: &UserAttributes) -> bool {
    attrs.0.iter().any(|ua| {
        ua.name.name() == sn::user_attributes::MEMOIZE
            || ua.name.name() == sn::user_attributes::MEMOIZE_LSB
    })
}
