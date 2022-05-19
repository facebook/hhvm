// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::tast;
use crate::typing::ast::typing_localize::LocalizeEnv;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;
use pos::FunName;
use ty::decl;
use ty::local::{Ty, Ty_};
use ty::reason::Reason;
use utils::core::LocalId;

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

impl<R: Reason> Infer<R> for oxidized::aast::Expr<(), ()> {
    type Params = TCExprParams;
    type Typed = tast::Expr<R>;

    fn infer(&self, env: &mut TEnv<R>, params: TCExprParams) -> Result<Self::Typed> {
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
            Call(box (e, explicit_targs, el, unpacked_element)) => {
                infer_call(env, p, e, explicit_targs, el, unpacked_element)?
            }
            h => rupro_todo!(AST, "{:?}", h),
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
    env: &mut TEnv<R>,
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
    env: &mut TEnv<R>,
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
    env: &mut TEnv<R>,
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

fn infer_call<R: Reason>(
    env: &mut TEnv<R>,
    pos: R::Pos,
    e: &oxidized::aast::Expr<(), ()>,
    explicit_targs: &[oxidized::aast::Targ<()>],
    el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    unpacked_element: &Option<oxidized::aast::Expr<(), ()>>,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(MightThrow);
    rupro_todo_mark!(FakeMembersAndRefinement);
    dispatch_call(env, pos, e, explicit_targs, el, unpacked_element)
}

fn dispatch_call<R: Reason>(
    env: &mut TEnv<R>,
    _pos: R::Pos,
    e: &oxidized::aast::Expr<(), ()>,
    explicit_targs: &[oxidized::aast::Targ<()>],
    el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    unpacked_element: &Option<oxidized::aast::Expr<(), ()>>,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(Disposable);

    rupro_todo_mark!(AwaitableAsync, "~allow_awaitable:false");

    use oxidized::aast::Expr_::*;
    match &e.2 {
        Id(id) => {
            rupro_todo_mark!(SpecialFunctions, "needs_special_dispatch");
            dispatch_call_id(env, &e.1, explicit_targs, el, unpacked_element.as_ref(), id)
        }
        _ => rupro_todo!(AST),
    }
}

fn dispatch_call_id<R: Reason>(
    env: &mut TEnv<R>,
    expr_pos: &oxidized::pos::Pos,
    explicit_targs: &[oxidized::aast::Targ<()>],
    el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    unpacked_element: Option<&oxidized::aast::Expr<(), ()>>,
    id: &oxidized::ast_defs::Id,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(Disposable);
    let (fty, tal) = fun_type_of_id(env, id, explicit_targs, el)?;
    let (tel, tunpacked_element, ty, _should_forget_fake_members) =
        call(env, fty.clone(), el, unpacked_element)?;

    let te = oxidized::aast::Expr(
        fty,
        expr_pos.clone(),
        oxidized::aast::Expr_::Id(Box::new(id.clone())),
    );
    Ok((
        oxidized::aast::Expr_::Call(Box::new((te, tal, tel, tunpacked_element))),
        ty,
    ))
}

/// Typechecks a call.
///
/// Returns (1) the typed arguments (2) the typed variadic argument
/// (3) the return type (4) a boolean indicating whether fake
/// members should be forgotten.
fn call<R: Reason>(
    _env: &mut TEnv<R>,
    fty: Ty<R>,
    el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    unpacked_element: Option<&oxidized::aast::Expr<(), ()>>,
) -> Result<(
    Vec<(oxidized::ast_defs::ParamKind, tast::Expr<R>)>,
    Option<tast::Expr<R>>,
    Ty<R>,
    bool,
)> {
    rupro_todo_mark!(Dynamic);
    rupro_todo_mark!(GenericParameters, "get_concrete_supertypes");
    rupro_todo_mark!(Solving, "intersect_list, expand_type_and_solve");

    use Ty_::*;
    match &**fty.node() {
        Tfun(ft) => {
            rupro_todo_mark!(FormatStrings, "retype_magic_func");
            rupro_todo_mark!(BidirectionalTC);
            rupro_todo_mark!(Variance);
            rupro_todo_mark!(FakeMembersAndRefinement);
            rupro_todo_mark!(AST, "check arity");
            rupro_todo_mark!(Dynamic);

            rupro_todo_assert!(ft.params.is_empty() && el.is_empty(), AST);
            rupro_todo_assert!(unpacked_element.is_none(), AST);
            Ok((vec![], None, ft.ret.clone(), false))
        }
        _ => rupro_todo!(AST),
    }
}

/// Given an identifier for a function, return its localized type.
fn fun_type_of_id<R: Reason>(
    env: &mut TEnv<R>,
    id: &oxidized::ast_defs::Id,
    tal: &[oxidized::aast::Targ<()>],
    _el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
) -> Result<(Ty<R>, Vec<tast::Targ<R>>)> {
    let fty = env.decls().get_fun(FunName::new(&id.1))?;
    match fty.as_ref().map(|fty| (&**fty, &**fty.ty.node())) {
        None => rupro_todo!(MissingError),
        Some((fty, decl::Ty_::Tfun(ft))) => {
            rupro_todo_mark!(Dynamic);
            rupro_todo_mark!(Idx);
            rupro_todo_mark!(Modules);
            rupro_todo_mark!(MissingError, "deprecated");
            rupro_todo_assert!(tal.is_empty(), AST);
            let ft = ft.infer(env, LocalizeEnv::no_subst())?;
            let fty = Ty::fun(fty.ty.reason().clone(), ft);
            Ok((fty, vec![]))
        }
        Some((_, _)) => panic!("expected function type"),
    }
}
