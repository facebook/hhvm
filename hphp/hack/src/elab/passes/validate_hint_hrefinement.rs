// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Expr_;
use nast::Hint;
use nast::Hint_;

use crate::prelude::*;
#[derive(Copy, Clone, Default)]
pub struct ValidateHintHrefinementPass {
    context: ExprContext,
}

impl Pass for ValidateHintHrefinementPass {
    fn on_ty_expr__top_down(&mut self, _env: &Env, elem: &mut Expr_) -> ControlFlow<()> {
        match elem {
            Expr_::Is(..) => self.context = ExprContext::Is,
            Expr_::As(..) => self.context = ExprContext::As,
            _ => (),
        }
        ControlFlow::Continue(())
    }

    fn on_ty_hint_bottom_up(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        if matches!(&*elem.1, Hint_::Hrefinement(..)) && !matches!(self.context, ExprContext::Other)
        {
            env.emit_error(NastCheckError::RefinementInTypestruct {
                pos: elem.0.clone(),
                kind: self.context.to_string(),
            })
        }
        ControlFlow::Continue(())
    }
}

#[derive(Copy, Clone)]
enum ExprContext {
    Is,
    As,
    Other,
}

impl ToString for ExprContext {
    fn to_string(&self) -> String {
        match self {
            ExprContext::Is => "an is-expression".to_string(),
            ExprContext::As => "an as-expression".to_string(),
            ExprContext::Other => "other".to_string(),
        }
    }
}
impl Default for ExprContext {
    fn default() -> Self {
        Self::Other
    }
}
