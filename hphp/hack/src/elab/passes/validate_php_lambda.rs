// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::nast::Expr;
use oxidized::nast::Expr_;
use oxidized::nast_check_error::NastCheckError;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Default)]
pub struct ValidatePhpLambdaPass;

impl Pass for ValidatePhpLambdaPass {
    fn on_ty_expr_bottom_up(&mut self, env: &Env, expr: &mut Expr) -> ControlFlow<()> {
        if env.error_php_lambdas() && matches!(expr, Expr(_, _, Expr_::Efun(_))) {
            env.emit_error(NastCheckError::PhpLambdaDisallowed(expr.pos().clone()));
        }
        ControlFlow::Continue(())
    }
}
