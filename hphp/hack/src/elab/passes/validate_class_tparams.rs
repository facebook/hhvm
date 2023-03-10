// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use oxidized::aast_defs::ClassTypeconstDef;
use oxidized::aast_defs::Class_;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::ast_defs::Id;
use oxidized::nast_check_error::NastCheckError;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Default)]
pub struct ValidateClassTparamsPass {
    class_tparams: Option<Vec<Id>>,
    in_typeconst_def: bool,
}

impl Pass for ValidateClassTparamsPass {
    fn on_ty_class__top_down<Ex, En>(
        &mut self,
        elem: &mut Class_<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.class_tparams = Some(elem.tparams.iter().map(|tp| tp.name.clone()).collect());
        ControlFlow::Continue(())
    }

    fn on_ty_class_typeconst_def_top_down<Ex, En>(
        &mut self,
        _elem: &mut ClassTypeconstDef<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.in_typeconst_def = true;
        ControlFlow::Break(())
    }

    fn on_ty_hint_top_down(&mut self, elem: &mut Hint, env: &Env) -> ControlFlow<(), ()> {
        if self.in_typeconst_def {
            match &*elem.1 {
                Hint_::Habstr(tp_name, _) => {
                    if let Some(class_tparams) = &self.class_tparams {
                        class_tparams
                            .iter()
                            .filter(|tp| tp.name() == tp_name)
                            .for_each(|tp| {
                                env.emit_error(NastCheckError::TypeconstDependsOnExternalTparam {
                                    pos: elem.0.clone(),
                                    ext_pos: tp.pos().clone(),
                                    ext_name: tp.name().to_string(),
                                })
                            })
                    }
                }
                _ => (),
            }
        }
        ControlFlow::Continue(())
    }
}
