// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::ClassVar;
use nast::Id;

use crate::prelude::*;

#[derive(Clone, Default)]
pub struct ValidateClassVarUserAttributeLsbPass {
    final_class: Option<Rc<Id>>,
}

impl Pass for ValidateClassVarUserAttributeLsbPass {
    fn on_ty_class__top_down(&mut self, _: &Env, elem: &mut nast::Class_) -> ControlFlow<()> {
        self.final_class = if elem.final_ {
            Some(Rc::new(elem.name.clone()))
        } else {
            None
        };
        Continue(())
    }

    fn on_ty_class_var_bottom_up(&mut self, env: &Env, elem: &mut ClassVar) -> ControlFlow<()> {
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
        Continue(())
    }
}
