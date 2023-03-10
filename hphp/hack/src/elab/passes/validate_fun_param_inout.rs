// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::FunDef;
use oxidized::aast_defs::FunParam;
use oxidized::aast_defs::Method_;
use oxidized::aast_defs::UserAttributes;
use oxidized::ast_defs::Id;
use oxidized::ast_defs::ParamKind;
use oxidized::nast_check_error::NastCheckError;

use crate::env::Env;
use crate::Pass;

#[derive(Copy, Clone, Default)]
pub struct ValidateFunParamInoutPass;

impl Pass for ValidateFunParamInoutPass {
    fn on_ty_fun_def_bottom_up<Ex, En>(
        &mut self,
        elem: &mut FunDef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        check_params(env, &elem.name, &elem.fun.user_attributes, &elem.fun.params);
        ControlFlow::Continue(())
    }
    fn on_ty_method__bottom_up<Ex, En>(
        &mut self,
        elem: &mut Method_<Ex, En>,
        env: &Env,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        check_params(env, &elem.name, &elem.user_attributes, &elem.params);
        ControlFlow::Continue(())
    }
}

fn check_params<Ex, En>(
    env: &Env,
    id: &Id,
    attrs: &UserAttributes<Ex, En>,
    params: &[FunParam<Ex, En>],
) {
    let in_as_set_function = sn::members::AS_LOWERCASE_SET.contains(id.name());
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
        if let Some(param_pos) = has_inout_param && has_memoize_user_attr {
        env.emit_error(NastCheckError::InoutParamsMemoize { pos: id.pos().clone(), param_pos })
      }
    }
}

fn has_memoize_user_attr<Ex, En>(attrs: &UserAttributes<Ex, En>) -> bool {
    attrs.0.iter().any(|ua| {
        ua.name.name() == sn::user_attributes::MEMOIZE
            || ua.name.name() == sn::user_attributes::MEMOIZE_LSB
    })
}
