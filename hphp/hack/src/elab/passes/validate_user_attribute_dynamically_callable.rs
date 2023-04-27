// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::FunDef;
use nast::Method_;
use nast::Pos;
use nast::ReifyKind;
use nast::Tparam;
use nast::UserAttribute;
use nast::Visibility;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateUserAttributeDynamicallyCallable;

impl Pass for ValidateUserAttributeDynamicallyCallable {
    fn on_ty_fun_def_top_down(&mut self, env: &Env, elem: &mut FunDef) -> ControlFlow<()> {
        dynamically_callable_attr_pos(&elem.fun.user_attributes)
            .into_iter()
            .for_each(|pos| {
                if has_reified_generics(&elem.tparams) {
                    env.emit_error(NastCheckError::DynamicallyCallableReified(pos.clone()))
                }
            });

        Continue(())
    }

    fn on_ty_method__top_down(&mut self, env: &Env, elem: &mut Method_) -> ControlFlow<()> {
        dynamically_callable_attr_pos(&elem.user_attributes)
            .into_iter()
            .for_each(|pos| {
                match &elem.visibility {
                    Visibility::Private | Visibility::Protected => {
                        env.emit_error(NamingError::IllegalUseOfDynamicallyCallable {
                            attr_pos: pos.clone(),
                            meth_pos: elem.name.pos().clone(),
                            vis: elem.visibility.into(),
                        });
                    }
                    Visibility::Public | Visibility::Internal => (),
                }

                if has_reified_generics(&elem.tparams) {
                    env.emit_error(NastCheckError::DynamicallyCallableReified(pos.clone()))
                }
            });

        Continue(())
    }
}

fn has_reified_generics(tparams: &[Tparam]) -> bool {
    tparams
        .iter()
        .any(|tp| matches!(tp.reified, ReifyKind::Reified | ReifyKind::SoftReified))
}

fn dynamically_callable_attr_pos(attrs: &[UserAttribute]) -> Option<&Pos> {
    attrs
        .iter()
        .find(|ua| ua.name.name() == sn::user_attributes::DYNAMICALLY_CALLABLE)
        .map(|ua| ua.name.pos())
}
