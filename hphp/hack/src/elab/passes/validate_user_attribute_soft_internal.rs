// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use nast::Class_;
use nast::FunDef;
use nast::Fun_;
use nast::Method_;
use nast::UserAttribute;
use nast::Visibility;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateUserAttributeSoftInternalPass;

impl Pass for ValidateUserAttributeSoftInternalPass {
    fn on_ty_class__bottom_up(&mut self, env: &Env, elem: &mut Class_) -> ControlFlow<(), ()> {
        check_soft_internal(elem.internal, &elem.user_attributes, env);
        elem.vars.iter().for_each(|cv| {
            check_soft_internal(
                cv.visibility == Visibility::Internal,
                &cv.user_attributes,
                env,
            )
        });
        ControlFlow::Continue(())
    }

    fn on_ty_fun__bottom_up(&mut self, env: &Env, elem: &mut Fun_) -> ControlFlow<(), ()> {
        elem.params
            .iter()
            .for_each(|p| check_soft_internal_on_param(&p.visibility, &p.user_attributes, env));
        ControlFlow::Continue(())
    }
    fn on_ty_fun_def_bottom_up(&mut self, env: &Env, elem: &mut FunDef) -> ControlFlow<(), ()> {
        check_soft_internal(elem.internal, &elem.fun.user_attributes, env);
        ControlFlow::Continue(())
    }

    fn on_ty_method__bottom_up(&mut self, env: &Env, elem: &mut Method_) -> ControlFlow<(), ()> {
        check_soft_internal(
            elem.visibility == Visibility::Internal,
            &elem.user_attributes,
            env,
        );
        elem.params
            .iter()
            .for_each(|p| check_soft_internal_on_param(&p.visibility, &p.user_attributes, env));
        ControlFlow::Continue(())
    }
}

fn check_soft_internal(is_internal: bool, user_attrs: &[UserAttribute], env: &Env) {
    if !is_internal
        && let Some(ua) = user_attrs
            .iter()
            .find(|ua| ua.name.name() == sn::user_attributes::SOFT_INTERNAL)
    {
        env.emit_error(NastCheckError::SoftInternalWithoutInternal(
            ua.name.pos().clone(),
        ))
    }
}

fn check_soft_internal_on_param(
    visiblity_opt: &Option<Visibility>,
    user_attrs: &[UserAttribute],
    env: &Env,
) {
    user_attrs
        .iter()
        .find(|ua| ua.name.name() == sn::user_attributes::SOFT_INTERNAL)
        .iter()
        .for_each(|ua| match visiblity_opt {
            Some(Visibility::Internal) => (),
            Some(Visibility::Private | Visibility::Protected | Visibility::Public) => env
                .emit_error(NastCheckError::SoftInternalWithoutInternal(
                    ua.name.pos().clone(),
                )),
            _ => env.emit_error(NastCheckError::WrongExpressionKindBuiltinAttribute {
                pos: ua.name.pos().clone(),
                attr_name: ua.name.name().to_string(),
                expr_kind: "a parameter".to_string(),
            }),
        })
}
