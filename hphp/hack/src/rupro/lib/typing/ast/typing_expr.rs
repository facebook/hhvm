// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::reason::Reason;
use crate::tast;
use crate::typing::ast::typing_trait::TC;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;
use crate::typing_defs::Ty;
use crate::utils::core::LocalId;

/// Parameters that affect the typing of expressions.
#[derive(Default)]
pub struct TCExprParams {
    /// Type check the expression with an empty variable environment.
    ///
    /// The variable environment will be emptied before type checking the
    /// expression.
    empty_locals: bool,
}

impl TCExprParams {
    pub fn empty_locals() -> Self {
        Self { empty_locals: true }
    }
}

impl<R: Reason> TC<R> for oxidized::aast::Expr<(), ()> {
    type Params = TCExprParams;
    type Typed = tast::Expr<R>;

    fn infer(&self, env: &TEnv<R>, params: TCExprParams) -> Result<Self::Typed> {
        rupro_todo_mark!(BidirectionalTC);
        rupro_todo_mark!(Coeffects);

        if params.empty_locals {
            env.reinitialize_locals();
        }

        use oxidized::aast::Expr_::*;
        let p = R::Pos::from(&self.1);
        let (e, ty) = match &self.2 {
            Int(s) => (Int(s.clone()), Ty::int(R::witness(p))),
            Binop(box (op, e1, e2)) => infer_binop(env, p, op, e1, e2)?,
            Lvar(box id) => infer_lvar(env, id)?,
            _ => rupro_todo!(AST),
        };
        Ok(oxidized::aast::Expr(ty, self.1.clone(), e))
    }
}

fn infer_lvar<R: Reason>(
    env: &TEnv<R>,
    id: &oxidized::aast_defs::Lid,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(UsingVar);
    rupro_todo_mark!(CheckLocalDefined);
    let pos = R::Pos::from(&id.0);
    let lid = LocalId::from(&id.1);
    let te = oxidized::aast::Expr_::Lvar(Box::new(id.clone()));
    let ty = env.get_local_check_defined(pos, &lid);
    Ok((te, ty))
}

fn infer_binop<R: Reason>(
    env: &TEnv<R>,
    pos: R::Pos,
    bop: &oxidized::ast_defs::Bop,
    e1: &oxidized::aast::Expr<(), ()>,
    e2: &oxidized::aast::Expr<(), ()>,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(Coeffects, "Typing_local_ops.check_assignment");
    use oxidized::ast_defs::Bop::*;
    match bop {
        Eq(None) => infer_binop_assign(env, pos, e1, e2),
        Eq(Some(..)) => rupro_todo!(AST, "Compound assignment operators"),
        _ => rupro_todo!(AST),
    }
}

fn infer_binop_assign<R: Reason>(
    env: &TEnv<R>,
    pos: R::Pos,
    lhs: &oxidized::aast::Expr<(), ()>,
    rhs: &oxidized::aast::Expr<(), ()>,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    let trhs = rhs.infer(env, TCExprParams::default())?;
    let tlhs = infer_assign(env, pos, lhs, &trhs)?;
    rupro_todo_mark!(Holes);
    let ty = trhs.0.clone();
    let res =
        oxidized::aast::Expr_::Binop(Box::new((oxidized::ast_defs::Bop::Eq(None), tlhs, trhs)));
    Ok((res, ty))
}

fn infer_assign<R: Reason>(
    env: &TEnv<R>,
    pos: R::Pos,
    lhs: &oxidized::aast::Expr<(), ()>,
    trhs: &tast::Expr<R>,
) -> Result<tast::Expr<R>> {
    rupro_todo_mark!(Holes);
    rupro_todo_mark!(AwaitableAsync);
    rupro_todo_mark!(FakeMembersAndRefinement);
    use oxidized::aast::Expr_::*;
    let (te, ty) = match &lhs.2 {
        Lvar(x) => {
            env.set_local_new_value(false, LocalId::from(&x.1), trhs.0.clone(), pos);
            (Lvar(x.clone()), trhs.0.clone())
        }
        _ => rupro_todo!(AST),
    };
    Ok(oxidized::aast::Expr(ty, lhs.1.clone(), te))
}
