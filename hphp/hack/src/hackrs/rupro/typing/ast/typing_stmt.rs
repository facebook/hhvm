// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_expr::TCExprParams;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;

impl<R: Reason> Infer<R> for oxidized::aast::Stmt<(), ()> {
    type Params = ();
    type Typed = tast::Stmt<R>;

    fn infer(&self, env: &mut TEnv<R>, _params: ()) -> Result<Self::Typed> {
        use oxidized::aast::Stmt_::*;
        let res = match &self.1 {
            Noop => Noop,
            Expr(box e) => infer_expr(env, e)?,
            Return(box e) => infer_return(env, e.as_ref())?,
            _ => rupro_todo!(AST),
        };
        Ok(oxidized::aast::Stmt(self.0.clone(), res))
    }
}

fn infer_expr<R: Reason>(
    env: &mut TEnv<R>,
    e: &oxidized::aast::Expr<(), ()>,
) -> Result<tast::Stmt_<R>> {
    rupro_todo_mark!(Terminality);
    let te = e.infer(env, TCExprParams::default())?;
    Ok(oxidized::aast::Stmt_::Expr(Box::new(te)))
}

fn infer_return<R: Reason>(
    env: &mut TEnv<R>,
    e: Option<&oxidized::aast::Expr<(), ()>>,
) -> Result<tast::Stmt_<R>> {
    let res = match e {
        None => rupro_todo!(AST),
        Some(e) => {
            rupro_todo_mark!(InoutParameters);
            rupro_todo_mark!(AwaitableAsync);
            rupro_todo_mark!(BidirectionalTC);
            rupro_todo_mark!(Disposable);
            rupro_todo_mark!(Dynamic);
            rupro_todo_mark!(SubtypeCheck, "check against return type");
            rupro_todo_mark!(Flow, "move_and_merge_next_in_cont");

            let te = e.infer(env, TCExprParams::default())?;
            let ret_ty = env.get_return().return_type;
            let err = env.subtyper().subtype(&te.0, &ret_ty)?;
            rupro_todo_assert!(err.is_none(), MissingError);

            oxidized::aast::Stmt_::Return(Box::new(Some(te)))
        }
    };
    Ok(res)
}
