// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Expr;
use nast::Expr_;
use nast::ShapeFieldName;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateShapeNamePass;

impl Pass for ValidateShapeNamePass {
    fn on_ty_expr_bottom_up(&mut self, env: &Env, expr: &mut Expr) -> ControlFlow<()> {
        if let Expr(_, _, Expr_::Shape(flds)) = expr {
            error_if_duplicate_names(flds, env);
        }
        Continue(())
    }
}

fn error_if_duplicate_names(flds: &[(ShapeFieldName, Expr)], env: &Env) {
    let mut seen = hash::HashSet::<&ShapeFieldName>::default();
    for (name, _) in flds {
        if seen.contains(name) {
            env.emit_error(NamingError::FieldNameAlreadyBound(name.get_pos().clone()));
        }
        seen.insert(name);
    }
}
