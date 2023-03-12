// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::ClassId;
use nast::ClassId_;
use nast::Class_;
use nast::ClassishKind;
use nast::Expr;
use nast::Expr_;
use nast::FunctionPtrId;
use nast::Hint;
use nast::Hint_;
use nast::Id;

use crate::prelude::*;

#[derive(Clone, Default)]
pub struct ValidateFunctionPointerPass {
    pub in_final_class: bool,
    pub class_name: Option<String>,
    pub parent_name: Option<String>,
    pub is_trait: bool,
}

impl Pass for ValidateFunctionPointerPass {
    fn on_ty_class__top_down(&mut self, _: &Env, class: &mut Class_) -> ControlFlow<()> {
        *self = Self {
            in_final_class: class.final_,
            class_name: Some(class.name.name().to_string()),
            parent_name: class.extends.first().and_then(|h| {
                if let Hint(_, box Hint_::Happly(Id(_, name), _)) = h {
                    Some(name.clone())
                } else {
                    None
                }
            }),
            is_trait: matches!(class.kind, ClassishKind::Ctrait),
        };
        Continue(())
    }

    fn on_ty_expr_bottom_up(&mut self, env: &Env, expr: &mut Expr) -> ControlFlow<()> {
        let Expr(_, _, expr_) = expr;
        match expr_ {
            Expr_::FunctionPointer(box (
                FunctionPtrId::FPClassConst(ClassId(_, pos, ClassId_::CIself), (_, meth_name)),
                _,
            )) => {
                if !self.in_final_class {
                    if self.is_trait {
                        env.emit_error(NamingError::SelfInNonFinalFunctionPointer {
                            pos: pos.clone(),
                            meth_name: meth_name.clone(),
                            class_name: None,
                        });
                    }
                } else {
                    env.emit_error(NamingError::SelfInNonFinalFunctionPointer {
                        pos: pos.clone(),
                        meth_name: meth_name.clone(),
                        class_name: self.class_name.clone(),
                    });
                }
            }
            Expr_::FunctionPointer(box (
                FunctionPtrId::FPClassConst(ClassId(_, pos, ClassId_::CIparent), (_, meth_name)),
                _,
            )) => {
                env.emit_error(NamingError::ParentInFunctionPointer {
                    pos: pos.clone(),
                    parent_name: self.parent_name.clone(),
                    meth_name: meth_name.clone(),
                });
            }
            _ => (),
        }
        Continue(())
    }
}
