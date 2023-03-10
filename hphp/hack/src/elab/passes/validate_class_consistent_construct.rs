// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::Class_;
use oxidized::ast::ClassishKind;
use oxidized::naming_error::NamingError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateClassConsistentConstructPass;

impl Pass for ValidateClassConsistentConstructPass {
    fn on_ty_class__top_down<Ex: Default, En>(
        &mut self,
        class: &mut Class_<Ex, En>,
        cfg: &Config,
    ) -> ControlFlow<(), ()> {
        if cfg.consistent_ctor_level <= 0 {
            return ControlFlow::Continue(());
        }
        let attr_pos_opt = class
            .user_attributes
            .iter()
            .find(|ua| ua.name.name() == sn::user_attributes::CONSISTENT_CONSTRUCT)
            .map(|ua| ua.name.pos());
        let has_ctor = class
            .methods
            .iter()
            .any(|m| m.name.name() == sn::members::__CONSTRUCT);
        match attr_pos_opt {
            // If this classish is attributed `__ConsistentConstruct` & does not
            // define a constructor then, if either it's a trait or
            // `consistent_ctor_level` > 1, it's an error.
            Some(pos)
                if !has_ctor
                    && (class.kind == ClassishKind::Ctrait || cfg.consistent_ctor_level > 1) =>
            {
                cfg.emit_error(NamingError::ExplicitConsistentConstructor {
                    classish_kind: class.kind,
                    pos: pos.clone(),
                })
            }
            _ => (),
        }
        ControlFlow::Continue(())
    }
}
