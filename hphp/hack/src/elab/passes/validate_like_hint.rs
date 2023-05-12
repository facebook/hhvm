// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Expr_;
use nast::Hint;
use nast::Hint_;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateLikeHintPass {
    allow_like: bool,
}

impl Pass for ValidateLikeHintPass {
    fn on_ty_expr__top_down(&mut self, _: &Env, expr: &mut Expr_) -> ControlFlow<()> {
        self.allow_like = matches!(expr, Expr_::Is(_) | Expr_::As(_) | Expr_::Upcast(_));
        Continue(())
    }

    fn on_ty_hint_top_down(&mut self, env: &Env, hint: &mut Hint) -> ControlFlow<()> {
        match hint {
            Hint(pos, box Hint_::Hlike(_))
                if !(self.allow_like || env.like_type_hints_enabled()) =>
            {
                env.emit_error(ExperimentalFeature::LikeType(pos.clone()));
            }
            Hint(
                _,
                box Hint_::Hfun(..)
                | box Hint_::Happly(..)
                | box Hint_::Haccess(..)
                | box Hint_::Habstr(..)
                | box Hint_::HvecOrDict(..),
            ) => self.allow_like = false,
            _ => (),
        }
        Continue(())
    }
}
