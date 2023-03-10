// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::naming_error::NamingError;
use oxidized::nast::ClassVar;
use oxidized::nast::Id;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Default)]
pub struct ValidateClassVarUserAttributeLsbPass {
    final_class: Option<Id>,
}

impl Pass for ValidateClassVarUserAttributeLsbPass {
    fn on_ty_class__bottom_up(
        &mut self,
        elem: &mut oxidized::nast::Class_,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.final_class = if elem.final_ {
            Some(elem.name.clone())
        } else {
            None
        };
        ControlFlow::Continue(())
    }

    fn on_ty_class_var_bottom_up(&mut self, elem: &mut ClassVar, env: &Env) -> ControlFlow<()> {
        if let Some(ua) = elem
            .user_attributes
            .0
            .iter()
            .find(|ua| ua.name.name() == sn::user_attributes::LSB)
        {
            // Non-static properties cannot have attribute `__LSB`
            if !elem.is_static {
                env.emit_error(NamingError::NonstaticPropertyWithLsb(ua.name.pos().clone()))
            }
            // `__LSB` attribute is unnecessary in final classes
            if let Some(id) = &self.final_class {
                env.emit_error(NamingError::UnnecessaryAttribute {
                    pos: ua.name.pos().clone(),
                    attr: sn::user_attributes::LSB.to_string(),
                    class_pos: id.pos().clone(),
                    class_name: id.name().to_string(),
                    suggestion: None,
                });
            }
        }
        ControlFlow::Continue(())
    }
}
