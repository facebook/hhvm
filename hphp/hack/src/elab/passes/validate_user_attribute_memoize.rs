// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use nast::FunParam;
use nast::Fun_;
use nast::Method_;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateUserAttributeMemoizePass;

// Ban variadic arguments on memoized methods.
impl Pass for ValidateUserAttributeMemoizePass {
    fn on_ty_fun__bottom_up(&mut self, env: &Env, elem: &mut Fun_) -> ControlFlow<(), ()> {
        if elem
            .user_attributes
            .iter()
            .any(|ua| ua.name.name() == sn::user_attributes::MEMOIZE)
        {
            check_variadic_param(&elem.params, env)
        }
        ControlFlow::Continue(())
    }

    fn on_ty_method__bottom_up(&mut self, env: &Env, elem: &mut Method_) -> ControlFlow<(), ()> {
        let memo_opt = elem
            .user_attributes
            .iter()
            .find(|ua| ua.name.name() == sn::user_attributes::MEMOIZE);
        let memo_lsb_opt = elem
            .user_attributes
            .iter()
            .find(|ua| ua.name.name() == sn::user_attributes::MEMOIZE_LSB);
        match (memo_opt, memo_lsb_opt) {
            (Some(memo), Some(memo_lsb)) => {
                check_variadic_param(&elem.params, env);
                env.emit_error(NastCheckError::AttributeConflictingMemoize {
                    pos: memo.name.pos().clone(),
                    second_pos: memo_lsb.name.pos().clone(),
                })
            }
            (Some(_), _) | (_, Some(_)) => check_variadic_param(&elem.params, env),
            _ => (),
        }
        ControlFlow::Continue(())
    }
}

fn check_variadic_param(params: &[FunParam], env: &Env) {
    if let Some(param) = params.iter().find(|fp| fp.is_variadic) {
        env.emit_error(NastCheckError::VariadicMemoize(param.pos.clone()))
    }
}
