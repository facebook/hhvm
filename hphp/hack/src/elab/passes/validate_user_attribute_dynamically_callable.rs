// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::Fun_;
use oxidized::aast_defs::Method_;
use oxidized::aast_defs::Pos;
use oxidized::aast_defs::ReifyKind;
use oxidized::aast_defs::Tparam;
use oxidized::aast_defs::UserAttribute;
use oxidized::aast_defs::Visibility;
use oxidized::naming_error::NamingError;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::nast_check_error::NastCheckError;

use crate::config::Config;
use crate::Pass;

#[derive(Copy, Clone, Default)]
pub struct ValidaetUserAttributeDynamicallyCallable;

impl Pass for ValidaetUserAttributeDynamicallyCallable {
    fn on_ty_fun__top_down<Ex, En>(
        &mut self,
        elem: &mut Fun_<Ex, En>,
        _cfg: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        dynamically_callable_attr_pos(&elem.user_attributes)
            .into_iter()
            .for_each(|pos| {
                if has_reified_generics(&elem.tparams) {
                    errs.push(NamingPhaseError::NastCheck(
                        NastCheckError::DynamicallyCallableReified(pos.clone()),
                    ))
                }
            });

        ControlFlow::Continue(())
    }

    fn on_ty_method__top_down<Ex, En>(
        &mut self,
        elem: &mut Method_<Ex, En>,
        _cfg: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        dynamically_callable_attr_pos(&elem.user_attributes)
            .into_iter()
            .for_each(|pos| {
                match &elem.visibility {
                    Visibility::Private | Visibility::Protected => {
                        errs.push(NamingPhaseError::Naming(
                            NamingError::IllegalUseOfDynamicallyCallable {
                                attr_pos: pos.clone(),
                                meth_pos: elem.name.pos().clone(),
                                vis: elem.visibility.into(),
                            },
                        ));
                    }
                    Visibility::Public | Visibility::Internal => (),
                }

                if has_reified_generics(&elem.tparams) {
                    errs.push(NamingPhaseError::NastCheck(
                        NastCheckError::DynamicallyCallableReified(pos.clone()),
                    ))
                }
            });

        ControlFlow::Continue(())
    }
}

fn has_reified_generics<Ex, En>(tparams: &[Tparam<Ex, En>]) -> bool {
    tparams
        .iter()
        .any(|tp| matches!(tp.reified, ReifyKind::Reified | ReifyKind::SoftReified))
}

fn dynamically_callable_attr_pos<Ex, En>(attrs: &[UserAttribute<Ex, En>]) -> Option<&Pos> {
    attrs
        .iter()
        .find(|ua| ua.name.name() == sn::user_attributes::DYNAMICALLY_CALLABLE)
        .map(|ua| ua.name.pos())
}
