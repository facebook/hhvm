// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::naming_error::NamingError;
use oxidized::nast::FunDef;
use oxidized::nast::Method_;
use oxidized::nast::Pos;
use oxidized::nast::ReifyKind;
use oxidized::nast::Tparam;
use oxidized::nast::UserAttribute;
use oxidized::nast::Visibility;
use oxidized::nast_check_error::NastCheckError;

use crate::env::Env;
use crate::Pass;

#[derive(Copy, Clone, Default)]
pub struct ValidaetUserAttributeDynamicallyCallable;

impl Pass for ValidaetUserAttributeDynamicallyCallable {
    fn on_ty_fun_def_top_down(&mut self, env: &Env, elem: &mut FunDef) -> ControlFlow<()> {
        dynamically_callable_attr_pos(&elem.fun.user_attributes)
            .into_iter()
            .for_each(|pos| {
                if has_reified_generics(&elem.tparams) {
                    env.emit_error(NastCheckError::DynamicallyCallableReified(pos.clone()))
                }
            });

        ControlFlow::Continue(())
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

        ControlFlow::Continue(())
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
