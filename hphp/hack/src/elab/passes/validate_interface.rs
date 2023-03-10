// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use itertools::FoldWhile;
use itertools::Itertools;
use oxidized::nast::Class_;
use oxidized::nast::Hint;
use oxidized::nast_check_error::NastCheckError;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateInterfacePass;

impl Pass for ValidateInterfacePass {
    fn on_ty_class__bottom_up(&mut self, elem: &mut Class_, env: &Env) -> ControlFlow<()> {
        if elem.kind.is_cinterface() {
            // Raise an error for each `use` clause
            elem.uses.iter().for_each(|Hint(pos, _)| {
                env.emit_error(NastCheckError::InterfaceUsesTrait(pos.clone()))
            });

            // Raise an error for the first static and instance member variable
            // declared, if any
            let (instance_var_pos_opt, static_var_pos_opt) = elem
                .vars
                .iter()
                .fold_while(
                    (None, None),
                    |(has_instance_var, has_static_var), var| match (
                        &has_instance_var,
                        &has_static_var,
                    ) {
                        (Some(_), Some(_)) => FoldWhile::Done((has_instance_var, has_static_var)),
                        (_, None) if var.is_static => {
                            FoldWhile::Continue((has_instance_var, Some(var.id.pos().clone())))
                        }
                        (None, _) if !var.is_static => {
                            FoldWhile::Continue((Some(var.id.pos().clone()), has_static_var))
                        }
                        _ => FoldWhile::Continue((has_instance_var, has_static_var)),
                    },
                )
                .into_inner();
            if let Some(pos) = instance_var_pos_opt {
                env.emit_error(NastCheckError::InterfaceWithMemberVariable(pos))
            }
            if let Some(pos) = static_var_pos_opt {
                env.emit_error(NastCheckError::InterfaceWithStaticMemberVariable(pos))
            }

            // Raise an error for each method with a non-empty body
            elem.methods
                .iter()
                .filter(|m| !m.body.fb_ast.0.is_empty())
                .for_each(|m| env.emit_error(NastCheckError::AbstractBody(m.name.pos().clone())));
        }
        ControlFlow::Continue(())
    }
}
