// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::ClassTypeconstDef;
use nast::Class_;
use nast::Hint;
use nast::Hint_;
use nast::Id;

use crate::prelude::*;

#[derive(Clone, Default)]
pub struct ValidateClassTparamsPass {
    class_tparams: Option<Rc<Vec<Id>>>,
    in_typeconst_def: bool,
}

impl Pass for ValidateClassTparamsPass {
    fn on_ty_class__top_down(&mut self, _: &Env, elem: &mut Class_) -> ControlFlow<()> {
        self.class_tparams = Some(Rc::new(
            elem.tparams.iter().map(|tp| tp.name.clone()).collect(),
        ));
        Continue(())
    }

    fn on_ty_class_typeconst_def_top_down(
        &mut self,
        _: &Env,
        _: &mut ClassTypeconstDef,
    ) -> ControlFlow<()> {
        self.in_typeconst_def = true;
        Continue(())
    }

    fn on_ty_hint_top_down(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
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
        Continue(())
    }
}
