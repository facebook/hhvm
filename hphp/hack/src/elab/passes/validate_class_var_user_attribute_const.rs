// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::ClassVar;
use oxidized::aast_defs::Pos;
use oxidized::aast_defs::UserAttributes;
use oxidized::naming_phase_error::ExperimentalFeature;
use oxidized::naming_phase_error::NamingPhaseError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateClassVarUserAttributeConstPass;

impl Pass for ValidateClassVarUserAttributeConstPass {
    fn on_ty_class_var_bottom_up<Ex, En>(
        &mut self,
        elem: &mut ClassVar<Ex, En>,
        cfg: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        if !cfg.const_static_props() && elem.is_static {
            check_const(elem.id.pos(), &elem.user_attributes, errs)
        }
        ControlFlow::Continue(())
    }
}

fn check_const<Ex, En>(
    pos: &Pos,
    attrs: &UserAttributes<Ex, En>,
    errs: &mut Vec<NamingPhaseError>,
) {
    if attrs
        .0
        .iter()
        .any(|ua| ua.name.name() == sn::user_attributes::CONST)
    {
        errs.push(NamingPhaseError::ExperimentalFeature(
            ExperimentalFeature::ConstStaticProp(pos.clone()),
        ))
    }
}
